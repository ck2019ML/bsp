/*
FORCES - Fast interior point code generation for multistage problems.
Copyright (C) 2011-14 Alexander Domahidi [domahidi@control.ee.ethz.ch],
Automatic Control Laboratory, ETH Zurich.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __beliefPenaltyMPC_H__
#define __beliefPenaltyMPC_H__


/* DATA TYPE ------------------------------------------------------------*/
typedef double beliefPenaltyMPC_FLOAT;


/* SOLVER SETTINGS ------------------------------------------------------*/
/* print level */
#ifndef beliefPenaltyMPC_SET_PRINTLEVEL
#define beliefPenaltyMPC_SET_PRINTLEVEL    (0)
#endif

/* timing */
#ifndef beliefPenaltyMPC_SET_TIMING
#define beliefPenaltyMPC_SET_TIMING    (0)
#endif

/* Numeric Warnings */
/* #define PRINTNUMERICALWARNINGS */

/* maximum number of iterations  */
#define beliefPenaltyMPC_SET_MAXIT         (30)	

/* scaling factor of line search (affine direction) */
#define beliefPenaltyMPC_SET_LS_SCALE_AFF  (0.9)      

/* scaling factor of line search (combined direction) */
#define beliefPenaltyMPC_SET_LS_SCALE      (0.95)  

/* minimum required step size in each iteration */
#define beliefPenaltyMPC_SET_LS_MINSTEP    (1E-08)

/* maximum step size (combined direction) */
#define beliefPenaltyMPC_SET_LS_MAXSTEP    (0.995)

/* desired relative duality gap */
#define beliefPenaltyMPC_SET_ACC_RDGAP     (0.0001)

/* desired maximum residual on equality constraints */
#define beliefPenaltyMPC_SET_ACC_RESEQ     (1E-06)

/* desired maximum residual on inequality constraints */
#define beliefPenaltyMPC_SET_ACC_RESINEQ   (1E-06)

/* desired maximum violation of complementarity */
#define beliefPenaltyMPC_SET_ACC_KKTCOMPL  (1E-06)


/* RETURN CODES----------------------------------------------------------*/
/* solver has converged within desired accuracy */
#define beliefPenaltyMPC_OPTIMAL      (1)

/* maximum number of iterations has been reached */
#define beliefPenaltyMPC_MAXITREACHED (0)

/* no progress in line search possible */
#define beliefPenaltyMPC_NOPROGRESS   (-7)




/* PARAMETERS -----------------------------------------------------------*/
/* fill this with data before calling the solver! */
typedef struct beliefPenaltyMPC_params
{
    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f1[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb1[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub1[7];

    /* matrix of size [10 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C1[170];

    /* vector of size 10 */
    beliefPenaltyMPC_FLOAT e1[10];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f2[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb2[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub2[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C2[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e2[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f3[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb3[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub3[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C3[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e3[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f4[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb4[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub4[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C4[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e4[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f5[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb5[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub5[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C5[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e5[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f6[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb6[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub6[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C6[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e6[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f7[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb7[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub7[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C7[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e7[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f8[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb8[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub8[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C8[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e8[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f9[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb9[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub9[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C9[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e9[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f10[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb10[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub10[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C10[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e10[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f11[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb11[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub11[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C11[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e11[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f12[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb12[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub12[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C12[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e12[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f13[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb13[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub13[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C13[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e13[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f14[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb14[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub14[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C14[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e14[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f15[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb15[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub15[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C15[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e15[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f16[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb16[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub16[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C16[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e16[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f17[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb17[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub17[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C17[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e17[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f18[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb18[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub18[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C18[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e18[5];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT f19[17];

    /* vector of size 17 */
    beliefPenaltyMPC_FLOAT lb19[17];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT ub19[7];

    /* matrix of size [5 x 17] (column major format) */
    beliefPenaltyMPC_FLOAT C19[85];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT e19[5];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT lb20[5];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT ub20[5];

} beliefPenaltyMPC_params;


/* OUTPUTS --------------------------------------------------------------*/
/* the desired variables are put here by the solver */
typedef struct beliefPenaltyMPC_output
{
    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z1[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z2[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z3[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z4[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z5[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z6[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z7[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z8[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z9[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z10[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z11[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z12[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z13[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z14[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z15[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z16[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z17[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z18[7];

    /* vector of size 7 */
    beliefPenaltyMPC_FLOAT z19[7];

    /* vector of size 5 */
    beliefPenaltyMPC_FLOAT z20[5];

} beliefPenaltyMPC_output;


/* SOLVER INFO ----------------------------------------------------------*/
/* diagnostic data from last interior point step */
typedef struct beliefPenaltyMPC_info
{
    /* iteration number */
    int it;
	
    /* inf-norm of equality constraint residuals */
    beliefPenaltyMPC_FLOAT res_eq;
	
    /* inf-norm of inequality constraint residuals */
    beliefPenaltyMPC_FLOAT res_ineq;

    /* primal objective */
    beliefPenaltyMPC_FLOAT pobj;	
	
    /* dual objective */
    beliefPenaltyMPC_FLOAT dobj;	

    /* duality gap := pobj - dobj */
    beliefPenaltyMPC_FLOAT dgap;		
	
    /* relative duality gap := |dgap / pobj | */
    beliefPenaltyMPC_FLOAT rdgap;		

    /* duality measure */
    beliefPenaltyMPC_FLOAT mu;

	/* duality measure (after affine step) */
    beliefPenaltyMPC_FLOAT mu_aff;
	
    /* centering parameter */
    beliefPenaltyMPC_FLOAT sigma;
	
    /* number of backtracking line search steps (affine direction) */
    int lsit_aff;
    
    /* number of backtracking line search steps (combined direction) */
    int lsit_cc;
    
    /* step size (affine direction) */
    beliefPenaltyMPC_FLOAT step_aff;
    
    /* step size (combined direction) */
    beliefPenaltyMPC_FLOAT step_cc;    

	/* solvertime */
	beliefPenaltyMPC_FLOAT solvetime;   

} beliefPenaltyMPC_info;


/* SOLVER FUNCTION DEFINITION -------------------------------------------*/
/* examine exitflag before using the result! */
int beliefPenaltyMPC_solve(beliefPenaltyMPC_params* params, beliefPenaltyMPC_output* output, beliefPenaltyMPC_info* info);


#endif