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

#include "common/check_malloc.h"
#include "common/log.h"

#include "nurbs_internal.h"
#include "nurbs_basis.h"
#include "nurbs_controlbox.h"

#include "bezier_basis.inl"

/* Calculate the basis k-degree coeficient at index i for the parameter t */
static NurbsFloat uniform_basis_term
    ( const NurbsFloat nk1[]    /* Vector with the k-1 basis coefficient */
    , const int up_knot         /* Upper limit of the knots */
    , const NurbsFloat t        /* Parameter */
    , const int order           /* Order */
    , const int k               /* Iteration */
    , const int i               /* knot interval: x[i] < k < x[i+1] */
    , NurbsFloat* bi_1          /* basis of the previous iteration */
    )
{
    NurbsFloat c, nl, nr;
    int xik1, xi1;

    nl = *bi_1 * nk1[i];

    xi1 = i + 1 - order;
    xik1 = xi1 + k;

    if (xik1 > up_knot){
        xik1 = up_knot;
    }
    if (xi1 < 0){
        xi1 = 0;
    }

    if (xik1 > xi1){ /* x[i + k + 1] - x[i + 1] != 0 */
        c = (xik1 - t) / (xik1 - xi1);
        *bi_1 = 1 - c;
        nr = c * nk1[i+1];
    }
    else{
        nr = 0;
        *bi_1 = 0;
    }

    return nl + nr;
}

    /* Calculate the basis k-degree coeficient at index i for the parameter t */
