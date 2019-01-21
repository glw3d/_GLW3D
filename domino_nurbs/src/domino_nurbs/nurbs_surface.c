 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    Methods for operating with two dimensional NURBS surface

*******************************************************************************/ 

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "common/check_malloc.h"
#include "common/log.h"

#include "nurbs_internal.h"
#include "nurbs_basis.h"
#include "nurbs_surface.h"

/* Cross product between two vectors */
static inline NurbsVector3 cross(const NurbsVector3 a, const NurbsVector3 b)
{
    NurbsVector3 r;

    r.x = a.y * b.z - b.y * a.z;
    r.y = a.z * b.x - b.z * a.x;
    r.z = a.x * b.y - b.x * a.y;

    return r;
}


/* Equivalent to a C++ default constructor */
void nurbs_surface_init( NurbsSurface* surface )
{
    surface->cp_stream = nullptr;
    surface->knot_stream = nullptr;
    surface->cp = nullptr;
    surface->label[0] = '\0';
    surface->id = -1;

    surface->cp_length_u = 0;
    surface->cp_length_v = 0;

    surface->knot_u = nullptr;
    surface->knot_length_u = 0;

    surface->knot_v = nullptr;
    surface->knot_length_v = 0;

    surface->basis_u = nullptr;
    surface->basis_v = nullptr;

    surface->d_basis_u = nullptr;
    surface->d_basis_v = nullptr;
}


/* Allocates memory. */
NurbsSurface* nurbs_surface_alloc
    ( NurbsSurface* surface
    , const int num_cp_u  
    , const int num_cp_v
    , const int degree_u
    , const int degree_v
    )
{
    int i;
    size_t memsize;
    NurbsVector4* pv4;
    NurbsFloat* p1;

    if (surface == nullptr){
        _check_(surface = (NurbsSurface*)_malloc_(sizeof(NurbsSurface)));
        if (surface == nullptr){
            /* Out of memory */
            return nullptr;
        }
    }

    surface->degree_u = degree_u;
    surface->degree_v = degree_v;
    surface->cp_length_u = num_cp_u;
    surface->cp_length_v = num_cp_v;
    surface->knot_length_u = num_cp_u + degree_u + 1;
    surface->knot_length_v = num_cp_v + degree_v + 1;
    surface->cp = nullptr;
    surface->cp_stream = nullptr;
    surface->knot_stream = nullptr;

    if (num_cp_u < 1 || num_cp_v < 1 
        || surface->knot_length_u < 1 || surface->knot_length_v < 1){

        return surface;
    }

    memsize = surface->cp_length_u*sizeof(NurbsVector4*);
    _check_(surface->cp = (NurbsVector4**)_malloc_(memsize));
    if (surface->cp == nullptr){
        /* Out of memory */
        return nullptr;
    }

    memsize = surface->cp_length_u * surface->cp_length_v * sizeof(NurbsVector4);
    _check_( surface->cp_stream = (NurbsVector4*)_malloc_( memsize ) );
    if (surface->cp_stream == nullptr){
        /* Out of memory */
        return nullptr;
    }
    pv4 = (NurbsVector4*)surface->cp_stream;
    for (i = 0; i < surface->cp_length_u; i++){
        surface->cp[i] = pv4;
        pv4 += surface->cp_length_v;
    }

    memsize = (surface->knot_length_u + surface->knot_length_v) * 3
            * sizeof(NurbsFloat);

    _check_( surface->knot_stream = (NurbsFloat*)_malloc_( memsize ) );
    if (surface->knot_stream == nullptr){
        /* Out of memory */
        return nullptr;
    }
    p1 = surface->knot_stream;

    /* Set the pointers array */
    surface->knot_u = p1;
    p1 += surface->knot_length_u;

    surface->knot_v = p1;
    p1 += surface->knot_length_v;

    surface->basis_u = p1;
    p1 += surface->knot_length_u;

    surface->basis_v = p1;
    p1 += surface->knot_length_v;

    surface->d_basis_u = p1;
    p1 += surface->knot_length_u;

    surface->d_basis_v = p1;
    p1 += surface->knot_length_v;

    return surface;
}


