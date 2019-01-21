 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    The control box uses its own basis formulation. 
    I was not able yet to generalize the formulation, but I am working on that.
    They are close related with Bezier and Hermite basis.
    The main difference with conventional NURBS is that the curve or surface,
    always pass through a control point.

*******************************************************************************/

#ifndef _DOMINO_NURBS_FREE_FORM_H
#define _DOMINO_NURBS_FREE_FORM_H

#include <stdio.h>

#include "nurbs_controlbox_data.h"

#ifdef  __cplusplus
  extern "C" {
#endif
      

/** Equivalent to a default constructor */
void nurbs_controlbox_init( NurbsControlBox* ffd );

/** Initializes or creates a control box object. */
NurbsControlBox* nurbs_controlbox_alloc
    ( NurbsControlBox* cb /** If nullptr is pass, creates a new data structure */
    , const int num_cp_u /** Number of control points in the u direction. */
    , const int num_cp_v /** Number of control points in the v direction. */
    , const int num_cp_w /** Number of control points in the w direction. */
    );

/** Alias of nurbs_controlbox_alloc. */
NurbsControlBox* nurbs_controlbox_create
    ( NurbsControlBox* cb
    , const int num_cp_u
    , const int num_cp_v
    , const int num_cp_w
    );

/** Releases memory resources. */
void nurbs_controlbox_dispose(NurbsControlBox* ffd);

/** Releases memory resources. */
void nurbs_controlbox_free( NurbsControlBox* p_ffd, const int len );

/** Calculates the basis functions */
#ifndef SWIG 
int nurbs_controlbox_basis_function
    ( NurbsFloat basis[]        /** (out) Basis functions */
    , const NurbsFloat t        /** Parameter value */
    , const int num_cp          /** Number of control points */
    , const int order           /** Order */
    , const int basis_equation  /** Basis equation (1:bezier 0:b-spline) */
    );
#endif

/** Calculates the point coordinates from the parametric coordinates */
NurbsVector3 nurbs_controlbox_get_point
    ( const NurbsControlBox* cb   /** Control box */
    , const NurbsFloat u    /** Parameter value */
    , const NurbsFloat v    /** Parameter value */
    , const NurbsFloat w    /** Parameter value */
    );

/** Calculates the derivative of the basis functions */
#ifndef SWIG 
int nurbs_controlbox_d_basis_function
    ( NurbsFloat d_basis[]      /* (out) Derivative of the basis functions */
    , NurbsFloat basis[]        /* (out) Basis functions */
    , const NurbsFloat t        /* Parameter value */
    , const int num_cp          /* Number of control points */
    , const int order           /* Order */
    , const int basis_equation  /** Basis equation */
    );
#endif

/** Calculates the point coordinates from the parametric coordinates */
#ifndef SWIG 
void nurbs_controlbox_get_derivatives
    ( const NurbsControlBox* cb   /** Control box */
    , NurbsVector3 *deriv_u   /* (out) First derivative Su */
    , NurbsVector3 *deriv_v   /* (out) First derivative Sv */
    , NurbsVector3 *deriv_w   /* (out) First derivative Sw */
    , NurbsVector3 *point     /* (out) Point coordinates {x, y, z} */
    , const NurbsFloat u    /** Parameter value */
    , const NurbsFloat v    /** Parameter value */
    , const NurbsFloat w    /** Parameter value */
    );
#endif

/** Copies the control box. 
 * Make sure that the destine is already empty to avoid memory leaks. */
void nurbs_controlbox_copy
    ( NurbsControlBox* dest, const NurbsControlBox* orig );

/** Calculates the parametric values from the spatial coordinates */
NurbsVector3 nurbs_controlbox_inversion
    ( const NurbsControlBox* cb
    , const NurbsFloat x
    , const NurbsFloat y
    , const NurbsFloat z
    , const NurbsFloat epsilon
    , NurbsFloat* OUTPUT
    );

/** Calculates the parametric values from the spatial coordinates 
  * using an iterative Newthon-Raphson */
#ifndef SWIG 
NurbsVector3 nurbs_controlbox_inversion_newton
    ( const NurbsControlBox* cb
    , const NurbsVector3 p
    , const NurbsVector3 t
    , const NurbsFloat epsilon
    , NurbsFloat* err
    );

#endif

#ifdef  __cplusplus
  }
#endif

#endif /* _DOMINO_NURBS_LITE_SURFACE_H */

/**/