 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    Basis functions (only for NURBS surfaces and NURBS curves)

*******************************************************************************/

#include <memory.h>
#include <stdlib.h>

#include "nurbs_internal.h"
#include "nurbs_basis.h"


static inline void set_zero(NurbsFloat* dest, const int length)
{
    memset(dest, 0, sizeof(NurbsFloat)*length); 
    /* Maybe this is not full portable if the machine is not IEEE754 (very rare)
     * but memset is much more faster!!! */
    /*
    for (i = 0; i <= length; i++)
        dest[i] = 0;
    */
}


/* Finds the lower knot index of the knot interval. */
int nurbs_basis_knot_index
    ( const NurbsFloat t        /* Parameter */
    , const NurbsFloat knots[]  /* Knot vector */
    , const int knots_length    /* Knot vector length */
    )
{
    register int low, mid, high;
    int n = knots_length - 1;

    if (t >= knots[n]){
        return n; /* Special case. It is out of the lower boundary. */
    }
    if (t < knots[0]){
        return -1; /* Special case. It is out of the upper boundary. */
    }

    low = 0;
    high = n;

    /* Performs a bisection search to find the knot intervale */
    mid = (low + high) / 2;
    while (t < knots[mid] || t >= knots[mid+1])
    {
        if (t < knots[mid])  
            high = mid;
        else      
            low = mid;

        mid = (low + high) / 2;
    }

    return(mid);
}

/* Calculate the basis k-degree coeficient at index i for the parameter t */
static NurbsFloat nurbs_basis_calculate_term
    ( const NurbsFloat nk1[]    /* Vector with the k-1 basis coefficient */
    , const NurbsFloat x[]      /* Knot vector */
    , const NurbsFloat t        /* Parameter */
    , const int k               /* Degree */
    , const int i               /* lower index of the knot interval: x[i] < k < x[i+1] */
    , NurbsFloat* bi_1          /* basis of the previous iteration */
    )
{
    NurbsFloat c, nl, nr, xik1, xi1;
    int i1 = i+1;

    nl = *bi_1 * nk1[i];

    xik1 = x[i1 + k];
    xi1 = x[i1];

    if (xik1 > xi1){ /* x[i + k + 1] - x[i + 1] != 0 */
        c = (xik1 - t) / (xik1 - xi1);
        *bi_1 = 1 - c; /* It happens that the term Bi = 1-Ai */
        nr = c * nk1[i1];
    }
    else{
        nr = 0;
        *bi_1 = 0;
    }

    return nl + nr;
}

#if 0
/* Calculate the basis k-degree coeficient at index i for the parameter t.
 * This is the former routine, that maybe it is more clear to follow. */
static NurbsFloat nurbs_basis_calculate_legacy
    ( const NurbsFloat nk1[]    /* Vector with the k-1 basis coefficient */
    , const NurbsFloat x[]      /* Knot vector */
    , const NurbsFloat t        /* Parameter */
    , const int k               /* Degree */
    , const int i               /* knot interval: x[i] < k < x[i+1] */
    )
{
    NurbsFloat a, b, c, d, nl, nr;

    a = (t - x[i]);
    b = x[i + k] - x[i];

    if (b > NURBS_EPSILON){ /* b != 0 */
        nl = (a / b) * nk1[i];
    }
    else{
        nl = 0;
    }

    c = (x[i + k + 1] - t);
    d = x[i + k + 1] - x[i + 1];

    if (d > NURBS_EPSILON){ /* d != 0 */
        nr = (c / d) * nk1[i + 1];
    }
    else{
        nr = 0;
    }

    return nl + nr;
}
#endif

/* Calculates the all basis functions (one for each knot). 
 * Returns the lower knot index interval */
int nurbs_basis_function
    ( NurbsFloat basis[]        /* (out) Basis functions */
    , const NurbsFloat t        /* Parameter value */
    , const int degree          /* Degree */
    , const NurbsFloat knots[]  /* Knot vectors */
    , const int knots_length    /* Knot length */
    )
{
    register int i, k;
    int n;            /* Last index of the knot vector */
    int iknot;        /* Lower index of the knot interval */
    int ik0;          /* Knot_index - degree */
    NurbsFloat bi_1;

    n = knots_length - 1;

    /* Get the lower index of the knot interval */
    iknot = nurbs_basis_knot_index(t, knots, knots_length);

    /* Solution for degree 0 */
    set_zero(basis, knots_length);

    /* If the parameter is out of the knots interval the basis are not defined */
    if (iknot < 0){
        basis[0] = 1;
        return iknot;
    }
    else if (iknot >= n){
        basis[n - degree - 1] = 1;
        return iknot;
    }

    basis[iknot] = 1;

    /* Accumulated calculation of the basis from 1 to k degree */
    for (k = 1; k <= degree; k++){
        ik0 = iknot-k;
        if (ik0 < 0){
            ik0 = 0;
        }

        bi_1 = 0;
        for (i = ik0; i <= iknot; i++){    
            basis[i] = nurbs_basis_calculate_term(basis, knots, t, k, i, &bi_1);
        }
    }

    return iknot;
}

