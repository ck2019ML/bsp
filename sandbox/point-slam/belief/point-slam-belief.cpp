#include <vector>
#include <iomanip>

#include "../point-slam.h"

#include "util/matrix.h"
#include "util/Timer.h"
#include "util/logging.h"
//#include "util/utils.h"

#include <Python.h>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>

namespace py = boost::python;



extern "C" {
#include "beliefPenaltyMPC.h"
beliefPenaltyMPC_FLOAT **H, **f, **lb, **ub, **C, **e, **z;
}

Matrix<X_DIM> x0;
Matrix<X_DIM,X_DIM> SqrtSigma0;
Matrix<X_DIM> xGoal;
Matrix<X_DIM> xMin, xMax;
Matrix<U_DIM> uMin, uMax;

namespace cfg {
const double improve_ratio_threshold = .1;
const double min_approx_improve = 1e-4;
const double min_trust_box_size = 1e-3;
const double trust_shrink_ratio = .1;
const double trust_expand_ratio = 1.5;
const double cnt_tolerance = 1e-4;
const double penalty_coeff_increase_ratio = 10;
const double initial_penalty_coeff = 10;
const double initial_trust_box_size = 1;
const int max_penalty_coeff_increases = 2;
const int max_sqp_iterations = 50;
}


double computeCost(const std::vector< Matrix<B_DIM> >& B, const std::vector< Matrix<U_DIM> >& U)
{
	double cost = 0;
	Matrix<X_DIM> x;
	Matrix<X_DIM, X_DIM> SqrtSigma;

	for(int t = 0; t < T-1; ++t) {
		unVec(B[t], x, SqrtSigma);
		cost += alpha_belief*tr(SqrtSigma*SqrtSigma) + alpha_control*tr(~U[t]*U[t]);
	}
	unVec(B[T-1], x, SqrtSigma);
	cost += alpha_final_belief*tr(SqrtSigma*SqrtSigma);
	return cost;
}

// Jacobians: dg(b,u)/db, dg(b,u)/du
void linearizeBeliefDynamics(const Matrix<B_DIM>& b, const Matrix<U_DIM>& u, Matrix<B_DIM,B_DIM>& F, Matrix<B_DIM,U_DIM>& G, Matrix<B_DIM>& h)
{
	F.reset();
	Matrix<B_DIM> br(b), bl(b);
	for (size_t i = 0; i < B_DIM; ++i) {
		br[i] += step; bl[i] -= step;
		//std::cout << "bplus: " << ~(beliefDynamics(br, u)) << std::endl;
		//std::cout << "bminus: " << ~(beliefDynamics(bl, u)) << std::endl;
		F.insert(0,i, (beliefDynamics(br, u) - beliefDynamics(bl, u)) / (br[i] - bl[i]));
		br[i] = b[i]; bl[i] = b[i];
	}

	G.reset();
	Matrix<U_DIM> ur(u), ul(u);
	for (size_t i = 0; i < U_DIM; ++i) {
		ur[i] += step; ul[i] -= step;
		G.insert(0,i, (beliefDynamics(b, ur) - beliefDynamics(b, ul)) / (ur[i] - ul[i]));
		ur[i] = u[i]; ul[i] = u[i];
	}

	h = beliefDynamics(b, u);
}