/* Alias of nurbs_surface_alloc */
NurbsSurface* nurbs_surface_create
    ( NurbsSurface* surface
    , const int num_cp_u  
    , const int num_cp_v
    , const int degree_u
    , const int degree_v
    )
{
    return nurbs_surface_alloc( surface, num_cp_u, num_cp_v, degree_u, degree_v );
}

/* Releases the memory in the stack. Equivalent to a C++ destructor. */
void nurbs_surface_dispose(NurbsSurface* surface)
{
    if (surface == nullptr)
        return;

    if (surface->cp_stream != nullptr){
        free(surface->cp_stream);
    }
    if (surface->knot_stream != nullptr){
        free(surface->knot_stream);
    }
    if (surface->cp != nullptr){
        free(surface->cp);
    }

    surface->cp_stream = nullptr;
    surface->knot_stream = nullptr;
    surface->cp = nullptr;
    surface->id = -1;

    surface->cp_length_u = 0;
    surface->cp_length_v = 0;

    surface->knot_u = nullptr;
    surface->knot_length_u = 0;

    surface->knot_v = nullptr;
    surface->knot_length_v = 0;

    surface->basis_u = nullptr;
    surface->basis_v = nullptr;

    surface->d_basis_u = nullptr;
    surface->d_basis_v = nullptr;
}


/* Releases the memory of an array of nurbs. */
void nurbs_surface_free(NurbsSurface* nurbs_array, const int length)
{
    int i;

    if (nurbs_array == nullptr)
        return;

    for (i = 0; i < length; i++){
        nurbs_surface_dispose(&(nurbs_array[i]));
    }

    free(nurbs_array);
}

/* Copy the nurbs. Make sure that the dest nurbs is already empty. */
NurbsSurface* nurbs_surface_copy(NurbsSurface* dest, const NurbsSurface* orig)
{
    if (orig == nullptr){
        return nullptr;
    }

    if (dest == nullptr){
        _check_(dest = (NurbsSurface*)_malloc_(sizeof(NurbsSurface)));
        if (dest == nullptr){
            /* Out of memory */
            return nullptr;
        }
    }

    /* Initialize a new nurbs */
    dest = nurbs_surface_alloc(dest, orig->cp_length_u, orig->cp_length_v
        , orig->degree_u, orig->degree_v);

    if (dest == nullptr){
        return dest;
    }

    /* Copy id */
    dest->id = orig->id;

    /* Copy control points */
    memcpy(dest->cp_stream, orig->cp_stream
        , sizeof(NurbsFloat) * dest->cp_length_u * dest->cp_length_v * 4);

    /* Copy knots */
    memcpy(dest->knot_u, orig->knot_u, sizeof(NurbsFloat) * dest->knot_length_u);
    memcpy(dest->knot_v, orig->knot_v, sizeof(NurbsFloat) * dest->knot_length_v);

    /* Copy the description */
    strcpy(dest->label, orig->label);

    return dest;
}


