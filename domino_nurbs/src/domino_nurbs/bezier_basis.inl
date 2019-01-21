 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    The control box uses its own basis formulation. 
    I was not able yet to generalize the formulation, but I am working on that.
    They are close related with Bezier and Hermite basis.
    The main difference with conventional NURBS is that the curve or surface,
    always pass through a control point.

*******************************************************************************/


#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include "nurbs_internal.h"
#include "nurbs_controlbox.h"



/* Calculate the basis for order 2 */
static void bezier_basis2L
    ( NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3;
    const NurbsFloat coeff[4][4] = 
    {  { 0,  0,  0,  0 }, 
 	   { 2, -2,	-1,	 1 }, 
 	   { 0,  2,  2, -2 }, 
 	   { 0, -0,	-1,  1 }
    };
    
    t2 = t*t;
    t3 = t2*t;

    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3];
    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3];
    *basis2 = coeff[3][0] + t*coeff[3][1] + t2*coeff[3][2] + t3*coeff[3][3];

    *basis0 /= 2;
    *basis1 /= 2;
    *basis2 /= 2;
}

/* Calculate the derivative basis for order 2 */
static void bezier_d_basis2L
    ( NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2;
    const NurbsFloat coeff[4][4] = 
    {
       { 0,  0,  0,  0 }, 
       { 2, -2,	-1,	 1 }, 
       { 0,  2,  2, -2 }, 
       { 0, -0,	-1,	 1 }
    };
    
    t2 = t*t;

    *basis0 = coeff[1][1] + 2*t*coeff[1][2] + 3*t2*coeff[1][3];
    *basis1 = coeff[2][1] + 2*t*coeff[2][2] + 3*t2*coeff[2][3];
    *basis2 = coeff[3][1] + 2*t*coeff[3][2] + 3*t2*coeff[3][3];

    *basis0 /= 2;
    *basis1 /= 2;
    *basis2 /= 2;
}


/* Calculate the second derivative basis for order 2 */
static void bezier_d2_basis2L
    ( NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    const NurbsFloat coeff[4][4] = 
    {
       { 0,  0,  0,  0 }, 
       { 2, -2, -1,	 1 }, 
       { 0,  2,  2, -2 }, 
       { 0, -0,	-1,	 1 }
    };

    *basis0 = coeff[1][2] + 3*t*coeff[1][3];
    *basis1 = coeff[2][2] + 3*t*coeff[2][3];
    *basis2 = coeff[3][2] + 3*t*coeff[3][3];
}


/* Calculate the basis for order 2 */
static void bezier_basis2C
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3;
    const NurbsFloat coeff[4][4] = 
    {
 	    { 0, -1,  2, -1 }, 
 	    { 2,  0, -5,  3 }, 
 	    { 0,  1,  4, -3 }, 
 	    { 0,  0, -1,  1 } 
    };
    
    t2 = t*t;
    t3 = t2*t;

    *basis_1 = coeff[0][0] + t*coeff[0][1] + t2*coeff[0][2] + t3*coeff[0][3];
    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3];
    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3];
    *basis2 = coeff[3][0] + t*coeff[3][1] + t2*coeff[3][2] + t3*coeff[3][3];

    *basis_1 /= 2;
    *basis0 /= 2;
    *basis1 /= 2;
    *basis2 /= 2;
}


/* Calculate the derivative basis for order 2 */
static void bezier_d_basis2C
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2;
    const NurbsFloat coeff[4][4] = 
    {
        { 0, -1,  2, -1 }, 
        { 2,  0, -5,  3 }, 
        { 0,  1,  4, -3 }, 
        { 0,  0, -1,  1 } 
    };
    
    t2 = t*t;

    *basis_1 = coeff[0][1] + 2*t*coeff[0][2] + 3*t2*coeff[0][3];
    *basis0 = coeff[1][1] + 2*t*coeff[1][2] + 3*t2*coeff[1][3];
    *basis1 = coeff[2][1] + 2*t*coeff[2][2] + 3*t2*coeff[2][3];
    *basis2 = coeff[3][1] + 2*t*coeff[3][2] + 3*t2*coeff[3][3];

    *basis_1 /= 2;
    *basis0 /= 2;
    *basis1 /= 2;
    *basis2 /= 2;
}