void setupBeliefVars(beliefPenaltyMPC_params& problem, beliefPenaltyMPC_output& output)
{
	H = new beliefPenaltyMPC_FLOAT*[T];
	f = new beliefPenaltyMPC_FLOAT*[T-1];
	lb = new beliefPenaltyMPC_FLOAT*[T];
	ub = new beliefPenaltyMPC_FLOAT*[T];
	C = new beliefPenaltyMPC_FLOAT*[T-1];
	e = new beliefPenaltyMPC_FLOAT*[T-1];
	z = new beliefPenaltyMPC_FLOAT*[T];

#define SET_VARS(n)    \
		H[ BOOST_PP_SUB(n,1) ] = problem.H##n ;  \
		f[ BOOST_PP_SUB(n,1) ] = problem.f##n ;  \
		C[ BOOST_PP_SUB(n,1) ] = problem.C##n ;  \
		e[ BOOST_PP_SUB(n,1) ] = problem.e##n ;  \
		lb[ BOOST_PP_SUB(n,1) ] = problem.lb##n ;	\
		ub[ BOOST_PP_SUB(n,1) ] = problem.ub##n ;	\
		z[ BOOST_PP_SUB(n,1) ] = output.z##n ;

#define BOOST_PP_LOCAL_MACRO(n) SET_VARS(n)
#define BOOST_PP_LOCAL_LIMITS (1, TIMESTEPS-1)
#include BOOST_PP_LOCAL_ITERATE()


#define SET_LAST_VARS(n)    \
		H[ BOOST_PP_SUB(n,1) ] = problem.H##n ;  \
		lb[ BOOST_PP_SUB(n,1) ] = problem.lb##n ;	\
		ub[ BOOST_PP_SUB(n,1) ] = problem.ub##n ;	\
		z[ BOOST_PP_SUB(n,1) ] = output.z##n ;

#define BOOST_PP_LOCAL_MACRO(n) SET_LAST_VARS(n)
#define BOOST_PP_LOCAL_LIMITS (TIMESTEPS, TIMESTEPS)
#include BOOST_PP_LOCAL_ITERATE()

	int index;
	for(int t = 0; t < T-1; ++t) {
		index = 0;
		for(int i = 0; i < X_DIM; ++i) { H[t][index++] = 0; }
		for(int i = 0; i < S_DIM; ++i) { H[t][index++] = alpha_belief; }
		for(int i = 0; i < U_DIM; ++i) { H[t][index++] = alpha_control; }
		for(int i = 0; i < 2*B_DIM; ++i) { H[t][index++] = 0; }
	}

	for(int i = 0; i < X_DIM; ++i) { H[T-1][index++] = 0; }
	for(int i = 0; i < S_DIM; ++i) { H[T-1][index++] = alpha_final_belief; }


}

void cleanupBeliefMPCVars()
{
	delete[] f;
	delete[] lb; 
	delete[] ub; 
	delete[] C;
	delete[] e;
	delete[] z;
}


double computeMerit(const std::vector< Matrix<B_DIM> >& B, const std::vector< Matrix<U_DIM> >& U, double penalty_coeff)
{
	double merit = 0;
	Matrix<X_DIM> x;
	Matrix<X_DIM, X_DIM> SqrtSigma;
	Matrix<B_DIM> dynviol;
	for(int t = 0; t < T-1; ++t) {
		unVec(B[t], x, SqrtSigma);
		merit += alpha_belief*tr(SqrtSigma*SqrtSigma) + alpha_control*tr(~U[t]*U[t]);
		dynviol = (B[t+1] - beliefDynamics(B[t], U[t]) );
		for(int i = 0; i < B_DIM; ++i) {
			merit += penalty_coeff*fabs(dynviol[i]);
		}
	}
	unVec(B[T-1], x, SqrtSigma);
	merit += alpha_final_belief*tr(SqrtSigma*SqrtSigma);
	return merit;
}

bool isValidInputs()
{
	for(int t = 0; t < T-1; ++t) {

		std::cout << "t: " << t << std::endl << std::endl;

		std::cout << "f: ";
		for(int i = 0; i < (3*B_DIM+U_DIM); ++i) {
			std::cout << f[t][i] << " ";
		}
		std::cout << std::endl;

		std::cout << "lb b: ";
		for(int i = 0; i < B_DIM; ++i) {
			std::cout << lb[t][i] << " ";
		}
		std::cout << std::endl;

		std::cout << "lb u: ";
		for(int i = 0; i < U_DIM; ++i) {
			std::cout << lb[t][B_DIM+i] << " ";
		}
		std::cout << std::endl;

		std::cout << "lb s, t: ";
		for(int i = 0; i < 2*B_DIM; ++i) {
			std::cout << lb[t][B_DIM+U_DIM+i] << " ";
		}
		std::cout << std::endl;

		std::cout << "ub b: ";
		for(int i = 0; i < B_DIM; ++i) {
			std::cout << ub[t][i] << " ";
		}
		std::cout << std::endl;

		std::cout << "ub u: ";
		for(int i = 0; i < U_DIM; ++i) {
			std::cout << ub[t][B_DIM+i] << " ";
		}
		std::cout << std::endl;

		//std::cout << "ub s, t: ";
		//for(int i = 0; i < 2*B_DIM; ++i) {
		//	std::cout << ub[t][B_DIM+U_DIM+i] << " ";
		//}
		//std::cout << std::endl;

		std::cout << "C:" << std::endl;
		if (t == 0) {
			for(int i = 0; i < 170; ++i) {
				std::cout << C[t][i] << " ";
			}
		} else {
			for(int i = 0; i < 85; ++i) {
				std::cout << C[t][i] << " ";
			}
		}
		std::cout << std::endl;

		std::cout << "e:" << std::endl;
		if (t == 0) {
			for(int i = 0; i < 10; ++i) {
				std::cout << e[t][i] << " ";
			}
		} else {
			for(int i = 0; i < 5; ++i) {
				std::cout << e[t][i] << " ";
			}
		}

		std::cout << std::endl << std::endl;
	}
	return true;
}