/* Gets the point coordinates on a nurbs surface with parameters [u,v] */
NurbsVector3 nurbs_surface_get_point
    ( const NurbsSurface *nurbs
    , const NurbsFloat u
    , const NurbsFloat v
    )
{
    register int i, j;
    register NurbsVector4 cp;
    register NurbsFloat gw;
    register NurbsVector3 point = {0};
    NurbsFloat nu, nv;
    int knotU;  /* Lower knot index of the interval where t belongs */
    int knotV;  /* Lower knot index of the interval where t belongs */
    int iu0, iv0, iu1, iv1;  
    NurbsFloat norm;  /* = Sum(basis_ij * weight_ij) */

    knotU = nurbs_basis_function(nurbs->basis_u, u, nurbs->degree_u
        , nurbs->knot_u, nurbs->knot_length_u);

    knotV = nurbs_basis_function(nurbs->basis_v, v, nurbs->degree_v
        , nurbs->knot_v, nurbs->knot_length_v);

    norm = 0;

    /* Set the intervals where the basis functions are defined */
    iu0 = knotU - nurbs->degree_u - 1;
    if (iu0 < 0){ 
        iu0 = 0;
    }

    iu1 = knotU;
    if (iu1 > nurbs->cp_length_u - 1){
        iu1 = nurbs->cp_length_u - 1;
    }
    if (iu1 < 0){
        iu1 = 0;
    }

    iv0 = knotV - nurbs->degree_v - 1;
    if (iv0 < 0) {
        iv0 = 0;
    }
    iv1 = knotV;
    if (iv1 > nurbs->cp_length_v - 1){
        iv1 = nurbs->cp_length_v - 1;
    }
    if (iv1 < 0){
        iv1 = 0;
    }

    for (i = iu0; i <= iu1; i++){
        nu = nurbs->basis_u[i];
        for (j = iv0; j <= iv1; j++){
            nv = nurbs->basis_v[j];
            cp = nurbs->cp[i][j];
            gw = nu * nv * cp.w;

            point.x += gw * cp.x;
            point.y += gw * cp.y;
            point.z += gw * cp.z;

            norm += gw;
        }
    }

    point.x /= norm;
    point.y /= norm;
    point.z /= norm;

    return point;
}


/* Gets the derivate of the nurbs surface */
void nurbs_surface_get_derivatives
    ( NurbsVector3 *deriv_u /* (out) Derivative in u */
    , NurbsVector3 *deriv_v /* (out) Derivative in v */
    , NurbsVector3 *point   /* (out) Point coordinates {x, y , z} */
    , const NurbsSurface *nurbs /* NURBS surface */
    , const NurbsFloat u  /* u parameter */
    , const NurbsFloat v  /* v parameter */
    )
{
    int i, j;
    NurbsFloat ut = u, vt = v;
    register NurbsVector4 cp;
    register NurbsVector4 fpoint = {0};
    register NurbsVector4 fderiv_u = {0};
    register NurbsVector4 fderiv_v = {0};
    register NurbsFloat gw, guw, gvw;
    NurbsFloat nu, dnu, nv, dnv;
    NurbsFloat uL, vL, uL1, vL1;
    int knotU;  /* Lower knot index of the knot interval where t belongs */
    int knotV;  /* Lower knot index of the knot interval where t belongs */
    int iu0, iv0, iu1, iv1;  

    /* The formulation of the derivatives in NURBS is a backward derivative.
     * So, for the last control point, there is no derivative. 
     * The only work-around I found is to calculate the derivative to a point
     * very close, but not exactly on the last control point */
    uL = nurbs->knot_u[ nurbs->knot_length_u - nurbs->degree_u - 1 ];
    if (u >= uL){
        uL1 = nurbs->knot_u[ nurbs->knot_length_u - nurbs->degree_u - 2 ];
        ut = uL - (uL - uL1) / 256;
    }
    vL = nurbs->knot_v[ nurbs->knot_length_v - nurbs->degree_v - 1 ];
    if (v >= vL){
        vL1 = nurbs->knot_v[ nurbs->knot_length_v - nurbs->degree_v - 2 ];
        vt = vL - (vL - vL1) / 256;
    }

    knotU = nurbs_basis_derivate_function
        ( nurbs->d_basis_u, nurbs->basis_u
        , ut, nurbs->degree_u
        , nurbs->knot_u, nurbs->knot_length_u
        );

    knotV = nurbs_basis_derivate_function
        ( nurbs->d_basis_v, nurbs->basis_v
        , vt, nurbs->degree_v
        , nurbs->knot_v, nurbs->knot_length_v
        );

    /* Set the intervals where the basis functions are defined */
    iu0 = knotU - nurbs->degree_u - 1;
    if (iu0 < 0){
        iu0 = 0;
    }

    iu1 = knotU;
    /* This is already checked  */ 
    if (iu1 > nurbs->cp_length_u - 1){
        iu1 = nurbs->cp_length_u - 1;
    }
    if (iu1 < 0){
        iu1 = 0;
    }

    iv0 = knotV - nurbs->degree_v - 1;
    if (iv0 < 0){
        iv0 = 0;
    }

    iv1 = knotV;
    /* This is already checked  */ 
    if (iv1 > nurbs->cp_length_v - 1){
        iv1 = nurbs->cp_length_v - 1;
    }
    if (iv1 < 0){
        iv1 = 0;
    }

    for (i = iu0; i <= iu1; i++){
        nu = nurbs->basis_u[i];
        dnu = nurbs->d_basis_u[i];
        for (j = iv0; j <= iv1; j++){
            nv = nurbs->basis_v[j];
            dnv = nurbs->d_basis_v[j];
            cp = nurbs->cp[i][j];
            gw = nu * nv * cp.w; 
            guw = dnu * nv * cp.w; 
            gvw = nu * dnv * cp.w; 

            fpoint.x += gw * cp.x;
            fpoint.y += gw * cp.y;
            fpoint.z += gw * cp.z;
            fpoint.w += gw;
            fderiv_u.x += guw * cp.x;
            fderiv_u.y += guw * cp.y;
            fderiv_u.z += guw * cp.z;
            fderiv_u.w += guw;
            fderiv_v.x += gvw * cp.x;
            fderiv_v.y += gvw * cp.y;
            fderiv_v.z += gvw * cp.z;
            fderiv_v.w += gvw;
        }
    }

    point->x = fpoint.x / fpoint.w;
    point->y = fpoint.y / fpoint.w;
    point->z = fpoint.z / fpoint.w;

    deriv_u->x = (fderiv_u.x - fderiv_u.w * fpoint.x) / fpoint.w;
    deriv_u->y = (fderiv_u.y - fderiv_u.w * fpoint.y) / fpoint.w;
    deriv_u->z = (fderiv_u.z - fderiv_u.w * fpoint.z) / fpoint.w;

    deriv_v->x = (fderiv_v.x - fderiv_v.w * fpoint.x) / fpoint.w;
    deriv_v->y = (fderiv_v.y - fderiv_v.w * fpoint.y) / fpoint.w;
    deriv_v->z = (fderiv_v.z - fderiv_v.w * fpoint.z) / fpoint.w;
}


