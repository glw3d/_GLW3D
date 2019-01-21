 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    NURBS curves

*******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "nurbs_internal.h"
#include "nurbs_basis.h"
#include "nurbs_curve.h"


/* Equivalent to a C++ default constructor */
void nurbs_curve_init( NurbsCurve* curve )
{
    curve->label = nullptr;
    curve->cp = nullptr;
    curve->knot = nullptr;
    curve->id = -1;
    curve->degree = 0;
    curve->cp_length = 0;
    curve->knot_length =0;
    curve->next = nullptr;
}

/* Reserves the memory. Equivalent to a C++ constructor. */
NurbsCurve* nurbs_curve_alloc
    ( NurbsCurve* curve
    , const int num_control_points
    , const int degree
    )
{
    size_t cp_size;
    size_t k_size;

    if (curve == nullptr){
        _check_(curve = (NurbsCurve*)_malloc_(sizeof(NurbsCurve)));
        if (curve == nullptr){
            /* Out of memory */
            return nullptr;
        }
    }


    curve->label = nullptr;
    curve->cp = nullptr;
    curve->knot = nullptr;
    curve->id = -1;
    curve->degree = degree;
    curve->cp_length = num_control_points;
    curve->knot_length = num_control_points + degree + 1;
    curve->next = nullptr;

    if (num_control_points < 1 || curve->knot_length < 1){
        return curve;
    }

    cp_size = curve->cp_length * sizeof(NurbsVector4);
    k_size = curve->knot_length * sizeof(NurbsFloat);

    _check_(curve->cp = (NurbsVector4*)_malloc_(cp_size));
    _check_( curve->knot = (NurbsFloat*)_malloc_( k_size ) );
    _check_( curve->basis = (NurbsFloat*)_malloc_( k_size ) );
    _check_( curve->d_basis = (NurbsFloat*)_malloc_( k_size ) );

    return curve;
}

/* Alias of nurbs_curve_alloc */
NurbsCurve* nurbs_curve_create
    ( NurbsCurve* curve
    , const int num_control_points
    , const int degree
    )
{
    return nurbs_curve_alloc( curve, num_control_points, degree );
}

/* Releases the stack memory. Equivalent to a C++ destructor. */
void nurbs_curve_dispose(NurbsCurve* curve)
{
    if (curve == nullptr){
        return;
    }

    if (curve->cp != nullptr){
        free(curve->cp);
        curve->cp = nullptr;
    }

    curve->cp_length = 0;

    if (curve->knot != nullptr){
        free(curve->knot);
        curve->knot = nullptr;
    }
    curve->knot_length = 0;

    if (curve->basis != nullptr){
        free(curve->basis);
        curve->basis = nullptr;
    }

    if (curve->d_basis != nullptr){
        free(curve->d_basis);
        curve->d_basis = nullptr;
    }

    if (curve->label != nullptr){
        free(curve->label);
        curve->label = nullptr;
    }
}


/* Releases the memory. */
void nurbs_curve_free(NurbsCurve* curve_array, const int length)
{
    int i;

    if (curve_array == nullptr){
        return;
    }

    for (i = 0; i < length; i++){
        nurbs_curve_dispose(&(curve_array[i]));
    }

    free(curve_array);
}

/* Make a copy of the nurbs curve. */
NurbsCurve* nurbs_curve_copy(NurbsCurve *dest, const NurbsCurve *orig)
{
    int i;
    const size_t k_size = dest->knot_length * sizeof(NurbsFloat);

    if (dest == nullptr){
        /* Create a new nurbs pointer */
        _check_( dest = (NurbsCurve*)_malloc_( sizeof( NurbsCurve ) ) );
    }
    else{
        /* Clear former possible values */
        nurbs_curve_dispose(dest);
    }

    dest->id = orig->id;
    dest->degree = orig->degree;
    dest->cp_length = orig->cp_length;
    dest->knot_length = orig->knot_length;

    _check_( dest->cp = (NurbsVector4*)_malloc_( dest->cp_length * sizeof( NurbsVector4 ) ) );

    for (i = 0; i < dest->cp_length; i++){
        dest->cp[i] = orig->cp[i];
    }

    _check_( dest->knot = (NurbsFloat*)_malloc_( dest->knot_length * sizeof( NurbsFloat ) ) );

    for (i = 0; i < dest->knot_length; i++){
        dest->knot[i] = orig->knot[i];
    }
    _check_( dest->basis = (NurbsFloat*)_malloc_( k_size ) );
    _check_( dest->d_basis = (NurbsFloat*)_malloc_( k_size ) );

    strcpy(dest->label, orig->label);

    return dest;
}