bool minimizeMeritFunction(std::vector< Matrix<B_DIM> >& B, std::vector< Matrix<U_DIM> >& U, beliefPenaltyMPC_params& problem, beliefPenaltyMPC_output& output, beliefPenaltyMPC_info& info, double penalty_coeff, double trust_box_size)
{
	LOG_DEBUG("Solving sqp problem with penalty parameter: %2.4f", penalty_coeff);

	// box constraint around goal
	double delta = 0.01;

	Matrix<B_DIM,1> b0 = B[0];

	std::vector< Matrix<B_DIM,B_DIM> > F(T-1);
	std::vector< Matrix<B_DIM,U_DIM> > G(T-1);
	std::vector< Matrix<B_DIM> > h(T-1);

	double Beps = trust_box_size;
	double Ueps = trust_box_size;

	double prevcost, optcost;

	std::vector<Matrix<B_DIM> > Bopt(T);
	std::vector<Matrix<U_DIM> > Uopt(T-1);

	double merit, model_merit, new_merit;
	double approx_merit_improve, exact_merit_improve, merit_improve_ratio;

	int sqp_iter = 1, index = 0;
	bool success;

	Matrix<B_DIM,B_DIM> I = identity<B_DIM>();
	Matrix<B_DIM,B_DIM> minusI = I;
	for(int i = 0; i < B_DIM; ++i) {
		minusI(i,i) = -1;
	}

	// sqp loop
	while(true)
	{
		// In this loop, we repeatedly construct a linear approximation to the nonlinear belief dynamics constraint
		LOG_DEBUG("  sqp iter: %d", sqp_iter);

		merit = computeMerit(B, U, penalty_coeff);
		LOG_DEBUG("  merit: %4.10f", merit);

		for (int t = 0; t < T-1; ++t) {
			linearizeBeliefDynamics(B[t], U[t], F[t], G[t], h[t]);
		}

		// trust region size adjustment
		while(true)
		{
			LOG_DEBUG("       trust region size: %2.6f %2.6f", Beps, Ueps);

			// solve the innermost QP here
			for(int t = 0; t < T-1; ++t)
			{
				Matrix<B_DIM>& bt = B[t];
				Matrix<U_DIM>& ut = U[t];

				// Fill in f, lb, ub, C, e
				for(int i = 0; i < (B_DIM+U_DIM); ++i) {
					f[t][i] = 0;
				}
				for(int i = 0; i < 2*B_DIM; ++i) {
					f[t][B_DIM+U_DIM+i] = penalty_coeff;
				}


				for (int i = 0; i<X_DIM; ++i) {
				  lb[t][i] = MAX(xMin[i], bt[i] - Beps);
				}
				for (int i = 0; i<S_DIM; ++i) {
				  lb[t][X_DIM+i] = bt[X_DIM+i] - Beps;
				}
				for (int i = 0; i<U_DIM; ++i) {
				  lb[t][B_DIM+i] = MAX(uMin[i], ut[i] - Ueps);
				}


				for(int i = 0; i < 2*B_DIM; ++i) {
					lb[t][B_DIM+U_DIM+i] = 0;
				}


				for (int i = 0; i<X_DIM; ++i) {
				  ub[t][i] = MIN(xMax[i], bt[i] + Beps); 
				}
				for (int i = 0; i<S_DIM; ++i) {
				  ub[t][X_DIM+i] = bt[X_DIM+i] + Beps;
				}
				for (int i = 0; i<U_DIM; ++i) {
				  ub[t][B_DIM+i] = MIN(uMax[i], ut[i] + Ueps);
				}
				//for(int i = 0; i < 2*B_DIM; ++i) {
				//	ub[t][B_DIM+U_DIM+i] = INFTY;
				//}

				if (t > 0) {
					Matrix<B_DIM,3*B_DIM+U_DIM> CMat;
					Matrix<B_DIM> eVec;

					CMat.insert<B_DIM,B_DIM>(0,0,F[t]);
					CMat.insert<B_DIM,U_DIM>(0,B_DIM,G[t]);
					CMat.insert<B_DIM,B_DIM>(0,B_DIM+U_DIM,I);
					CMat.insert<B_DIM,B_DIM>(0,2*B_DIM+U_DIM,minusI);

					//std::cout << CMat << std::endl;

					int idx = 0;
					int nrows = CMat.numRows(), ncols = CMat.numColumns();
					for(int c = 0; c < ncols; ++c) {
						for(int r = 0; r < nrows; ++r) {
							C[t][idx++] = CMat[c + r*ncols];
						}
					}
					eVec = -h[t] + F[t]*bt + G[t]*ut;
					int nelems = eVec.numRows();
					for(int i = 0; i < nelems; ++i) {
						e[t][i] = eVec[i];
					}
				}
				else {
					Matrix<2*B_DIM,3*B_DIM+U_DIM> CMat;
					Matrix<2*B_DIM> eVec;

					CMat.insert<B_DIM,B_DIM>(0,0,I);
					CMat.insert<B_DIM,U_DIM>(0,B_DIM,zeros<B_DIM,U_DIM>());
					CMat.insert<B_DIM,2*B_DIM>(0,B_DIM+U_DIM,zeros<B_DIM,2*B_DIM>());

					CMat.insert<B_DIM,B_DIM>(B_DIM,0,F[t]);
					CMat.insert<B_DIM,U_DIM>(B_DIM,B_DIM,G[t]);
					CMat.insert<B_DIM,B_DIM>(B_DIM,B_DIM+U_DIM,I);
					CMat.insert<B_DIM,B_DIM>(B_DIM,2*B_DIM+U_DIM,minusI);

					int idx = 0;
					int nrows = CMat.numRows(), ncols = CMat.numColumns();
					for(int c = 0; c < ncols; ++c) {
						for(int r = 0; r < nrows; ++r) {
							C[t][idx++] = CMat[c + r*ncols];
						}
					}
					eVec.insert<B_DIM,1>(0,0,b0);
					eVec.insert<B_DIM,1>(B_DIM,0,zeros<B_DIM,1>());
					int nelems = eVec.numRows();
					for(int i = 0; i < nelems; ++i) {
						e[t][i] = eVec[i];
					}
				}
			}

			Matrix<B_DIM>& bT = B[T-1];

			// Fill in lb, ub, C, e
			for (int i = 0; i<X_DIM; ++i) {
			  lb[T-1][i] = MAX(xGoal[i]-delta, bT[i] - Beps);  
			}
			for (int i = 0; i<S_DIM; ++i) {
			  lb[T-1][X_DIM+i] = bT[X_DIM+i] - Beps;
			}

			for (int i = 0; i<X_DIM; ++i) {
			  ub[T-1][i] = MIN(xGoal[i]+delta, bT[i] + Beps); 
			}
			for (int i = 0; i<S_DIM; ++i) {
			  ub[T-1][X_DIM+i] = bT[X_DIM+i] + Beps;
			}

			// Verify problem inputs
			//if (!isValidInputs()) {
			//	std::cout << "Inputs are not valid!" << std::endl;
			//	exit(-1);
			//}

			//std::cerr << "PAUSING INSIDE MINIMIZE MERIT FUNCTION FOR INPUT VERIFICATION" << std::endl;
			//int num;
			//std::cin >> num;

			int exitflag = beliefPenaltyMPC_solve(&problem, &output, &info);
			if (exitflag == 1) {
				for(int t = 0; t < T-1; ++t) {
					Matrix<B_DIM>& bt = Bopt[t];
					Matrix<U_DIM>& ut = Uopt[t];

					for(int i = 0; i < B_DIM; ++i) {
						bt[i] = z[t][i];
					}
					for(int i = 0; i < U_DIM; ++i) {
						ut[i] = z[t][B_DIM+i];
					}
					optcost = info.pobj;
				}
				for(int i = 0; i < B_DIM; ++i) {
					Bopt[T-1][i] = z[T-1][i];
				}
			}
			else {
				LOG_ERROR("Some problem in solver");
				std::exit(-1);
			}

			LOG_DEBUG("Optimized cost: %4.10f", optcost);

			model_merit = optcost;
			new_merit = computeMerit(Bopt, Uopt, penalty_coeff);

			LOG_DEBUG("merit: %4.10f", merit);
			LOG_DEBUG("model_merit: %4.10f", model_merit);
			LOG_DEBUG("new_merit: %4.10f", new_merit);

			approx_merit_improve = merit - model_merit;
			exact_merit_improve = merit - new_merit;
			merit_improve_ratio = exact_merit_improve / approx_merit_improve;

			LOG_DEBUG("approx_merit_improve: %1.6f", approx_merit_improve);
			LOG_DEBUG("exact_merit_improve: %1.6f", exact_merit_improve);
			LOG_DEBUG("merit_improve_ratio: %1.6f", merit_improve_ratio);

			//std::cout << "PAUSED INSIDE minimizeMeritFunction" << std::endl;
			//int num;
			//std::cin >> num;

			if (approx_merit_improve < -1e-5) {
				LOG_ERROR("Approximate merit function got worse: %1.6f", approx_merit_improve);
				LOG_ERROR("Either convexification is wrong to zeroth order, or you are in numerical trouble");
				LOG_ERROR("Failure!");
				success = false;
			} else if (approx_merit_improve < cfg::min_approx_improve) {
				LOG_DEBUG("Converged: improvement small enough");
				B = Bopt; U = Uopt;
				return true;
			} else if ((exact_merit_improve < 0) || (merit_improve_ratio < cfg::improve_ratio_threshold)) {
				Beps *= cfg::trust_shrink_ratio;
				Ueps *= cfg::trust_shrink_ratio;
				LOG_DEBUG("Shrinking trust region size to: %2.6f %2.6f", Beps, Ueps);
			} else {
				Beps *= cfg::trust_expand_ratio;
				Ueps *= cfg::trust_expand_ratio;
				B = Bopt; U = Uopt;
				prevcost = optcost;
				LOG_DEBUG("Accepted, Increasing trust region size to:  %2.6f %2.6f", Beps, Ueps);
				break;
			}

			if (Beps < cfg::min_trust_box_size && Ueps < cfg::min_trust_box_size) {
			    LOG_DEBUG("Converged: x tolerance");
			    return true;
			}

		} // trust region loop
		sqp_iter++;
	} // sqp loop

	return success;
}