/* Gets the derivate of the nurbs surface */
void nurbs_surface_get_second_derivatives
    ( NurbsVector3 *deriv_uu  /* (out) Second derivative Suu */
    , NurbsVector3 *deriv_uv  /* (out) Second derivative Suv */
    , NurbsVector3 *deriv_vv  /* (out) Second derivative Svv */
    , NurbsVector3 *deriv_u   /* (out) First derivative Su */
    , NurbsVector3 *deriv_v   /* (out) First derivative Sv */
    , NurbsVector3 *point     /* (out) Point coordinates {x, y, z} */
    , const NurbsSurface *nurbs
    , NurbsFloat u
    , NurbsFloat v
    )
{
    int i, j;
    register NurbsVector4 cp;
    register NurbsVector4 fpoint = {0};
    register NurbsVector4 fderiv_u = {0};
    register NurbsVector4 fderiv_v = {0};
    register NurbsVector4 fderiv_uu = {0};
    register NurbsVector4 fderiv_uv = {0};
    register NurbsVector4 fderiv_vv = {0};
    NurbsFloat nu, nv, du, dv, d2u, d2v;
    int iKnotU;  /* Lower knot index of the knot interval where t belongs */
    int iKnotV;  /* Lower knot index of the knot interval where t belongs */
    int iu0, iv0, iu1, iv1;  
    NurbsFloat uL, vL, uL1, vL1;

    NurbsFloat *d2_basis_u = nullptr;
    NurbsFloat *d2_basis_v = nullptr;

    _check_(d2_basis_u = (NurbsFloat*)_malloc_
        (sizeof(NurbsFloat)*nurbs->knot_length_u));

    _check_( d2_basis_v = (NurbsFloat*)_malloc_
        (sizeof(NurbsFloat)*nurbs->knot_length_v));

    if (d2_basis_u == nullptr || d2_basis_u == nullptr){
        /* Out of memory */
        if (d2_basis_u != nullptr){
            free(d2_basis_u);
        }
        if (d2_basis_v != nullptr){
            free(d2_basis_v);
        }
        return;
    }

    /* The formulation of the derivatives in NURBS is a forward derivative.
     * So, for the last control point, there is no derivative. 
     * The only work-around I found is to calculate the derivative to a point
     * very close, but not exactly on the last control point */
    uL = nurbs->knot_u[ nurbs->knot_length_u - nurbs->degree_u - 1 ];
    if (u >= uL){
        uL1 = nurbs->knot_u[ nurbs->knot_length_u - nurbs->degree_u - 2 ];
        u = uL - (uL - uL1) / 256;
    }
    vL = nurbs->knot_v[ nurbs->knot_length_v - nurbs->degree_v - 1 ];
    if (v >= vL){
        vL1 = nurbs->knot_v[ nurbs->knot_length_v - nurbs->degree_v - 2 ];
        v = vL - (vL - vL1) / 256;
    }

    iKnotU = nurbs_basis_second_derivate_function
        ( d2_basis_u, nurbs->d_basis_u, nurbs->basis_u, u, nurbs->degree_u
        , nurbs->knot_u, nurbs->knot_length_u
        );

    iKnotV = nurbs_basis_second_derivate_function
        ( d2_basis_v, nurbs->d_basis_v, nurbs->basis_v, v, nurbs->degree_v
        , nurbs->knot_v, nurbs->knot_length_v
        );

    fpoint.x = 0;
    fpoint.y = 0;
    fpoint.z = 0;

    fderiv_u.x = 0;
    fderiv_u.y = 0;
    fderiv_u.z = 0;
    fderiv_v.x = 0;
    fderiv_v.y = 0;
    fderiv_v.z = 0;

    fderiv_uu.x = 0;
    fderiv_uu.y = 0;
    fderiv_uu.z = 0;
    fderiv_uv.x = 0;
    fderiv_uv.y = 0;
    fderiv_uv.z = 0;
    fderiv_vv.x = 0;
    fderiv_vv.y = 0;
    fderiv_vv.z = 0;

    fpoint.w = 0;
    fderiv_u.w = 0;
    fderiv_v.w = 0;
    fderiv_uu.w  = 0;
    fderiv_uv.w  = 0;
    fderiv_vv.w  = 0;

    /* Set the intervals where the basis functions are defined */
    iu0 = iKnotU - nurbs->degree_u - 1;
    if (iu0 < 0){
        iu0 = 0;
    }
    iu1 = iKnotU;
    if (iu1 < 0){
        iu1 = 0;
    }

    iv0 = iKnotV - nurbs->degree_v - 1;
    if (iv0 < 0){ 
        iv0 = 0;
    }
    iv1 = iKnotV;
    if (iv1 < 0){
        iv1 = 0;
    }

    for (i = iu0; i <= iu1; i++){
        nu = nurbs->basis_u[i];
        du = nurbs->d_basis_u[i];
        d2u = d2_basis_u[i];
        for (j = iv0; j <= iv1; j++){
            nv = nurbs->basis_v[j];
            dv = nurbs->d_basis_v[j];
            d2v = d2_basis_v[j];
            cp = nurbs->cp[i][j];
            fpoint.x += nu * nv * cp.x * cp.w;
            fpoint.y += nu * nv * cp.y * cp.w;
            fpoint.z += nu * nv * cp.z * cp.w;

            fderiv_u.x += du * nv * cp.x * cp.w;
            fderiv_u.y += du * nv * cp.y * cp.w;
            fderiv_u.z += du * nv * cp.z * cp.w;
            fderiv_v.x += dv * nu * cp.x * cp.w;
            fderiv_v.y += dv * nu * cp.y * cp.w;
            fderiv_v.z += dv * nu * cp.z * cp.w;

            fderiv_uu.x += d2u * nv * cp.x * cp.w;
            fderiv_uu.y += d2u * nv * cp.y * cp.w;
            fderiv_uu.z += d2u * nv * cp.z * cp.w;
            fderiv_uv.x += du * dv * cp.x * cp.w;
            fderiv_uv.y += du * dv * cp.y * cp.w;
            fderiv_uv.z += du * dv * cp.z * cp.w;
            fderiv_vv.x += nu * d2v * cp.x * cp.w;
            fderiv_vv.y += nu * d2v * cp.y * cp.w;
            fderiv_vv.z += nu * d2v * cp.z * cp.w;

            fpoint.w += nu * nv * cp.w;
            fderiv_u.w += du * nv * cp.w;
            fderiv_v.w += nu * dv * cp.w;
            fderiv_uu.w += d2u * nv * cp.w;
            fderiv_uv.w += du * dv * cp.w;
            fderiv_vv.w += nu * d2v * cp.w;
        }
    }

    point->x = fpoint.x / fpoint.w;
    point->y = fpoint.y / fpoint.w;
    point->z = fpoint.z / fpoint.w;

    deriv_u->x = (fderiv_u.x - fderiv_u.w * fpoint.x) / fpoint.w;
    deriv_u->y = (fderiv_u.y - fderiv_u.w * fpoint.y) / fpoint.w;
    deriv_u->z = (fderiv_u.z - fderiv_u.w * fpoint.z) / fpoint.w;

    deriv_v->x = (fderiv_v.x - fderiv_v.w * fpoint.x) / fpoint.w;
    deriv_v->y = (fderiv_v.y - fderiv_v.w * fpoint.y) / fpoint.w;
    deriv_v->z = (fderiv_v.z - fderiv_v.w * fpoint.z) / fpoint.w;

    deriv_uu->x = fderiv_uu.x - 2*fderiv_u.x*fderiv_u.w - fpoint.x*fderiv_uu.w;
    deriv_uu->x /= fpoint.w;
    deriv_uu->y = fderiv_uu.y - 2*fderiv_u.y*fderiv_u.w - fpoint.y*fderiv_uu.w;
    deriv_uu->y /= fpoint.w;
    deriv_uu->z = fderiv_uu.z - 2*fderiv_u.z*fderiv_u.w - fpoint.z*fderiv_uu.w;
    deriv_uu->z /= fpoint.w;

    deriv_uv->x = fderiv_uv.x - fderiv_u.x*fderiv_v.w 
        - fderiv_v.x*fderiv_u.w - fpoint.x*fderiv_uv.w;
    deriv_uv->x /= fpoint.w;
    deriv_uv->y = fderiv_uv.y - fderiv_u.y*fderiv_v.w 
        - fderiv_v.y*fderiv_u.w - fpoint.y*fderiv_uv.w;
    deriv_uv->y /= fpoint.w;
    deriv_uv->z = fderiv_uv.z - fderiv_u.z*fderiv_v.w 
        - fderiv_v.z*fderiv_u.w - fpoint.z*fderiv_uv.w;
    deriv_uv->z /= fpoint.w;

    deriv_vv->x = fderiv_vv.x - 2*fderiv_v.x*fderiv_v.w - fpoint.x*fderiv_vv.w;
    deriv_vv->x /= fpoint.w;
    deriv_vv->y = fderiv_vv.y - 2*fderiv_v.y*fderiv_v.w - fpoint.y*fderiv_vv.w;
    deriv_vv->y /= fpoint.w;
    deriv_vv->z = fderiv_vv.z - 2*fderiv_v.z*fderiv_v.w - fpoint.z*fderiv_vv.w;
    deriv_vv->z /= fpoint.w;
}