/* Calculate the second derivative basis for order 2 */
static void bezier_d2_basis2C
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    const NurbsFloat coeff[4][4] = 
    {
        { 0, -1,  2, -1 }, 
        { 2,  0, -5,  3 }, 
        { 0,  1,  4, -3 }, 
        { 0,  0, -1,  1 } 
    };

    *basis_1 = coeff[0][2] + 3*t*coeff[0][3];
    *basis0 = coeff[1][2] + 3*t*coeff[1][3];
    *basis1 = coeff[2][2] + 3*t*coeff[2][3];
    *basis2 = coeff[3][2] + 3*t*coeff[3][3];
}


/* Calculate the basis for order 3 */
static void bezier_basis2R
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3;
    const NurbsFloat coeff[4][4] = 
    {
        { 0, -1,  2, -1 }, 
        { 2,  0, -4,  2 }, 
        { 0,  1,  2, -1 }, 
        { 0,  0,  0,  0 }
    };
    
    t2 = t*t;
    t3 = t2*t;

    *basis_1 = coeff[0][0] + t*coeff[0][1] + t2*coeff[0][2] + t3*coeff[0][3];
    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3];
    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3];

    *basis_1 /= 2;
    *basis0 /= 2;
    *basis1 /= 2;
}


/* Calculate the basis for order 3 */
static void bezier_d_basis2R
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2;
    const NurbsFloat coeff[4][4] = 
    {
 	   { 0, -1, 2, -1 }, 
 	   { 2, 0, -4,  2 }, 
 	   { 0, 1, 	2, -1 }, 
 	   { 0, 0, 	0, 	0 }
    };
    
    t2 = t*t;

    *basis_1 = coeff[0][1] + 2*t*coeff[0][2] + 3*t2*coeff[0][3];
    *basis0 = coeff[1][1] + 2*t*coeff[1][2] + 3*t2*coeff[1][3];
    *basis1 = coeff[2][1] + 2*t*coeff[2][2] + 3*t2*coeff[2][3];

    *basis_1 /= 2;
    *basis0 /= 2;
    *basis1 /= 2;
}


/* Calculate the basis for order 3 */
static void bezier_d2_basis2R
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    const NurbsFloat coeff[4][4] = 
    {
        { 0, -1,  2, -1 }, 
        { 2,  0, -4,  2 }, 
        { 0,  1,  2, -1 }, 
        { 0,  0,  0,  0 }
    };
    
    *basis_1 = coeff[0][2] + 3*t*coeff[0][3];
    *basis0 = coeff[1][2] + 3*t*coeff[1][3];
    *basis1 = coeff[2][2] + 3*t*coeff[2][3];
}


/* Calculate the basis for order 3 */
static void bezier_basis3L
    ( NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3, t4, t5;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,   0,   0, 	 0,   0,   0 },
        { 1,  -1.5, 0.5, 0,	  0,   0 },
        { 0,   2, 	-1,  0,   0,   0 },
        { 0,  -0.5, 0.5, 0,   0,   0 }
    };
    
    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;
    t5 = t4*t;

    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3] 
        + t4*coeff[1][4] + t5*coeff[1][5];

    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3] 
        + t4*coeff[2][4] + t5*coeff[2][5];

    *basis2 = coeff[3][0] + t*coeff[3][1] + t2*coeff[3][2] + t3*coeff[3][3] 
        + t4*coeff[3][4] + t5*coeff[3][5];
}


/* Calculate the basis for order 3 */
static void bezier_d_basis3L
    ( NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t1, t2, t3, t4;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,   0,   0, 	 0,   0,   0 },
        { 1,  -1.5, 0.5, 0,	  0,   0 },
        { 0,   2, 	-1,  0,   0,   0 },
        { 0,  -0.5, 0.5, 0,   0,   0 }
    };
    
    t1 = t;
    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;

    t1 *= 2;
    t2 *= 3;
    t3 *= 4;
    t4 *= 5;

    *basis0 = coeff[1][1] + t1*coeff[1][2] + t2*coeff[1][3] 
        + t3*coeff[1][4] + t4*coeff[1][5];

    *basis1 = coeff[2][1] + t1*coeff[2][2] + t2*coeff[2][3] 
        + t3*coeff[2][4] + t4*coeff[2][5];

    *basis2 = coeff[3][1] + t1*coeff[3][2] + t2*coeff[3][3] 
        + t3*coeff[3][4] + t4*coeff[3][5];
}