/* Gets the point coordinates on a nurbs curve with parameter t */
NurbsVector3 nurbs_curve_get_point
    ( const NurbsCurve *nurbs
    , const NurbsFloat t
    )
{
    register int i;
    register NurbsFloat norm;  /* = Sum(basis_i * weight_i) */
    register NurbsFloat gw;
    NurbsFloat *Nu = nurbs->basis;
    NurbsVector3 point;
    int iknot, u0, u1;

    iknot = nurbs_basis_function
        ( Nu, t, nurbs->degree
        , nurbs->knot, nurbs->knot_length
        );

    /* Get the interval where there are non zero basis */
    u0 = iknot - nurbs->degree - 1;
    if (u0 < 0){
        u0 = 0;
    }

    u1 = iknot;
    if (u1 > (int)nurbs->cp_length - 1){
        u1 = nurbs->cp_length - 1;
    }
    if (u1 < 0){
        u1 = 0;
    }

    point.x = 0;
    point.y = 0;
    point.z = 0;
    norm = 0;

    for (i = u0; i <= u1; i++)
    {
        gw = Nu[i] * nurbs->cp[i].w;
        point.x += gw * nurbs->cp[i].x; 
        point.y += gw * nurbs->cp[i].y; 
        point.z += gw * nurbs->cp[i].z; 

        norm += gw;
    }

    point.x /= norm;
    point.y /= norm;
    point.z /= norm;

    return point;
}


/* Gets the derivate of a nurbs curve on the parameter t */
void nurbs_curve_get_derivatives
    ( NurbsVector3* deriv
    , NurbsVector3* point
    , const NurbsCurve *nurbs
    , const NurbsFloat t
    )
{
    register int i;
    register NurbsFloat norm;  /* = Sum(basis_i * weight_i) */
    register NurbsFloat dNorm;  /* = Sum(d_basis_i * weight_i) */
    register NurbsFloat gw;  
    register NurbsFloat dgw;  
    NurbsFloat *Nu = nurbs->basis;
    NurbsFloat *dNu = nurbs->d_basis;
    int iknot, u0, u1;

    iknot = nurbs_basis_derivate_function
        ( dNu, Nu, t, nurbs->degree
        , nurbs->knot, nurbs->knot_length
        );

    /* Get the interval where there are non zero basis */
    u0 = iknot - nurbs->degree - 1;
    if (u0 < 0){ 
        u0 = 0;
    }
    u1 = iknot;
    if (u1 > (int)nurbs->cp_length - 1){
        u1 = nurbs->cp_length - 1;
    }
    if (u1 < 0){
        u1 = 0;
    }

    point->x = 0;
    point->y = 0;
    point->z = 0;
    deriv->x = 0;
    deriv->y = 0;
    deriv->z = 0;
    norm = 0;
    dNorm = 0;

    for (i = u0; i <= u1; i++){
        gw = Nu[i] * nurbs->cp[i].w;
        point->x += gw * nurbs->cp[i].x;
        point->y += gw * nurbs->cp[i].y;
        point->z += gw * nurbs->cp[i].z;

        dgw = dNu[i] * nurbs->cp[i].w;
        deriv->x += dgw * nurbs->cp[i].x;
        deriv->y += dgw * nurbs->cp[i].y;
        deriv->z += dgw * nurbs->cp[i].z;

        norm += gw;
        dNorm += dgw;
    }

    point->x /= norm;
    point->y /= norm;
    point->z /= norm;
    deriv->x = (deriv->x * norm - point->x * dNorm) / (norm * norm);
    deriv->y = (deriv->y * norm - point->y * dNorm) / (norm * norm);
    deriv->z = (deriv->z * norm - point->z * dNorm) / (norm * norm);
}