/* Calculates the normal by finite diferences.  */
NurbsVector3 nurbs_surface_get_normal_by_FD
    ( const NurbsSurface *nurbs
    , const NurbsFloat u
    , const NurbsFloat v
    )
{
    NurbsVector3 normal;
    NurbsVector3 p0, p1;
    NurbsVector3 a, b;
    NurbsVector3 n;
    NurbsFloat t0, t1;
    NurbsFloat mod;
    const NurbsFloat zero_dot_one = (NurbsFloat)0.001;

    /* u direction */
    t0 = u - zero_dot_one;
    t1 = u + zero_dot_one;

    p0 = nurbs_surface_get_point(nurbs, t0, v);
    p1 = nurbs_surface_get_point(nurbs, t1, v);

    a.x = p1.x - p0.x;
    a.y = p1.y - p0.y;
    a.z = p1.z - p0.z;

    /* v direction */
    t0 = v - zero_dot_one;
    t1 = v + zero_dot_one;

    p0 = nurbs_surface_get_point(nurbs, u, t0);
    p1 = nurbs_surface_get_point(nurbs, u, t1);

    b.x = p1.x - p0.x;
    b.y = p1.y - p0.y;
    b.z = p1.z - p0.z;

    n = cross(a, b);
    mod = n.x*n.x + n.y*n.y + n.z*n.z;
    mod = (NurbsFloat)sqrt(mod);
    if (mod > 0){
        normal.x = n.x / mod;
        normal.y = n.y / mod;
        normal.z = n.z / mod;
    }
    else{
        normal.x = 0;
        normal.y = 0;
        normal.z = 0;
    }

    return normal;
}