/* Calculate the basis for order 3 */
static void bezier_d2_basis3L
    ( NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t1, t2, t3;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,   0,   0, 	 0,   0,   0 },
        { 1,  -1.5, 0.5, 0,	  0,   0 },
        { 0,   2, 	-1,  0,   0,   0 },
        { 0,  -0.5, 0.5, 0,   0,   0 }
    };
    
    t1 = t;
    t2 = t*t;
    t3 = t2*t;

    t1 *= 3;
    t2 *= 6;
    t3 *= 10;

    *basis0 = coeff[1][2] + t1*coeff[1][3] + t2*coeff[1][4] + t3*coeff[1][5];
    *basis1 = coeff[2][2] + t1*coeff[2][3] + t2*coeff[2][4] + t3*coeff[2][5];
    *basis2 = coeff[3][2] + t1*coeff[3][3] + t2*coeff[3][4] + t3*coeff[3][5];
}


/* Calculate the basis for order 3 */
static void bezier_basis3C
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3, t4, t5;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,  -1,   1,   3,  -5,   2 },
        { 2,   0,  -2,  -9,  15,  -6 },
        { 0,   1, 	1,   9, -15,   6 },
        { 0,   0,   0,  -3,   5,  -2 }
    };
    
    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;
    t5 = t4*t;

    *basis_1 = coeff[0][0] + t*coeff[0][1] + t2*coeff[0][2] + t3*coeff[0][3] 
        + t4*coeff[0][4] + t5*coeff[0][5];

    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3] 
        + t4*coeff[1][4] + t5*coeff[1][5];

    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3] 
        + t4*coeff[2][4] + t5*coeff[2][5];

    *basis2 = coeff[3][0] + t*coeff[3][1] + t2*coeff[3][2] + t3*coeff[3][3] 
        + t4*coeff[3][4] + t5*coeff[3][5];

    *basis_1 /= 2;
    *basis0 /= 2;
    *basis1 /= 2;
    *basis2 /= 2;
}


/* Calculate the basis for order 3 */
static void bezier_d_basis3C
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t1, t2, t3, t4;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,  -1,   1,   3,  -5,   2 },
        { 2,   0,  -2,  -9,  15,  -6 },
        { 0,   1, 	1,   9, -15,   6 },
        { 0,   0,   0,  -3,   5,  -2 }
    };
    
    t1 = t;
    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;

    t1 *= 2;
    t2 *= 3;
    t3 *= 4;
    t4 *= 5;

    *basis_1 = coeff[0][1] + t1*coeff[0][2] + t2*coeff[0][3] 
        + t3*coeff[0][4] + t4*coeff[0][5];

    *basis0 = coeff[1][1] + t1*coeff[1][2] + t2*coeff[1][3] 
        + t3*coeff[1][4] + t4*coeff[1][5];

    *basis1 = coeff[2][1] + t1*coeff[2][2] + t2*coeff[2][3] 
        + t3*coeff[2][4] + t4*coeff[2][5];

    *basis2 = coeff[3][1] + t1*coeff[3][2] + t2*coeff[3][3] 
        + t3*coeff[3][4] + t4*coeff[3][5];

    *basis_1 /= 2;
    *basis0 /= 2;
    *basis1 /= 2;
    *basis2 /= 2;
}


