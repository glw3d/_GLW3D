 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    NURBS curves

*******************************************************************************/

#ifndef _NURBS_CURVE_H
#define _NURBS_CURVE_H

#include <stdio.h>

#include "nurbs_curve_data.h"
#include "nurbs_basis.h"

#ifdef  __cplusplus
  extern "C" {
#endif

/*******************************************************************************
*  Description:
*     Equivalent to a C++ default constructor
*******************************************************************************/
void nurbs_curve_init( NurbsCurve* curve );


/*******************************************************************************
*  Description:
*     Initializes a NURBS curve data. 
*  Return Values:
*    NurbsCurve pointer
*    @return pointer to a NURBS curve data structure; the same pointer as 
*    parameter or a new data structure created in the heap, if nullptr is pass 
*    as argument.
*******************************************************************************/
NurbsCurve* nurbs_curve_alloc
    ( NurbsCurve* curve             /** 'nullptr' or the NURBS data structure */
    , const int num_control_points  /** Number of control points */
    , const int degree    /** Degree of the NURBS curve (degree = order-1) */
    );

/*******************************************************************************
*  Description:
*     Alias of nurbs_curve_alloc. 
*  Return Values:
*    NurbsCurve pointer
*    @return pointer to a NURBS curve data structure; the same pointer as 
*    parameter or a new data structure created in the heap, if nullptr is pass 
*    as argument.
*******************************************************************************/
NurbsCurve* nurbs_curve_create
    ( NurbsCurve* curve
    , const int num_control_points
    , const int degree
    );

/*******************************************************************************
*  Description:
*     Releases the memory from a NURBS curve. Equivalent to a C++ destructor.
*     This is meant to be used with a data structure created in the stack.
*     This routine realeses the data and the pointer itself.
*  Return Values:
*    void.
*******************************************************************************/
void nurbs_curve_dispose(NurbsCurve* nurbs);


/*******************************************************************************
*  Description:
*     Releases the memory of all curves defined in the array.
*     This is meant to be used with a data structure created in the stack.
*     This routine realeses the data but NOT the pointer itself.
*  Return Values:
*    void.
*******************************************************************************/
void nurbs_curve_free
    ( NurbsCurve* p_curve_array  /** Pointer to the array */
    , const int length            /** Number of curves in the array */
    );


/*******************************************************************************
*  Description:
*     Makes a copy of the nurbs curve. 
*  Return Values:
*     NurbsCurve pointer
* @return the same pointer as the first argument. If nullptr is pass it returns
*  a new pointer to a NURBS data structure created in the heap.
*******************************************************************************/
NurbsCurve* nurbs_curve_copy
    ( NurbsCurve* dest        /** Target pointer  */
    , const NurbsCurve *orig  /** Pointer with the data to be copied */
    );


/*******************************************************************************
*  Description:
*    Gets the point coordinates on a nurbs curve with parameter t.
*  Return Values:
*    Point coordinates {x, y, z}.
*******************************************************************************/
NurbsVector3 nurbs_curve_get_point
	( const NurbsCurve *nurbs /** Pointer to the NURBS data structure */
    , const NurbsFloat t      /** Parameter */
    );

/*******************************************************************************
*  Description:
*    Gets the derivative on a nurbs curve at parameter t.
*  Return Values:
*    void.
*******************************************************************************/
#ifndef SWIG 
void nurbs_curve_get_derivatives
    ( NurbsVector3* deriv   /** (out) Derivative coordinates {x, y, z} */
    , NurbsVector3* point   /** (out) Point coordinates {x, y, z} */
    , const NurbsCurve *nurbs /** Pointer to NURBS data structure */
    , const NurbsFloat t      /** parameter */
    );
#endif

#ifdef  __cplusplus
  }
#endif

#endif /* _NURBS_CURVE_H */


/**/