/* Calculates the normal to the nurbs surface.  
 * Take care that on kinks and edges there is no normal and may returns {0,0,0}. */
NurbsVector3 nurbs_surface_get_normal
    ( const NurbsSurface *nurbs
    , const NurbsFloat u
    , const NurbsFloat v
    )
{
    NurbsVector3 normal;
    NurbsVector3 a, b, p;
    NurbsVector3 n;
    NurbsFloat mod;

    nurbs_surface_get_derivatives(&a, &b, &p, nurbs, u, v);

    n = cross(a, b);
    mod = n.x*n.x + n.y*n.y + n.z*n.z;

    /* If fails, usually is because one derivative is zero */
    if (mod > 0){
        mod = (NurbsFloat)sqrt(mod);
        normal.x = n.x / mod;
        normal.y = n.y / mod;
        normal.z = n.z / mod;
    }
    else{
        normal.x = 0;
        normal.y = 0;
        normal.z = 0;
    }

    return normal;
}


/* Reduces the order of the knots by one order */
static void reduceOrderKnot(NurbsFloat *knot, int *knot_length, int* degree)
{
    int j, k = 0;
    int ncp = *knot_length - *degree - 1;
    NurbsFloat alpha = (NurbsFloat)1 / (ncp - *degree + 1);

    j = *degree;
    for (k = 1; k < ncp+1; k++){
        knot[j] = k*alpha*knot[j] + (1-k*alpha)*knot[j+1];
        j++;
    }

    *knot_length -= 1;
    *degree -= 1;
}


/* Creates a reduced second order nurbs */
NurbsSurface* nurbs_surface_reduce_order2
    ( NurbsSurface* nurbs2
    , const NurbsSurface* surface
    )
{
    int i;
    if (surface == nullptr){
        return nullptr;
    }

    nurbs2 = nurbs_surface_copy(nurbs2, surface);

    /* Reduce the original nurbs to order 2 */
    for (i = 0; i < surface->degree_u - 1; i++){
        reduceOrderKnot(nurbs2->knot_u, &nurbs2->knot_length_u, &nurbs2->degree_u);
    }
    for (i = 0; i < surface->degree_v - 1; i++){
        reduceOrderKnot(nurbs2->knot_v, &nurbs2->knot_length_v, &nurbs2->degree_v);
    }

    nurbs2->degree_u = 1;
    nurbs2->degree_v = 1;

    return nurbs2;
}