/* Calculate the basis for order 3 */
static void bezier_d2_basis3C
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t1, t2, t3;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,  -1,   1,   3,  -5,   2 },
        { 2,   0,  -2,  -9,  15,  -6 },
        { 0,   1, 	1,   9, -15,   6 },
        { 0,   0,   0,  -3,   5,  -2 }
    };
    
    t1 = t;
    t2 = t*t;
    t3 = t2*t;

    t1 *= 3;
    t2 *= 6;
    t3 *= 10;

    *basis_1 = coeff[0][2] + t1*coeff[0][3] + t2*coeff[0][4] + t3*coeff[0][5];
    *basis0 = coeff[1][2] + t1*coeff[1][3] + t2*coeff[1][4] + t3*coeff[1][5];
    *basis1 = coeff[2][2] + t1*coeff[2][3] + t2*coeff[2][4] + t3*coeff[2][5];
    *basis2 = coeff[3][2] + t1*coeff[3][3] + t2*coeff[3][4] + t3*coeff[3][5];
}


/* Calculate the basis for order 3 */
static void bezier_basis3R
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3, t4, t5;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,  -0.5, 0.5, 0,  0,  0 },
        { 1,   0,  -1,   0,  0,  0 },
        { 0,   0.5, 0.5, 0,  0,  0 },
        { 0,   0,   0,   0,   0,   0 }
    };
    
    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;
    t5 = t4*t;

    *basis_1 = coeff[0][0] + t*coeff[0][1] + t2*coeff[0][2] + t3*coeff[0][3] 
        + t4*coeff[0][4] + t5*coeff[0][5];

    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3] 
        + t4*coeff[1][4] + t5*coeff[1][5];

    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3] 
        + t4*coeff[2][4] + t5*coeff[2][5];
}


/* Calculate the basis for order 3 */
static void bezier_d_basis3R
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t1, t2, t3, t4;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,  -0.5, 0.5, 0,  0,  0 },
        { 1,   0,  -1,   0,  0,  0 },
        { 0,   0.5, 0.5, 0,  0,  0 },
        { 0,   0,   0,   0,   0,   0 }
    };
    
    t1 = t;
    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;

    t1 *= 2;
    t2 *= 3;
    t3 *= 4;
    t4 *= 5;

    *basis_1 = coeff[0][1] + t1*coeff[0][2] + t2*coeff[0][3] 
        + t3*coeff[0][4] + t4*coeff[0][5];

    *basis0 = coeff[1][1] + t1*coeff[1][2] + t2*coeff[1][3] 
        + t3*coeff[1][4] + t4*coeff[1][5];

    *basis1 = coeff[2][1] + t1*coeff[2][2] + t2*coeff[2][3] 
        + t3*coeff[2][4] + t4*coeff[2][5];
}


/* Calculate the basis for order 3 */
static void bezier_d2_basis3R
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t1, t2, t3;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,  -0.5, 0.5, 0,  0,  0 },
        { 1,   0,  -1,   0,  0,  0 },
        { 0,   0.5, 0.5, 0,  0,  0 },
        { 0,   0,   0,   0,   0,   0 }
    };
    
    t1 = t;
    t2 = t*t;
    t3 = t2*t;

    t1 *= 3;
    t2 *= 6;
    t3 *= 10;

    *basis_1 = coeff[0][2] + t1*coeff[0][3] + t2*coeff[0][4] + t3*coeff[0][5];
    *basis0 = coeff[1][2] + t1*coeff[1][3] + t2*coeff[1][4] + t3*coeff[1][5];
    *basis1 = coeff[2][2] + t1*coeff[2][3] + t2*coeff[2][4] + t3*coeff[2][5];
}