/* Gets the valid interval where the nurbs is defined */
void nurbs_basis_get_parameter_interval
    ( NurbsFloat* first         /* (out) Lower knot interval */
    , NurbsFloat* last          /* (out) Upper knot interval */
    , const NurbsFloat knots[]  /* Knot vector */
    , const int knots_length    /* Knot vector length */
    , const int degree          /* Degree (degree = order - 1) */
    )
{
    *first = knots[degree];
    *last = knots[knots_length - degree - 1];
}


/* Calculates the basis derivative of the k-degree coeficient */
static void nurbs_basis_derivate_term
    ( NurbsFloat* basis         /* (out) Value of the bais function */
    , NurbsFloat* d_basis       /* (out) Value of the basis derivative */
    , const NurbsFloat nk1[]    /* Vector with k-1 degree basis values */
    , const NurbsFloat dk1[]    /* Vector with the derivates of the k-1 basis */
    , const NurbsFloat x[]      /* Knot vector */
    , const NurbsFloat t        /* Parameter */
    , const int k               /* Degree */
    , const int i               /* Lower knot interval: x[i] < k < x[i+1] */
    , NurbsFloat* bi_1          /* Basis of the previous iteration */
    )
{
    NurbsFloat c, d, cd, nl, nr, dnl, dnr, xki, xi;

    /* Calculate left basis term */
    xki = x[k + i];
    xi = x[i];
    if (xki > xi){ /* xki - xi != 0 */
        nl = (*bi_1) * nk1[i];
        dnl = (*bi_1) * dk1[i] + nk1[i] / (xki - xi);
    }
    else{
        nl = 0;
        dnl = 0;
    }

    xki = x[i + k + 1];
    xi = x[i + 1];
    /* Calculate right basis term */
    if (xki > xi){ /* d != 0 */
        c = xki - t;
        d = xki - xi;
        cd = c / d;
        nr = cd * nk1[i + 1];
        dnr = (c * dk1[i + 1] - nk1[i + 1]) / d;
        *bi_1 = 1 - cd;
    }
    else{
        nr = 0;
        dnr = 0;
        *bi_1 = 0;
    }

    *basis = nl + nr;
    *d_basis = dnl + dnr;
}

/* Calculates the basis and the derivate of the basis.
 * Returns the lower index of the knot interval. */
int nurbs_basis_derivate_function
    ( NurbsFloat d_basis[]      /* (out) Derivative of the basis function */
    , NurbsFloat basis[]        /* (out) Basis functions */
    , const NurbsFloat t        /* Parameter */
    , const int degree          /* Degree */
    , const NurbsFloat knots[]  /* Knot vector */
    , const int knots_length    /* Knot vector length */
    )
{
    int i, k;
    const int n = knots_length - 1;  /* last index of the knot vector */
    int iknot;          /* Lower index of the knot interval */
    int ik0;            /* iknot - degree */
    NurbsFloat bi_1;    /* To temporaly store the basis value of the previous iteration */ 

    /* Get the index of the knot interval */
    iknot = nurbs_basis_knot_index(t, knots, knots_length);

    /* Solution for degree 0 */
    set_zero(basis, knots_length);
    set_zero(d_basis, knots_length);

    if (iknot < 0){
        basis[0] = 1;
        return iknot;
    }
    else if (iknot >= n){
        basis[n - degree - 1] = 1;
        return iknot;
    }

    basis[iknot] = 1;

    /* Recursive calculation of basis and derivatives for k degree */
    for (k = 1; k <= degree; k++){
        ik0 = iknot - k;
        if (ik0 < 0){
            ik0 = 0;
        }

        bi_1 = 0;
        for (i = ik0; i <= iknot; i++){
            nurbs_basis_derivate_term
            ( &basis[i], &d_basis[i]
            , basis, d_basis, knots, t, k, i, &bi_1
            );
        }
    }

    return iknot;
}