double beliefPenaltyCollocation(std::vector< Matrix<B_DIM> >& B, std::vector< Matrix<U_DIM> >& U, beliefPenaltyMPC_params& problem, beliefPenaltyMPC_output& output, beliefPenaltyMPC_info& info)
{
	util::Timer costTimer;
	double costTime = 0;

	double penalty_coeff = cfg::initial_penalty_coeff;
	double trust_box_size = cfg::initial_trust_box_size;

	int penalty_increases = 0;

	Matrix<B_DIM> dynviol;

	// penalty loop
	while(penalty_increases < cfg::max_penalty_coeff_increases)
	{
		bool success = minimizeMeritFunction(B, U, problem, output, info, penalty_coeff, trust_box_size);

		double cntviol = 0;
		for(int t = 0; t < T-1; ++t) {
			dynviol = (B[t+1] - beliefDynamics(B[t], U[t]) );
			for(int i = 0; i < B_DIM; ++i) {
				cntviol += fabs(dynviol[i]);
			}
		}
	    success = success && (cntviol < cfg::cnt_tolerance);
	    LOG_DEBUG("Constraint violations: %2.10f",cntviol);
	    if (!success) {
	        penalty_increases++;
	        penalty_coeff = penalty_coeff*cfg::penalty_coeff_increase_ratio;
	        trust_box_size = cfg::initial_trust_box_size;
	    }
	    else {
	    	return computeCost(B, U);
	    }
	}
	return computeCost(B, U);
}