/* Calculates the basis functions. */
static int bezier_basis
    ( NurbsFloat basis[]        /* (out) Basis functions */
    , const NurbsFloat t        /* Parameter value */
    , const int num_cp          /* Number of control points */
    , const int order           /* Order */
    )
{
    int iknot = 0;
    NurbsFloat tn = t * (num_cp-1);
    int icp = (int)floor( tn );
    NurbsFloat t01 = tn - icp;

    memset( basis, 0, sizeof(NurbsFloat)*num_cp );

    if (t < 0){
        t01 = 0;
        icp = 0;
    }
    if (t >= 1){
        t01 = 1;
        icp = num_cp - 2;
    }

    if (order <= 0){
        if (icp > num_cp - 2){
            icp = num_cp - 2;
        }
        if (icp < 0){
            icp = 0;
        }
        basis[icp] = 1;
        iknot = icp;
    }
    else if (order == 1){
        if (icp > num_cp - 2){
            icp = num_cp - 2;
        }
        basis[icp] = 1 - t01;
        basis[icp+1] = t01;
        iknot = icp;
    }
    else if (order == 2){
        if (icp == 0){
            bezier_basis2L( &basis[0], &basis[1], &basis[2], t01 );
            iknot = 0;
        }
        else if (icp >= num_cp - 2){
            bezier_basis2R
                ( &basis[num_cp-3], &basis[num_cp-2], &basis[num_cp-1], t01 );
            iknot = num_cp - 4;
        }
        else{
            bezier_basis2C
                ( &basis[icp-1], &basis[icp], &basis[icp+1], &basis[icp+2], t01 );
            iknot = icp - 1;
        }
    }
    else if (order == 3){
        if (icp == 0){
            bezier_basis3L( &basis[0], &basis[1], &basis[2], t01 );
            iknot = icp;
        }
        else if (icp >= num_cp - 2){
            bezier_basis3R
                ( &basis[num_cp-3], &basis[num_cp-2], &basis[num_cp-1], t01 );
            iknot = num_cp - 4;
        }
        else{
            bezier_basis3C
                ( &basis[icp-1], &basis[icp], &basis[icp+1], &basis[icp+2], t01 );
            iknot = icp - 1;
        }
    }

    return iknot;
}


/* Calculates the derivative of the basis. */
static int bezier_d_basis
    ( NurbsFloat d_basis[]        /* (out) Derivative of the basis functions */
    , const NurbsFloat t        /* Parameter value */
    , const int num_cp          /* Number of control points */
    , const int order           /* Order */
    )
{
    int i, iknot = 0;
    int ncp_1 = num_cp - 1;
    NurbsFloat tn = t * (ncp_1);
    int icp = (int)floor( tn );
    NurbsFloat t01 = tn - icp;

    for (i = 0; i < num_cp; i++){
        d_basis[i] = 0;
    }

    if (t < 0){
        return 0;
    }
    if (t > 1){
        if (order == 0){ 
            return num_cp - 2;
        }
        else if (order == 1){
            return num_cp - 2;
        }
        else if (order == 2 || order == 3){
            return num_cp - 4;
        }
    }

    if (order <= 0){
        if (icp > num_cp - 2){
            icp = num_cp - 2;
        }
        if (icp < 0){
            icp = 0;
        }
        iknot = icp;
    }
    else if (order == 1){
        if (icp > num_cp - 2){
            icp = num_cp - 2;
        }
        d_basis[icp] = -ncp_1;
        d_basis[icp+1] = ncp_1;
        iknot = icp;
    }
    else if (order == 2){
        if (icp == 0){
            bezier_d_basis2L( &d_basis[0], &d_basis[1], &d_basis[2], t01 );
            d_basis[0] *= ncp_1;
            d_basis[1] *= ncp_1;
            d_basis[2] *= ncp_1;

            iknot = 0;
        }
        else if (icp >= num_cp - 2){
            bezier_d_basis2R
                ( &d_basis[num_cp-3], &d_basis[num_cp-2], &d_basis[num_cp-1], t01 );

            d_basis[num_cp-3] *= ncp_1;
            d_basis[num_cp-2] *= ncp_1;
            d_basis[num_cp-1] *= ncp_1;

            iknot = num_cp - 4;
        }
        else{
            bezier_d_basis2C
                ( &d_basis[icp-1], &d_basis[icp], &d_basis[icp+1], &d_basis[icp+2], t01 );

            d_basis[icp-1] *= ncp_1;
            d_basis[icp] *= ncp_1;
            d_basis[icp+1] *= ncp_1;
            d_basis[icp+2] *= ncp_1;

            iknot = icp - 1;
        }
    }
    else if (order == 3){
        if (icp == 0){
            bezier_d_basis3L( &d_basis[0], &d_basis[1], &d_basis[2], t01 );
            d_basis[0] *= ncp_1;
            d_basis[1] *= ncp_1;
            d_basis[2] *= ncp_1;

            iknot = icp;
        }
        else if (icp >= num_cp - 2){
            bezier_d_basis3R
                ( &d_basis[num_cp-3], &d_basis[num_cp-2], &d_basis[num_cp-1], t01 );
            d_basis[num_cp-3] *= ncp_1;
            d_basis[num_cp-2] *= ncp_1;
            d_basis[num_cp-1] *= ncp_1;

            iknot = num_cp - 4;
        }
        else{
            bezier_d_basis3C
                ( &d_basis[icp-1], &d_basis[icp], &d_basis[icp+1], &d_basis[icp+2], t01 );
            d_basis[icp-1] *= ncp_1;
            d_basis[icp] *= ncp_1;
            d_basis[icp+1] *= ncp_1;
            d_basis[icp+2] *= ncp_1;

            iknot = icp - 1;
        }
    }

    return iknot;
}