static void nurbs_uniform_derivative_basis_term
    ( NurbsFloat* basis
    , NurbsFloat* d_basis
    , const NurbsFloat nk1[]    /* Vector with k-1 degree basis values */
    , const NurbsFloat dk1[]    /* Vector with the derivates of the k-1 basis */
    , const int up_knot         /* Upper limit of the knots */
    , const NurbsFloat t        /* Parameter */
    , const int order           /* Order */
    , const int k               /* Iteration */
    , const int i               /* knot interval: x[i] < k < x[i+1] */
    , NurbsFloat* bi_1          /* basis of the previous iteration */
    )
{
    NurbsFloat c, cd, nl, nr, dnl, dnr;
    int d;
    int xik, xi, xik1, xi1;

    /* Calculate basis */
    xi = i - order;
    xik = xi + k;

    if (xi < 0){
        xi = 0;
    }
    if (xik > up_knot){
        xik = up_knot;
    }

    if (xik > xi){ /* xki - xi != 0 */
        nl = (*bi_1) * nk1[i];
        dnl = (*bi_1) * dk1[i] + nk1[i] / (xik - xi);
    }
    else{
        nl = 0;
        dnl = 0;
    }

    xi1 = i + 1 - order;
    xik1 = xi1 + k;

    if (xi1 < 0){
        xi1 = 0;
    }
    if (xik1 > up_knot){
        xik1 = up_knot;
    }

    if (xik1 > xi1){ /* d != 0 */
        c = xik1 - t;
        d = xik1 - xi1;
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

    /* Calculate d_basis */

    *d_basis = (dnl + dnr);
}


/* Calculates the derivative of the basis. */
static int nurbs_uniform_d_basis
    ( NurbsFloat d_basis[]      /* (out) Derivative of the basis functions */
    , NurbsFloat basis[]        /* (out) Basis functions */
    , const NurbsFloat t        /* Parameter value */
    , const int num_cp          /* Number of control points */
    , const int order           /* Order */
    )
{
    register int i, k;
    int ik0;      
    NurbsFloat bi_1;
    int up_knot = num_cp - order;   /* Upper knot */
    NurbsFloat tx = t * up_knot;
    int iknot = (int)(tx); /* Index of knot interval */

    for (i = 0; i < num_cp; i++){
        d_basis[i] = 0;
        basis[i] = 0;
    }

    /* Especial case when the parameter is out of the knots interval. */
    if (t < 0){ 
        tx = 0;
        iknot = 0;
    }
    else if (t >= 1){
        tx = up_knot;
        iknot = num_cp-order-1;
    }

    /* Solution basis order 0 */
    basis[iknot + order] = 1;

    /* Recursive calculation of basis coefficients for k degree */
    for (k = 1; k <= order; k++){
        ik0 = iknot + order - k;

        bi_1 = 0;
        for (i = ik0; i <= iknot + order; i++){    
            nurbs_uniform_derivative_basis_term(&(basis[i]), &(d_basis[i])
                , basis, d_basis, up_knot, tx, order, k, i, &bi_1);
        }
    }

    return iknot;
}


/* Calculates the basis functions. */
int nurbs_uniform_basis
    ( NurbsFloat basis[]        /* (out) Basis functions */
    , const NurbsFloat t        /* Parameter value */
    , const int num_cp          /* Number of control points */
    , const int order           /* Order */
    )
{
    register int i, k;
    int ik0;      
    NurbsFloat bi_1;
    int up_knot = num_cp - order;   /* Upper knot */
    NurbsFloat tx = t * up_knot;
    int iknot = (int)(tx); /* Index of knot interval */

    for (i = 0; i < num_cp; i++){
        basis[i] = 0;
    }

    /* Especial case when the parameter is out of the knots interval. */
    if (t < 0){ 
        basis[0] = 1;
        return 0;
    }
    else if (t >= 1){
        basis[num_cp-1] = 1;
        return num_cp-order-1;
    }

    /* Solution basis order 0 */
    basis[iknot + order] = 1;

    /* Recursive calculation of basis coefficients for k degree */
    for (k = 1; k <= order; k++){
        ik0 = iknot + order - k;

        bi_1 = 0;
        for (i = ik0; i <= iknot + order; i++){    
            basis[i] = uniform_basis_term
                (basis, up_knot, tx, order, k, i, &bi_1);
        }
    }

    return iknot;
}


/* Calculates the basis functions. */
int nurbs_controlbox_basis_function
    ( NurbsFloat basis[]        /* (out) Basis functions */
    , const NurbsFloat t        /* Parameter value */
    , const int num_cp          /* Number of control points */
    , const int order           /* Order */
    , const int basis_equation  /* Basis equation (1:bezier 0:b-spline) */
    )
{
    if (basis_equation == 1){
        return bezier_basis( basis, t, num_cp, order );
    }
    else{
        return nurbs_uniform_basis( basis, t, num_cp, order );
    }
}


/* Calculates the derivative of the basis. */
int nurbs_controlbox_d_basis_function
    ( NurbsFloat d_basis[]      /* (out) Derivative of the basis functions */
    , NurbsFloat basis[]        /* (out) Basis functions */
    , const NurbsFloat t        /* Parameter value */
    , const int num_cp          /* Number of control points */
    , const int order           /* Order */
    , const int basis_equation  /* Basis equation */
    )
{
    if (basis_equation == 1){
        bezier_d_basis( d_basis, t, num_cp, order );
        return bezier_basis( basis, t, num_cp, order );
    }
    else{
        return nurbs_uniform_d_basis( d_basis, basis, t, num_cp, order );
    }
}


/* Equivalent to a default constructor */
void nurbs_controlbox_init( NurbsControlBox* ffd )
{
    ffd->label[0] = '\0';
    ffd->id = -1;

    ffd->cp_length_u = 0;
    ffd->cp_length_v = 0;
    ffd->cp_length_w = 0;

    ffd->cp_stream = nullptr;
    ffd->cp = nullptr;

    ffd->basis_equation = 0;

    ffd->next = nullptr;
}


/* Initializes a control box data structure. If nullptr is pass 
 * as a first argument, creates and returns a new data structure. */
NurbsControlBox* nurbs_controlbox_alloc
    ( NurbsControlBox* pcb
    , const int num_cp_u
    , const int num_cp_v
    , const int num_cp_w
    )
{
    int iu, iv;
    size_t offset = 0;
    NurbsControlBox* cb = pcb;

    if (num_cp_u <= 0 || num_cp_v <= 0 || num_cp_w <= 0){
        _handle_error_("Incorrect number of control points");
        return nullptr;
    }

    if (cb == nullptr){
        _check_(cb = (NurbsControlBox*)_malloc_(sizeof(NurbsControlBox)));
        if (cb == nullptr){
            /* Out of memory */
            return nullptr;
        }
    }

    cb->cp_length_u = num_cp_u;
    cb->cp_length_v = num_cp_v;
    cb->cp_length_w = num_cp_w;

    /* By default the order is 1. 
     * If there is only one layer of control points, the order is 0 */
    if (cb->cp_length_u == 1){
        cb->order_u = 0;
    }
    else{
        cb->order_u = 1;
    }

    if (cb->cp_length_v == 1){
        cb->order_v = 0;
    }
    else{
        cb->order_v = 1;
    }

    if (cb->cp_length_w == 1){
        cb->order_w = 0;
    }
    else{
        cb->order_w = 1;
    }

    _check_(cb->cp_stream = (NurbsVector3*)_malloc_
        (sizeof(NurbsVector3) * num_cp_u * num_cp_v * num_cp_w));
    if (cb->cp_stream == nullptr){
        /* Out of memory */
        return nullptr;
    }

    /* The correct order to fill the stream is {u, v, w}:
    cp_stream =
    { {0,0,0}, {0,0,1}, {0,1,0}, {0,1,1}, {0,2,0}, {0,2,1}
    , {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1}, {1,2,0}, {1,2,1}
    };
    */
    _check_(cb->cp = (NurbsVector3***)_malloc_
        (sizeof(NurbsVector3**) * num_cp_u * num_cp_v * num_cp_w));

    /* Create a matrix of pointers for the sweet notation cp[iu][iv][iw] */
    for (iu = 0; iu < num_cp_u; iu++){
        _check_(cb->cp[iu] = (NurbsVector3**)_malloc_
                (sizeof(NurbsVector3*)*cb->cp_length_v));

        for (iv = 0; iv < num_cp_v; iv++){
            cb->cp[iu][iv] = &(cb->cp_stream[offset]);
            offset += num_cp_w;
        }
    }

    /* By default the basis are standar NURBS */
    cb->basis_equation = 0;

    return cb;
}


/* Alias of nurbs_controlbox_alloc. */
NurbsControlBox* nurbs_controlbox_create
    ( NurbsControlBox* cb
    , const int num_cp_u
    , const int num_cp_v
    , const int num_cp_w
    )
{
    return nurbs_controlbox_alloc( cb, num_cp_u, num_cp_v, num_cp_w );
}


/* Releases memory resources. */
void nurbs_controlbox_dispose(NurbsControlBox* cb)
{
    int iu;

    if (cb == nullptr){
        return;
    }

    cb->id = -1;

    if (cb->cp_stream != nullptr){
        free(cb->cp_stream);
        cb->cp_stream = nullptr;
    }

    if (cb->cp != nullptr){
        for (iu = 0; iu < cb->cp_length_u; iu++){
            free(cb->cp[iu]);
        }
        free(cb->cp);
    }

    cb->cp_length_u = 0;
    cb->cp_length_v = 0;
    cb->cp_length_w = 0;

    cb->cp = nullptr;
}


/* Releases memory resources. */
void nurbs_controlbox_free( NurbsControlBox* cb, const int len )
{
    int i;

    if (cb == nullptr){
        return;
    }

    for (i = 0; i < len; i++){
        nurbs_controlbox_dispose( &(cb[i]) );
    }

    free(cb);
}

/* Copies the control box. Make sure that the dest is already empty. */
void nurbs_controlbox_copy
    ( NurbsControlBox* dest, const NurbsControlBox* orig )
{
    int cp_length;

    if (orig == nullptr || dest == nullptr){
        return;
    }

    cp_length = orig->cp_length_u * orig->cp_length_v * orig->cp_length_w;

    nurbs_controlbox_alloc
        ( dest
        , orig->cp_length_u, orig->cp_length_v, orig->cp_length_w
        );

    dest->order_u = orig->order_u;
    dest->order_v = orig->order_v;
    dest->order_w = orig->order_w;

    dest->id = orig->id;
    strcpy( dest->label, orig->label );
    memcpy( dest->cp_stream, orig->cp_stream, sizeof(NurbsVector3)*cp_length );
}


/* Calculates the point coordinates from the parametric coordinates */
NurbsVector3 nurbs_controlbox_get_point
    ( const NurbsControlBox* cb
    , const NurbsFloat u
    , const NurbsFloat v
    , const NurbsFloat w
    )
{
    int iknu, iknv, iknw;   /* lower index of the knot interval */
    int iu, iv, iw;    
    int nu = 0, nv = 0, nw = 0;
    NurbsFloat bu, bv, bw, buvw;     
    NurbsVector3 cp, point;

    /* Basis functions */
    NurbsFloat* basis_u = nullptr;
    NurbsFloat* basis_v = nullptr;
    NurbsFloat* basis_w = nullptr;

    point.x = NURBS_ERROR_VALUE;
    point.y = NURBS_ERROR_VALUE;
    point.z = NURBS_ERROR_VALUE;

    if (cb == nullptr){
        goto ERROR;
    }
    
    _check_(basis_u = (NurbsFloat*)_malloc_
        ((cb->cp_length_u) * sizeof(NurbsFloat)));

    _check_(basis_v = (NurbsFloat*)_malloc_
        ((cb->cp_length_v) * sizeof(NurbsFloat)));

    _check_(basis_w = (NurbsFloat*)_malloc_
        ((cb->cp_length_w) * sizeof(NurbsFloat)));

    if (basis_u == nullptr || basis_v == nullptr || basis_w == nullptr){
        /* Out of memory */
        goto ERROR;
    }

    /*  Get basis */
    iknu = nurbs_controlbox_basis_function
        ( basis_u, u, cb->cp_length_u, cb->order_u, cb->basis_equation );

    iknv = nurbs_controlbox_basis_function
        ( basis_v, v, cb->cp_length_v, cb->order_v, cb->basis_equation );

    iknw = nurbs_controlbox_basis_function
        ( basis_w, w, cb->cp_length_w, cb->order_w, cb->basis_equation );
    
    if (iknu < 0 || iknv < 0 || iknw < 0){
        goto ERROR;
    }

    if (cb->basis_equation == 1){
        // Bezier
        if (cb->order_u == 0){
            nu = 1;
        }
        else if (cb->order_u == 1){
            nu = 2;
        }
        else if (cb->order_u == 2 || cb->order_u == 3){
            nu = 4;
        }

        if (cb->order_v == 0){
            nv = 1;
        }
        else if (cb->order_v == 1){
            nv = 2;
        }
        else if (cb->order_v == 2 || cb->order_v == 3){
            nv = 4;
        }

        if (cb->order_w == 0){
            nw = 1;
        }
        else if (cb->order_w == 1){
            nw = 2;
        }
        else if (cb->order_w == 2 || cb->order_w == 3){
            nw = 4;
        }
    }
    else{   
        // b-spline
        nu = cb->order_u + 1;
        nv = cb->order_v + 1;
        nw = cb->order_w + 1;
    }

    point.x = 0;
    point.y = 0;
    point.z = 0;

    for (iu = 0; iu < nu; iu++){
        bu = basis_u[iu + iknu];
        for (iv = 0; iv < nv; iv++){
            bv = basis_v[iv + iknv];
            for (iw = 0; iw < nw; iw++){
                bw = basis_w[iw + iknw];
                buvw = bu*bv*bw;

                cp = cb->cp[iu + iknu][iv + iknv][iw + iknw];
                point.x += buvw * cp.x;
                point.y += buvw * cp.y;
                point.z += buvw * cp.z;
            }
        }
    }

ERROR:
    if (basis_u != nullptr){
        free(basis_u);
    }
    if (basis_v != nullptr){
        free(basis_v);
    }
    if (basis_w != nullptr){
        free(basis_w);
    }

    return point;
}

/* Calculates the point coordinates from the parametric coordinates */
void nurbs_controlbox_get_derivatives
    ( const NurbsControlBox* cb
    , NurbsVector3* deriv_u
    , NurbsVector3* deriv_v
    , NurbsVector3* deriv_w
    , NurbsVector3* point
    , const NurbsFloat u
    , const NurbsFloat v
    , const NurbsFloat w
    )
{
    int iknu, iknv, iknw;   /* lower index of the knot interval */
    int iu, iv, iw;   
    int nu, nv, nw;
    NurbsFloat bu, bv, bw, buvw, du, dv, dw;     
    NurbsVector3 cp;
    const int order_u = cb->order_u;
    const int order_v = cb->order_v;
    const int order_w = cb->order_w;

    /* Basis functions */
    NurbsFloat* basis_u = nullptr;
    NurbsFloat* basis_v = nullptr;
    NurbsFloat* basis_w = nullptr;
    NurbsFloat* d_basis_u = nullptr;
    NurbsFloat* d_basis_v = nullptr;
    NurbsFloat* d_basis_w = nullptr;

    point->x = 0;
    point->y = 0;
    point->z = 0;
    
    deriv_u->x = 0;
    deriv_u->y = 0;
    deriv_u->z = 0;
    
    deriv_v->x = 0;
    deriv_v->y = 0;
    deriv_v->z = 0;
    
    deriv_w->x = 0;
    deriv_w->y = 0;
    deriv_w->z = 0;
    
    _check_(basis_u = (NurbsFloat*)_malloc_
        ((cb->cp_length_u) * sizeof(NurbsFloat)));

    _check_( basis_v = (NurbsFloat*)_malloc_
        ((cb->cp_length_v) * sizeof(NurbsFloat)));

    _check_( basis_w = (NurbsFloat*)_malloc_
        ((cb->cp_length_w) * sizeof(NurbsFloat)));

    _check_( d_basis_u = (NurbsFloat*)_malloc_
        ((cb->cp_length_u) * sizeof(NurbsFloat)));

    _check_( d_basis_v = (NurbsFloat*)_malloc_
        ((cb->cp_length_v) * sizeof(NurbsFloat)));

    _check_( d_basis_w = (NurbsFloat*)_malloc_
        ((cb->cp_length_w) * sizeof(NurbsFloat)));

    if (basis_u == nullptr || basis_v == nullptr || basis_w == nullptr 
      || d_basis_u == nullptr || d_basis_v == nullptr || d_basis_w == nullptr){
        /* Out of memory */
        goto ERROR;
    }

    /*  Get basis */
    iknu = nurbs_controlbox_d_basis_function
        ( d_basis_u, basis_u, u, cb->cp_length_u, order_u, cb->basis_equation);

    iknv = nurbs_controlbox_d_basis_function
        ( d_basis_v, basis_v, v, cb->cp_length_v, order_v, cb->basis_equation);

    iknw = nurbs_controlbox_d_basis_function
        ( d_basis_w, basis_w, w, cb->cp_length_w, order_w, cb->basis_equation);

    if (cb->basis_equation == 1){
        /* BEZIER basis */
        if (cb->order_u <= 0){
            nu = 1;
        }
        else if (cb->order_u == 1){
            nu = 2;
        }
        else if (cb->order_u == 2 || cb->order_u == 3){
            nu = 4;
        }
        else{
            /* unsuported */
            nu = 0;
        }

        if (cb->order_v <= 0){
            nv = 1;
        }
        else if (cb->order_v == 1){
            nv = 2;
        }
        else if (cb->order_v == 2 || cb->order_v == 3){
            nv = 4;
        }
        else{
            /* unsuported */
            nv = 0;
        }

        if (cb->order_w <= 0){
            nw = 1;
        }
        else if (cb->order_w == 1){
            nw = 2;
        }
        else if (cb->order_w == 2 || cb->order_w == 3){
            nw = 4;
        }
        else{
            /* unsuported */
            nw = 0;
        }
    }
    else{
        /* NURBS basis */
        nu = cb->order_u+1;
        nv = cb->order_v+1;
        nw = cb->order_w+1;
    }

    for (iu = 0; iu < nu; iu++){
        bu = basis_u[iu + iknu];
        du = d_basis_u[iu + iknu];
        for (iv = 0; iv < nv; iv++){
            bv = basis_v[iv + iknv];
            dv = d_basis_v[iv + iknv];
            for (iw = 0; iw < nw; iw++){
                bw = basis_w[iw + iknw];
                dw = d_basis_w[iw + iknw];

                cp = cb->cp[iu + iknu][iv + iknv][iw + iknw];

                buvw = bu*bv*bw;
                point->x += buvw * cp.x;
                point->y += buvw * cp.y;
                point->z += buvw * cp.z;

                buvw = du*bv*bw;
                deriv_u->x += buvw * cp.x;
                deriv_u->y += buvw * cp.y;
                deriv_u->z += buvw * cp.z;

                buvw = bu*dv*bw;
                deriv_v->x += buvw * cp.x;
                deriv_v->y += buvw * cp.y;
                deriv_v->z += buvw * cp.z;

                buvw = bu*bv*dw;
                deriv_w->x += buvw * cp.x;
                deriv_w->y += buvw * cp.y;
                deriv_w->z += buvw * cp.z;
            }
        }
    }

ERROR:
    if (basis_u != nullptr){
        free(basis_u);
    }
    if (basis_v != nullptr){
        free(basis_v);
    }
    if (basis_w != nullptr){
        free(basis_w);
    }
    if (d_basis_u != nullptr){
        free(d_basis_u);
    }
    if (d_basis_v != nullptr){
        free(d_basis_v);
    }
    if (d_basis_w != nullptr){
        free(d_basis_w);
    };
}

/* Calculates the point coordinates from the parametric coordinates */
static void nurbs_controlbox_deriv_fd
    ( const NurbsControlBox* cb
    , NurbsVector3* deriv_u
    , NurbsVector3* deriv_v
    , NurbsVector3* deriv_w
    , const NurbsFloat u
    , const NurbsFloat v
    , const NurbsFloat w
    )
{
    const NurbsFloat eps = 1e-4;
    NurbsFloat u0 = u - eps;
    NurbsFloat u1 = u + eps;
    NurbsFloat v0 = v - eps;
    NurbsFloat v1 = v + eps;
    NurbsFloat w0 = w - eps;
    NurbsFloat w1 = w + eps;
    NurbsVector3 p0;
    NurbsVector3 p1;
    NurbsFloat d;

    d = 2;
    if (u0 < 0){
        d = 1;
        u0 = 0;
    }
    if (u1 > 1){
        d = 1;
        u1 = 1;
    }
    p0 = nurbs_controlbox_get_point( cb, u0, v, w );
    p1 = nurbs_controlbox_get_point( cb, u1, v, w );
    deriv_u->x = (p1.x - p0.x) / (d*eps);
    deriv_u->y = (p1.y - p0.y) / (d*eps);
    deriv_u->z = (p1.z - p0.z) / (d*eps);

    p0 = nurbs_controlbox_get_point( cb, u, v0, w );
    p1 = nurbs_controlbox_get_point( cb, u, v1, w );
    deriv_v->x = (p1.x - p0.x) / (d*eps);
    deriv_v->y = (p1.y - p0.y) / (d*eps);
    deriv_v->z = (p1.z - p0.z) / (d*eps);

    p0 = nurbs_controlbox_get_point( cb, u, v, w0 );
    p1 = nurbs_controlbox_get_point( cb, u, v, w1 );
    deriv_w->x = (p1.x - p0.x) / (d*eps);
    deriv_w->y = (p1.y - p0.y) / (d*eps);
    deriv_w->z = (p1.z - p0.z) / (d*eps);
}

static NurbsFloat det3
    ( const NurbsFloat a0, const NurbsFloat a1, const NurbsFloat a2
    , const NurbsFloat b0, const NurbsFloat b1, const NurbsFloat b2
    , const NurbsFloat c0, const NurbsFloat c1, const NurbsFloat c2
    )
{
    return a0*b1*c2 + a2*b0*c1 + a1*b2*c0 - a2*b1*c0 - a1*b0*c2 - a0*b2*c1;
}


static inline int matrix_inverse
    ( NurbsFloat* out
    , const NurbsFloat a, const NurbsFloat b, const NurbsFloat c
    , const NurbsFloat d, const NurbsFloat e, const NurbsFloat f
    , const NurbsFloat g, const NurbsFloat h, const NurbsFloat k
    )
{
    NurbsFloat det = det3( a, b, c, d, e, f, g, h, k );

    if (det > 1e-6 || det < -1e-6){
        out[0] = (e*k - f*h) / det;
        out[1] = (f*g - d*k) / det;
        out[2] = (d*h - e*g) / det;
        out[3] = (c*h - b*k) / det;
        out[4] = (a*k - c*g) / det;
        out[5] = (g*b - a*h) / det;
        out[6] = (b*f - c*e) / det;
        out[7] = (c*d - a*f) / det;
        out[8] = (a*e - b*d) / det;

        return 0;
    }
    else{
        /* The matrix is singular. :(
         * Most probably one derivative is zero or 
         * the initial point is a bad choice on an edge. */
        return 1;
    }
}


/* Solves a linear equation system of three variables */
static void solve_linear3 
    ( NurbsFloat* u, NurbsFloat* v, NurbsFloat* w
    , const NurbsFloat a0, const NurbsFloat a1, const NurbsFloat a2
    , const NurbsFloat b0, const NurbsFloat b1, const NurbsFloat b2
    , const NurbsFloat c0, const NurbsFloat c1, const NurbsFloat c2
    , const NurbsFloat d0, const NurbsFloat d1, const NurbsFloat d2
    , const NurbsFloat det
    )
{
    *u = det3( d0, b0, c0, d1, b1, c1, d2, b2, c2 ) / det;
    *v = det3( a0, d0, c0, a1, d1, c1, a2, d2, c2 ) / det;
    *w = det3( a0, b0, d0, a1, b1, d1, a2, b2, d2 ) / det;
}


/* Calculates a fair approximation of the inversion point */
static NurbsVector3 cb_inversion_estimation_xyz
    ( const NurbsControlBox* ffd
    , const NurbsVector3 p
    )
{
    int iu, iv, iw, iu0, iv0, iw0, iu1, iv1, iw1;
    NurbsVector3 p000, p001, p010, p011, p100, p101, p110, p111;
    NurbsVector3 pt;
    NurbsFloat u, v, w;
    NurbsVector3 tuvw = {-1, -1, -1};
    NurbsFloat dist, best_dist = -1;
    NurbsFloat ma, mb, mc, md, me, mf, mg, mh, mk;
    NurbsFloat M;
    int nu, nv, nw;

    nu = ffd->cp_length_u - 1;
    nv = ffd->cp_length_v - 1;
    nw = ffd->cp_length_w - 1;

    /* Loop through all boxes to find a good approximation */
    for (iu = 0; iu < nu; iu++){
        for (iv = 0; iv < nv; iv++){
            for (iw = 0; iw < nw; iw++){

                iu0 = iu;
                iu1 = iu + 1;

                iv0 = iv;
                iv1 = iv + 1;

                iw0 = iw;
                iw1 = iw + 1;

                p000 = ffd->cp[iu0][iv0][iw0];
                p001 = ffd->cp[iu0][iv0][iw1];
                p010 = ffd->cp[iu0][iv1][iw0];
                p011 = ffd->cp[iu0][iv1][iw1];
                p100 = ffd->cp[iu1][iv0][iw0];
                p101 = ffd->cp[iu1][iv0][iw1];
                p110 = ffd->cp[iu1][iv1][iw0];
                p111 = ffd->cp[iu1][iv1][iw1];

                /* Translate to origin */
                p100.x -= p000.x;
                p100.y -= p000.y;
                p100.z -= p000.z;

                p010.x -= p000.x;
                p010.y -= p000.y;
                p010.z -= p000.z;

                p001.x -= p000.x;
                p001.y -= p000.y;
                p001.z -= p000.z;

                p110.x -= p000.x;
                p110.y -= p000.y;
                p110.z -= p000.z;

                p101.x -= p000.x;
                p101.y -= p000.y;
                p101.z -= p000.z;

                p011.x -= p000.x;
                p011.y -= p000.y;
                p011.z -= p000.z;

                p111.x -= p000.x;
                p111.y -= p000.y;
                p111.z -= p000.z;

                /* Calculate a linear transformation to make a unitary ortogonal
                control box. ==> p100={1,0,0}; p010={0,1,0}; p001={0,0,1}  */
                M = det3( p100.x, p100.y, p100.z
                        , p010.x, p010.y, p010.z 
                        , p001.x, p001.y, p001.z 
                        );

                if (M > 0 || M < 0){
                    solve_linear3
                        ( &ma, &md, &mg
                        , p100.x,   p100.y,     p100.z
                        , p010.x,   p010.y,     p010.z 
                        , p001.x,   p001.y,     p001.z 
                        , 1,        0,          0
                        , M
                        );

                    solve_linear3 
                        ( &mb, &me, &mh
                        , p100.x,   p100.y,     p100.z 
                        , p010.x,   p010.y,     p010.z 
                        , p001.x,   p001.y,     p001.z
                        , 0,        1,          0
                        , M
                        );

                    solve_linear3
                        ( &mc, &mf, &mk
                        , p100.x,   p100.y,     p100.z
                        , p010.x,   p010.y,     p010.z
                        , p001.x,   p001.y,     p001.z
                        , 0,        0,          1
                        , M
                        );

                    /* Apply the linear transformation to the control points */


                    /*
                    pt.x = p100.x * ma + p100.y * mb + p100.z * mc;
                    pt.y = p100.x * md + p100.y * me + p100.z * mf;
                    pt.z = p100.x * mg + p100.y * mh + p100.z * mk;
                    p100 = pt;  //( Should be {1, 0, 0} )

                    pt.x = p010.x * ma + p010.y * mb + p010.z * mc;
                    pt.y = p010.x * md + p010.y * me + p010.z * mf;
                    pt.z = p010.x * mg + p010.y * mh + p010.z * mk;
                    p010 = pt;  //( Should be {0, 1, 0} )

                    pt.x = p001.x * ma + p001.y * mb + p001.z * mc;
                    pt.y = p001.x * md + p001.y * me + p001.z * mf;
                    pt.z = p001.x * mg + p001.y * mh + p001.z * mk;
                    p001 = pt;  //( Should be {0, 0, 1} )


                    pt.x = p110.x * ma + p110.y * mb + p110.z * mc;
                    pt.y = p110.x * md + p110.y * me + p110.z * mf;
                    pt.z = p110.x * mg + p110.y * mh + p110.z * mk;
                    p110 = pt;  //( This leads to the non linear term )

                    pt.x = p101.x * ma + p101.y * mb + p101.z * mc;
                    pt.y = p101.x * md + p101.y * me + p101.z * mf;
                    pt.z = p101.x * mg + p101.y * mh + p101.z * mk;
                    p101 = pt;  //( This leads to the non linear term )

                    pt.x = p011.x * ma + p011.y * mb + p011.z * mc;
                    pt.y = p011.x * md + p011.y * me + p011.z * mf;
                    pt.z = p011.x * mg + p011.y * mh + p011.z * mk;
                    p011 = pt;  //( This leads to the non linear term )

                    pt.x = p111.x * ma + p111.y * mb + p111.z * mc;
                    pt.y = p111.x * md + p111.y * me + p111.z * mf;
                    pt.z = p111.x * mg + p111.y * mh + p111.z * mk;
                    p111 = pt;  //( This leads to the non linear term )
                    */

                    pt.x = p.x - p000.x;
                    pt.y = p.y - p000.y;
                    pt.z = p.z - p000.z;

                    u = pt.x * ma + pt.y * mb + pt.z * mc;
                    v = pt.x * md + pt.y * me + pt.z * mf;
                    w = pt.x * mg + pt.y * mh + pt.z * mk;

                    u = (u + iu) / (ffd->cp_length_u - 1);
                    v = (v + iv) / (ffd->cp_length_v - 1);
                    w = (w + iw) / (ffd->cp_length_w - 1);

                    if (u < -0.05 || u > 1.05 || v < -0.05 || v > 1.05 || w < -0.05 || w > 1.05){
                        continue;
                    }

                    pt = nurbs_controlbox_get_point( ffd, u, v, w );
                    
                    dist = (pt.x - p.x) * (pt.x - p.x);
                    dist += (pt.y - p.y) * (pt.y - p.y);
                    dist += (pt.z - p.z) * (pt.z - p.z);

                    if (best_dist > dist || best_dist < 0){
                        best_dist = dist;
                        tuvw.x = u;
                        tuvw.y = v;
                        tuvw.z = w;
                    }
                }
            }
        }
    }

    return tuvw;
}

/* Calculates a fair approximation of the inversion point */
static NurbsVector3 cb_inversion_estimation_2d
    ( const NurbsControlBox* ffd
    , const NurbsVector3 p
    )
{
    int iu, iv, iw, iu0, iv0, iw0, iu1, iv1, iw1;
    NurbsVector3 p000, p001, p010, p011, p100, p101, p110, p111;
    NurbsFloat a0, a1, a2;
    NurbsVector3 pt;
    NurbsFloat u, v, w;
    NurbsVector3 tuvw = {-1, -1, -1};
    NurbsFloat dist, best_dist = -1;
    NurbsFloat ma, mb, mc, md, me, mf, mg, mh, mk;
    NurbsFloat M;
    int nu, nv, nw;

    nu = ffd->cp_length_u - 1;
    nv = ffd->cp_length_v - 1;
    nw = ffd->cp_length_w - 1;

    if (nu <= 0){
        nu = 1;
    }
    if (nv <= 0){
        nv = 1;
    }
    if (nw <= 0){
        nw = 1;
    }

    /* Loop through all sub boxes to find a good approximation */
    for (iu = 0; iu < nu; iu++){
        for (iv = 0; iv < nv; iv++){
            for (iw = 0; iw < nw; iw++){

                iu0 = iu;
                if (ffd->cp_length_u > 1){
                    iu1 = iu + 1;
                }
                else{
                    iu1 = iu0;
                }

                iv0 = iv;
                if (ffd->cp_length_v > 1){
                    iv1 = iv + 1;
                }
                else{
                    iv1 = iv0;
                }

                iw0 = iw;
                if (ffd->cp_length_w > 1){
                    iw1 = iw + 1;
                }
                else{
                    iw1 = iw0;
                }

                p000 = ffd->cp[iu0][iv0][iw0];
                p001 = ffd->cp[iu0][iv0][iw1];
                p010 = ffd->cp[iu0][iv1][iw0];
                p011 = ffd->cp[iu0][iv1][iw1];
                p100 = ffd->cp[iu1][iv0][iw0];
                p101 = ffd->cp[iu1][iv0][iw1];
                p110 = ffd->cp[iu1][iv1][iw0];
                p111 = ffd->cp[iu1][iv1][iw1];

                /* Translate to origin */
                p100.x -= p000.x;
                p100.y -= p000.y;
                p100.z -= p000.z;

                p010.x -= p000.x;
                p010.y -= p000.y;
                p010.z -= p000.z;

                p001.x -= p000.x;
                p001.y -= p000.y;
                p001.z -= p000.z;

                p110.x -= p000.x;
                p110.y -= p000.y;
                p110.z -= p000.z;

                p101.x -= p000.x;
                p101.y -= p000.y;
                p101.z -= p000.z;

                p011.x -= p000.x;
                p011.y -= p000.y;
                p011.z -= p000.z;

                p111.x -= p000.x;
                p111.y -= p000.y;
                p111.z -= p000.z;

                /* Calculate a linear transformation to make a unitary ortogonal
                control box. ==> p100={1,0,0}; p010={0,1,0}; p001={0,0,1}  */
                if (ffd->cp_length_u <= 1){
                    a0 = 1;
                }
                else{
                    a0 = p100.x;
                }
                if (ffd->cp_length_v <= 1){
                    a1 = 1;
                }
                else{
                    a1 = p010.y;
                }
                if (ffd->cp_length_w <= 1){
                    a2 = 1;
                }
                else{
                    a2 = p001.z;
                }

                M = det3( a0, p100.y, p100.z
                        , p010.x, a1, p010.z 
                        , p001.x, p001.y, a2 
                        );

                if (M > 0 || M < 0){
                    solve_linear3
                        ( &ma, &md, &mg
                        , a0,       p100.y,     p100.z
                        , p010.x,   a1,         p010.z 
                        , p001.x,   p001.y,     a2 
                        , 1,        0,          0
                        , M
                        );

                    solve_linear3 
                        ( &mb, &me, &mh
                        , a0,       p100.y,     p100.z 
                        , p010.x,   a1,         p010.z 
                        , p001.x,   p001.y,     a2
                        , 0,        1,          0
                        , M
                        );

                    solve_linear3
                        ( &mc, &mf, &mk
                        , a0,       p100.y,     p100.z
                        , p010.x,   a1,         p010.z
                        , p001.x,   p001.y,     a2
                        , 0,        0,          1
                        , M
                        );

                    /* Apply the linear transformation to the control points */


                    pt.x = p100.x * ma + p100.y * mb + p100.z * mc;
                    pt.y = p100.x * md + p100.y * me + p100.z * mf;
                    pt.z = p100.x * mg + p100.y * mh + p100.z * mk;
                    p100 = pt;  /*( Should be {1, 0, 0} )*/

                    pt.x = p010.x * ma + p010.y * mb + p010.z * mc;
                    pt.y = p010.x * md + p010.y * me + p010.z * mf;
                    pt.z = p010.x * mg + p010.y * mh + p010.z * mk;
                    p010 = pt;  /*( Should be {0, 1, 0} )*/

                    pt.x = p001.x * ma + p001.y * mb + p001.z * mc;
                    pt.y = p001.x * md + p001.y * me + p001.z * mf;
                    pt.z = p001.x * mg + p001.y * mh + p001.z * mk;
                    p001 = pt;  /*( Should be {0, 0, 1} )*/


                    pt.x = p110.x * ma + p110.y * mb + p110.z * mc;
                    pt.y = p110.x * md + p110.y * me + p110.z * mf;
                    pt.z = p110.x * mg + p110.y * mh + p110.z * mk;
                    p110 = pt;  /*( This leads to the non linear term )*/

                    pt.x = p101.x * ma + p101.y * mb + p101.z * mc;
                    pt.y = p101.x * md + p101.y * me + p101.z * mf;
                    pt.z = p101.x * mg + p101.y * mh + p101.z * mk;
                    p101 = pt;  /*( This leads to the non linear term )*/

                    pt.x = p011.x * ma + p011.y * mb + p011.z * mc;
                    pt.y = p011.x * md + p011.y * me + p011.z * mf;
                    pt.z = p011.x * mg + p011.y * mh + p011.z * mk;
                    p011 = pt;  /*( This leads to the non linear term )*/

                    pt.x = p111.x * ma + p111.y * mb + p111.z * mc;
                    pt.y = p111.x * md + p111.y * me + p111.z * mf;
                    pt.z = p111.x * mg + p111.y * mh + p111.z * mk;
                    p111 = pt;  /*( This leads to the non linear term )*/
                    

                    pt.x = p.x - p000.x;
                    pt.y = p.y - p000.y;
                    pt.z = p.z - p000.z;

                    u = pt.x * ma + pt.y * mb + pt.z * mc;
                    v = pt.x * md + pt.y * me + pt.z * mf;
                    w = pt.x * mg + pt.y * mh + pt.z * mk;

                    u = (u + iu) / nu;
                    v = (v + iv) / nv;
                    w = (w + iw) / nw;

                    if (u < 0){
                        u = 0;
                        continue;
                    }
                    if (u > 1){
                        continue;
                        u = 1;
                    }
                    if (v < 0){
                        continue;
                        v = 0;
                    }
                    if (v > 1){
                        continue;
                        v = 1;
                    }
                    if (w < 0){
                        continue;
                        w = 0;
                    }
                    if (w > 1){
                        continue;
                        w = 1;
                    }

                    pt = nurbs_controlbox_get_point( ffd, u, v, w );
                    
                    dist = (pt.x - p.x) * (pt.x - p.x);
                    dist += (pt.y - p.y) * (pt.y - p.y);
                    dist += (pt.z - p.z) * (pt.z - p.z);

                    if (best_dist > dist || best_dist < 0){
                        best_dist = dist;
                        tuvw.x = u;
                        tuvw.y = v;
                        tuvw.z = w;
                    }
                }
            }
        }
    }

    return tuvw;
}


static void inversion_2d_iterative
    ( NurbsFloat* delta_u
    , NurbsFloat* delta_v
    , NurbsFloat* delta_w
    , const NurbsControlBox* ffd
    , const NurbsVector3 p
    , const NurbsVector3 t
    )
{
    NurbsVector3 du, dv, dw;
    NurbsVector3 q, qp;
    NurbsFloat dd, duu, dvv, dww, duv, duw, dvw, qpu, qpv, qpw;

    *delta_u = 0;
    *delta_v = 0;
    *delta_w = 0;

    /* Calculate the derivatives by finite differences */
    nurbs_controlbox_deriv_fd
        ( ffd, &du, &dv, &dw, t.x, t.y, t.z );

    q = nurbs_controlbox_get_point( ffd, t.x, t.y, t.z );

            //nurbs_controlbox_get_derivatives
            //    ( ffd, &du, &dv, &dw, &q, t.x, t.y, t.z );

    qp.x = q.x-p.x;
    qp.y = q.y-p.y;
    qp.z = q.z-p.z;

    //err = qp.x*qp.x + qp.y*qp.y + qp.z*qp.z;

    /* Check the derivatives. The lowest one is ignored */
    duu = du.x*du.x + du.y*du.y + du.z*du.z;
    dvv = dv.x*dv.x + dv.y*dv.y + dv.z*dv.z;
    dww = dw.x*dw.x + dw.y*dw.y + dw.z*dw.z;

    if (duu < dvv && duu < dww){
        /* Ignore 'u' */
        dvw = dv.x*dw.x + dv.y*dw.y + dv.z*dw.z;

        dd = dvv*dww - dvw*dvw;
        dd = sqrt( dd );
        if (dd > 0 || dd < 0){
            dd *= 4;

            qpv = qp.x*dv.x + qp.y*dv.y + qp.z*dv.z;
            qpw = qp.x*dw.x + qp.y*dw.y + qp.z*dw.z;

            *delta_v = -(qpv * dvv - qpw * dvw) / dd;
            *delta_w = -(qpw * dww - qpv * dvw) / dd;
        }
    }
    else if (dvv < duu && dvv < dww){
        /* Ignore 'v' */
        duw = du.x*dw.x + du.y*dw.y + du.z*dw.z;

        dd = duu*dww - duw*duw;
        dd = sqrt( dd );
        if (dd > 0 || dd < 0){
            dd *= 4;

            qpu = qp.x*du.x + qp.y*du.y + qp.z*du.z;
            qpw = qp.x*dw.x + qp.y*dw.y + qp.z*dw.z;

            *delta_u = -(qpu * duu - qpw * duw) / dd;
            *delta_w = -(qpw * dww - qpu * duw) / dd;
        }
    }
    else{
        /* Ignore 'w' */
        duv = du.x*dv.x + du.y*dv.y + du.z*dv.z;

        dd = duu*dvv - duv*duv;
        dd = sqrt( dd );
        if (dd > 0 || dd < 0){
            dd *= 4;

            qpu = qp.x*du.x + qp.y*du.y + qp.z*du.z;
            qpv = qp.x*dv.x + qp.y*dv.y + qp.z*dv.z;

            *delta_u = -(qpu * duu - qpv * duv) / dd;
            *delta_v = -(qpv * dvv - qpu * duv) / dd;
        }
    }
}


static NurbsFloat absf( const NurbsFloat a )
{
    if (a < 0) return -a;
    else return a;
}


/* Calculates the parametric values from the spatial coordinates */
NurbsVector3 nurbs_controlbox_inversion_newton
    ( const NurbsControlBox* ffd
    , const NurbsVector3 p
    , const NurbsVector3 t
    , const NurbsFloat epsilon
    , NurbsFloat* err
    )
{
    NurbsVector3 t0, t1;
    NurbsVector3 du, dv, dw;
    NurbsVector3 p0;
    NurbsFloat m[9] = {0};
    NurbsFloat bg;
    NurbsFloat delta_u = 0, delta_v = 0, delta_w = 0;
    NurbsFloat mod1, mod0;
    int it = 0, fail = 0, status;

    t0 = t;
    p0 = nurbs_controlbox_get_point( ffd, t0.x, t0.y, t0.z );

    mod0 = (p.x - p0.x) * (p.x - p0.x);
    mod0 += (p.y - p0.y) * (p.y - p0.y);
    mod0 += (p.z - p0.z) * (p.z - p0.z);
    mod0 = sqrt(mod0);
    bg = 1.0;

    delta_u = 1;
    delta_v = 1;
    delta_w = 1;
    while (
        (absf( delta_u ) > epsilon 
        || absf( delta_v ) > epsilon 
        || absf( delta_w ) > epsilon) 
        && it < 1000)
    {
        it++; /* safeguard */
        if (fail == 0){
            nurbs_controlbox_get_derivatives
                ( ffd, &du, &dv, &dw, &p0, t0.x, t0.y, t0.z );
            
            status = matrix_inverse
                ( m, du.x, dv.x, dw.x, du.y, dv.y, dw.y, du.z, dv.z, dw.z );

            if (status == 0){
                delta_u = (p.x - p0.x) * m[0];
                delta_u += (p.y - p0.y) * m[3];
                delta_u += (p.z - p0.z) * m[6];
                delta_u /= 4;
                
                delta_v = (p.x - p0.x) * m[1];
                delta_v += (p.y - p0.y) * m[4];
                delta_v += (p.z - p0.z) * m[7];
                delta_v /= 4;

                delta_w = (p.x - p0.x) * m[2];
                delta_w += (p.y - p0.y) * m[5];
                delta_w += (p.z - p0.z) * m[8];
                delta_w /= 4;
            }
            else{
                /* There are problems with the derivatives if the matrix is singular */
                inversion_2d_iterative( &delta_u, &delta_v, &delta_w, ffd, p, t0 );
            }
        }

        /* In case there is one derivative that might trigger impossible values
         * due to numerical floating error */
        if (delta_u > 1 || delta_u < -1){
            delta_u = 0;
        }
        if (delta_v > 1 || delta_v < -1){
            delta_v = 0;
        }
        if (delta_w > 1 || delta_w < -1){
            delta_w = 0;
        }

        t1.x = t0.x + delta_u * bg;
        if (t1.x < 0){
            t1.x = 0;
        }
        else if (t1.x > 1){
            t1.x = 1;
        }

        t1.y = t0.y + delta_v * bg;
        if (t1.y < 0){
            t1.y = 0;
        }
        else if (t1.y > 1){
            t1.y = 1;
        }

        t1.z = t0.z + delta_w * bg;
        if (t1.z < 0){
            t1.z = 0;
        }
        else if (t1.z > 1){
            t1.z = 1;
        }

        p0 = nurbs_controlbox_get_point( ffd, t1.x, t1.y, t1.z );

        mod1 = (p.x - p0.x) * (p.x - p0.x);
        mod1 += (p.y - p0.y) * (p.y - p0.y);
        mod1 += (p.z - p0.z) * (p.z - p0.z);
        mod1 = sqrt(mod1);
        if (mod1 < mod0){
            t0 = t1;
            mod0 = mod1;
            bg = 1;
            fail = 0;
        }
        else{
            bg *= 0.5;
            fail = 1;
        }
    }

    if (err != nullptr){
        *err = mod0;
    }

    return t0;
}


/* Calculates the parametric values from the spatial coordinates.
 * First estimates an initial value and then runs a Newthon-Raphson. */
NurbsVector3 nurbs_controlbox_inversion
    ( const NurbsControlBox* ffd
    , const NurbsFloat x
    , const NurbsFloat y
    , const NurbsFloat z
    , const NurbsFloat epsilon
    , NurbsFloat* err
    )
{
    NurbsVector3 t;
    NurbsVector3 p = {x, y, z};
    NurbsFloat dist;
    int iu, iv, iw;
    int nu = 0, nv = 0, nw = 0;
    NurbsVector3 t0, t1;

    if (ffd == nullptr){
        t.x = NURBS_ERROR_VALUE;
        t.y = NURBS_ERROR_VALUE;
        t.z = NURBS_ERROR_VALUE;

        return t;
    }

    if (ffd->cp_length_u > 1 && ffd->cp_length_v > 1 && ffd->cp_length_w > 1){
        t = cb_inversion_estimation_xyz( ffd, p );
    }
    else{
        t = cb_inversion_estimation_2d( ffd, p );
    }

    t = nurbs_controlbox_inversion_newton( ffd, p, t, epsilon, err );
    if (*err < epsilon){
        return t;
    }

    if (ffd->basis_equation == 1){
        nu = ffd->order_u - 1;
        nv = ffd->order_v - 1;
        nw = ffd->order_w - 1;

        t0 = t;
        for (iu = -nu; iu <= nu; iu++){
            for (iv = -nv; iv <= nv; iv++){
                for (iw = -nw; iw <= nw; iw++){
                    if (iu != 0 || iv != 0 || iw != 0){
                        t1.x = t0.x + (NurbsFloat)iu / (2 * (ffd->cp_length_u - 1));
                        t1.y = t0.y + (NurbsFloat)iv / (2 * (ffd->cp_length_v - 1));
                        t1.z = t0.z + (NurbsFloat)iw / (2 * (ffd->cp_length_w - 1));

                        t1 = nurbs_controlbox_inversion_newton
                            ( ffd, p, t1, epsilon, &dist );

                        if (dist < *err){
                            *err = dist;
                            t = t1;
                        }
                    }
                }
            }
        }
    }

    return t;
}