/* Calculates the basis second derivative of the basis function */
static void nurbs_basis_second_derivate_term
    ( NurbsFloat* basis        /* (out) basis */
    , NurbsFloat* d_basis      /* (out) first basis derivative */
    , NurbsFloat* d2_basis     /* (out) second basis derivative */
    , const NurbsFloat nk1[]   /* Vector with the basis previous iteration */
    , const NurbsFloat dk1[]   /* Vector with the first derivates of k-1  */
    , const NurbsFloat d2k1[]  /* Vector with the second derivates of k-1 */
    , const NurbsFloat x[]     /* Knot vector */
    , const NurbsFloat t       /* Parameter */
    , const int k              /* Degree */
    , const int i              /* knot interval: x[i] < k < x[i+1] */
    )
{
    NurbsFloat a, b, c, d, xki, xi, nl, nr, dnl, d2nl, dnr, d2nr;

    xki = x[i + k];
    xi = x[i];
    if (xki > xi){ /* xki - xi != 0 */
        a = (t - xi);
        b = xki - xi;
        nl = (a / b) * nk1[i];
        dnl = (a*dk1[i] + nk1[i]) / b;
        d2nl = (a*d2k1[i] + 2*dk1[i]) / b;
    }
    else{
        nl = 0;
        dnl = 0;
        d2nl = 0;
    }

    xki = x[i + k + 1];
    xi = x[i + 1];
    if (xki > xi){ /* xki - xi != 0 */
        c = xki - t;
        d = xki - xi;
        nr = (c / d) * nk1[i + 1];
        dnr = (c*dk1[i+1] - nk1[i+1]) / d;
        d2nr = (c*d2k1[i+1] - 2*dk1[i+1]) / d;
    }
    else{
        nr = 0;
        dnr = 0;
        d2nr = 0;
    }

    *basis = nl + nr;
    *d_basis = dnl + dnr;
    *d2_basis = d2nl + d2nr;
}


/* Calculates the basis, the first and the second derivate of the basis func.
 * Returns the lower index of the knot interval. */
int nurbs_basis_second_derivate_function
    ( NurbsFloat d2_basis[]     /* (out) Second derivative of the basis func */
    , NurbsFloat d_basis[]      /* (out) Derivative of the basis function */
    , NurbsFloat basis[]        /* (out) Basis functions */
    , const NurbsFloat t        /* Parameter */
    , const int degree          /* Degree */
    , const NurbsFloat knots[]  /* Knot vector */
    , const int knots_length    /* Knot vector length */
    )
{
    int i, k;
    const int n = knots_length - 1;  /* last index of the knot vector */
    int iknot;    /* Index of knot interval */
    int ik0;

    /* Get the index of the knot interval */
    iknot = nurbs_basis_knot_index(t, knots, knots_length);

    /* Solution for degree 0 */
    set_zero(basis, knots_length);
    set_zero(d_basis, knots_length);
    set_zero(d2_basis, knots_length);

    if (iknot < 0){
        basis[0] = 1;
        return iknot;
    }
    else if (iknot >= n){
        basis[n - degree - 1] = 1;
        return iknot;
    }
    basis[iknot]=1;

    /* Recursive calculation of basis and derivatives for k degree  */
    for (k = 1; k <= degree; k++){
        ik0 = iknot-k;
        if (ik0 < 0){
            ik0 = 0;
        }

        for (i = ik0; i <= iknot; i++){
            nurbs_basis_second_derivate_term
                (&basis[i], &d_basis[i], &d2_basis[i]
                , basis, d_basis, d2_basis, knots, t, k, i);
        }
    }

    return iknot;
}

/* Calculate the basis k-degree coeficient at index ibasis for the parameter t.
 * This is the recursive algorithm to calculate only efficient to calculate one single basis term. 
 * Also it is recursive, and cannot be implemented in all hardware. */
NurbsFloat nurbs_basis_term
    ( const NurbsFloat x[]      /* Knot vector */
    , const NurbsFloat t        /* Parameter */
    , const int degree          /* Degree of the basis*/
    , const int iknot           /* Lower knot interval index: x[i] < k < x[i+1] */
    , const int ibasis          /* Index of the basis */
    )
{
    NurbsFloat a, b, nl=0, nr=0, xki, xi;
    const int k = degree;
    const int iki = iknot - ibasis;

    /* Branch A_ki */
    xki = x[iki+k];
    xi = x[iki];

    if (xki > xi){
        a = (t - xi) / (xki - xi);
    }
    else{
        a = 0;
    }

    /* Branch B_ki-1 = 1-A_ki-1 */
    xki = x[iki+k+1];
    xi = x[iki+1];

    if (xki > xi){
        b = (xki - t) / (xki - xi);
    }
    else{
        b = 0;
    }

    if (degree == 1){
        if (ibasis < degree){
            nl = a;
        }
        if (ibasis > 0){
            nr = b;
        }
    }
    else{
        if (ibasis < degree){
            nl = a * nurbs_basis_term(x, t, degree-1, iknot, ibasis);
        }
        if (ibasis > 0){
            nr = b * nurbs_basis_term(x, t, degree-1, iknot, ibasis-1);
        }
    }

    return nl + nr;
}