/* Calculates the second derivative of the basis. */
static int bezier_d2_basis
    ( NurbsFloat d2_basis[]   /* (out) Second derivative of the basis functions */
    , const NurbsFloat t        /* Parameter value */
    , const int num_cp          /* Number of control points */
    , const int order           /* Order */
    )
{
    int i, iknot = 0;
    int ncp_1 = num_cp - 1;
    NurbsFloat tn = t * ncp_1;
    int icp = (int)floor( tn );
    NurbsFloat t01 = tn - icp;

    for (i = 0; i < num_cp; i++){
        d2_basis[i] = 0;
    }

    if (t < 0){
        t01 = 0;
        icp = 0;
    }
    if (t > 1 || icp > num_cp - 2){
        t01 = 1;
        icp = num_cp - 2;
    }

    if (order <= 0){
        d2_basis[icp] = 0;
        iknot = icp;
    }
    else if (order == 1){
        d2_basis[icp] = 0;
        d2_basis[icp+1] = 0;
        iknot = icp;
    }
    else if (order == 2){
        if (icp == 0){
            bezier_d2_basis2L( &d2_basis[0], &d2_basis[1], &d2_basis[2], t01 );
            d2_basis[0] *= ncp_1;
            d2_basis[1] *= ncp_1;
            d2_basis[2] *= ncp_1;

            iknot = 0;
        }
        else if (icp == num_cp - 2){
            bezier_d2_basis2R
                ( &d2_basis[num_cp-3], &d2_basis[num_cp-2], &d2_basis[num_cp-1], t01 );
            d2_basis[num_cp-3] *= ncp_1;
            d2_basis[num_cp-2] *= ncp_1;
            d2_basis[num_cp-1] *= ncp_1;

            iknot = num_cp - 4;
        }
        else{
            bezier_d2_basis2C
                ( &d2_basis[icp-1], &d2_basis[icp], &d2_basis[icp+1], &d2_basis[icp+2], t01 );
            d2_basis[icp-1] *= ncp_1;
            d2_basis[icp] *= ncp_1;
            d2_basis[icp+1] *= ncp_1;
            d2_basis[icp+2] *= ncp_1;

            iknot = icp - 1;
        }
    }
    else if (order == 3){
        if (icp == 0){
            bezier_d2_basis3L( &d2_basis[0], &d2_basis[1], &d2_basis[2], t01 );
            d2_basis[0] *= ncp_1;
            d2_basis[1] *= ncp_1;
            d2_basis[2] *= ncp_1;

            iknot = icp;
        }
        else if (icp == num_cp - 2){
            bezier_d2_basis3R
                ( &d2_basis[num_cp-3], &d2_basis[num_cp-2], &d2_basis[num_cp-1], t01 );

            d2_basis[num_cp-3] *= ncp_1;
            d2_basis[num_cp-2] *= ncp_1;
            d2_basis[num_cp-1] *= ncp_1;

            iknot = num_cp - 4;
        }
        else{
            bezier_d2_basis3C
                ( &d2_basis[icp-1], &d2_basis[icp], &d2_basis[icp+1], &d2_basis[icp+2], t01 );

            d2_basis[icp-1] *= ncp_1;
            d2_basis[icp] *= ncp_1;
            d2_basis[icp+1] *= ncp_1;
            d2_basis[icp+2] *= ncp_1;

            iknot = icp - 1;
        }
    }

    return iknot;
}

/**/