int main(int argc, char* argv[])
{
  //Need xGoal (should only check P_DIM), xMin, xMax, uMin, uMax
	std::vector< Matrix<P_DIM> > waypoints(NUM_WAYPOINTS);
	waypoints[0][0] = 2000; waypoints[0][1] = 0;
	waypoints[1][0] = 4000; waypoints[1][1] = 0;
	waypoints[2][0] = 4000; waypoints[2][1] = 4000;
	waypoints[3][0] = 2000; waypoints[3][1] = 200;
	waypoints[4][0] = 0; waypoints[4][1] = 4000;

	//testPlotting(waypoints);


	std::vector< Matrix<P_DIM> > landmarks(NUM_LANDMARKS);
	landmarks[0][0] = 2000; landmarks[0][1] = 0;
	landmarks[1][0] = 4000; landmarks[1][1] = 0;
	landmarks[2][0] = 4000; landmarks[2][1] = 4000;
	landmarks[3][0] = 2000; landmarks[3][1] = 200;
	landmarks[4][0] = 0; landmarks[4][1] = 4000;


	// for now, landmarks == waypoints
	for(int i = 0; i < NUM_LANDMARKS; ++i) {
		x0.insert(P_DIM+2*i, 0, landmarks[i]);
	}

	SqrtSigma0 = identity<X_DIM>();
	std::vector< std::vector<Matrix<B_DIM> > >B(NUM_WAYPOINTS);
	std::vector< std::vector<Matrix<U_DIM> > >U(NUM_WAYPOINTS);
	for (int i=0; i< NUM_WAYPOINTS; ++i) {
	  std::vector<Matrix<B_DIM> > Bsub(T);
	  B[i] = Bsub;
	}

	Matrix<P_DIM> pGoal;
	pGoal[0] = 0; pGoal[1] = 0;
	x0.insert(0, 0, pGoal);

	vec(x0, SqrtSigma0, B[0][0]);


	beliefPenaltyMPC_params problem;
	beliefPenaltyMPC_output output;
	beliefPenaltyMPC_info info;

	setupBeliefVars(problem, output);

	util::Timer solveTimer;
	util::Timer_tic(&solveTimer);
	
	// B&U optimized in-place


	double cost = 0;
	for(int i = 0; i < NUM_WAYPOINTS; ++i) {
		Matrix<P_DIM> p0 = pGoal;
		pGoal = waypoints[i];

		x0.insert(0, 0, p0);

		if (i > 0)
		  B[i][0] = B[i-1][T-1];

		Matrix<U_DIM> uinit = (pGoal - p0) / (T-1);
		std::vector<Matrix<U_DIM> > Usub(T-1, uinit);
		U[i] = Usub;
		for(int t = 0; t < T - 1; ++t) {
		  B[i][t+1] = beliefDynamics(B[i][t], uinit);
		}
		cost += beliefPenaltyCollocation(B[i], U[i], problem, output, info);
	}



	double solvetime = util::Timer_toc(&solveTimer);
	LOG_INFO("Optimized cost: %4.10f", cost);
	LOG_INFO("Solve time: %5.3f ms", solvetime*1000);
	
	cleanupBeliefMPCVars();
	
	std::vector< Matrix<B_DIM> > Bdisplay(T*NUM_WAYPOINTS);
	std::vector< Matrix<U_DIM> > Udisplay((T-1)*NUM_WAYPOINTS);
	int index = 0;
	for (int i=0; i<NUM_WAYPOINTS; ++i) {
	  for (int t=0; t<T-1; ++t) {
	    Bdisplay[index++] = B[i][t];
	  }
	}
	
	//pythonDisplayTrajectory(B, U);

	/*
	for (size_t t = 0; t < T; ++t) {
		std::cout << ~B[t] << std::endl;
	}
	*/

	//int k;
	//std::cin >> k;

	//CAL_End();
	return 0;
}
