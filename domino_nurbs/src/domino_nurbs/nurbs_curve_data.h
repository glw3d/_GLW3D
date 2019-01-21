 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    NURBS curves

*******************************************************************************/

#ifndef _NURBS_CURVE_DATA_H_
#define _NURBS_CURVE_DATA_H_

#include "nurbs_definitions.h"

/** Data structure for NURBS curves */
typedef struct NurbsCurve_
{
    int id;     /**< Curve id; can be used to identify different NURBS */

    int degree; /**< Degree of the nurbs. 
                   * (degree = number_knots - number_cp - 1). 
                   * Also: degree = order - 1 
    * Actually, I think there is a misundestanding in the literature
    * and what they call degree is in fact the order. */

    int cp_length;    /**< Size of the array of control points */
    NurbsVector4* cp; /**< Array of control points. */  
    NurbsFloat* knot; /**< Array of knots. */
    int knot_length;  /**< Length of the knot vector. */

    NurbsFloat *basis;  /**< Array of basis. 
    * The length of the basis arrays are the same as the knot vector.
    * Actually there is the same number of basis as control points */

    NurbsFloat *d_basis;  /**< Array of the derivate of the basis. */ 

    /** A small description of the curve. */
    char* label; 

    /** In case of an array, pointers the next item */
    struct NurbsCurve_* next;

}NurbsCurve;


#endif /* _NURBS_CURVE_DATA_H_ */

/**/