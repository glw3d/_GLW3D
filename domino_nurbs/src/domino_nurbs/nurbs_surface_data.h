 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    NURBS surfaces

*******************************************************************************/ 

#ifndef _NURBS_SURFACE_H_
#define _NURBS_SURFACE_H_

#include "nurbs_definitions.h"

/** Data structure for NURBS surfaces */
typedef struct NurbsSurface_
{
    /** A small description of the surface. */
    char label[DOMINO_NURBS_LABEL_LEN];

    int id;         /**< It is basically used to differenciate NURBS. */

    int degree_u;  /**< Degree of the nurbs in the u direction. */
                   /**< (degree_u = number_knots_u - number_cp_u - 1). */
                   /**< Also: degree = order - 1 */

    int degree_v;  /**< Degree of the nurbs in the v direction  */
                   /**< (degree_v = number_knots_v - number_cp_v - 1). */
                   /**< Also: degree = order - 1 */

    int cp_length_u; /**< Size of the control point matrix in the u direction. */
    int cp_length_v; /**< Size of the control point matrix in the v direction. */

    int knot_length_u;  /**< Length of the knot vector in the u direction. */
    int knot_length_v;  /**< Length of the knot vector in the v direction. */

    /* Here is where knots, buffers and control points are actually stored. */
    NurbsFloat* knot_stream;    /**< Stream of data of the knots and buffers. */
    NurbsVector4* cp_stream;    /**< Stream of data of the control points. */

    /* ^^^-DATA-^^^ */
    /**********************************************************************/
    /* vvv-POINTERS-vvv */

    NurbsFloat* knot_u;  /**< Array of knots in the u direction. */

    NurbsFloat* knot_v;  /**< Array of knots in the v direction. */

    NurbsVector4** cp;  /**< Array of pointers that allows the sweet notation */
                        /**< cp[u][v].{xyzw} */

    NurbsFloat *basis_u;  /**< Array of basis in the u direction.
    * The length of the basis arrays are the same as the knot vector.
    * Actually there is the same number of basis as control points */

    NurbsFloat *basis_v;  /**< Array of basis in the v direction.
    * The length of the basis arrays are the same as the knot vector.
    * Actually there is the same number of basis as control points */

    NurbsFloat *d_basis_u; /**< Array of derivate of basis in the u direction.*/

    NurbsFloat *d_basis_v; /**< Array of derivate of basis in the v direction.*/

    /** In the case of an array, pointers the next item. */
    struct NurbsSurface_* next;

}NurbsSurface;

#endif /*_NURBS_SURFACE_H_ */


/**/