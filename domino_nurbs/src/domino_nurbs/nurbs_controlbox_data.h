 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    Data structure for the control box. 
    To create a curve or surface control box, set the order to 0;
    although it is not intended to be used in that way.

*******************************************************************************/

#ifndef _DOMINO_NURBS_CONTROLBOX_DATA_H
#define _DOMINO_NURBS_CONTROLBOX_DATA_H

#include <stdio.h>

#include "nurbs_definitions.h"
      
/** Data structure for free form deformation box. 
  * The order of the nurbs is always 1 and the knots always have the form
  * {0, 0, 1/N, 2/N, ... , (N-1)/N, 1, 1}
  */
typedef struct NurbsControlBox_
{
    int id;         /**< Identification number of the entity. */

    int cp_length_u; /**< Size of the control point matrix in the u direction. */
    int cp_length_v; /**< Size of the control point matrix in the v direction. */
    int cp_length_w; /**< Size of the control point matrix in the w direction. */

    int order_u; /**< Order in the u direction. */
    int order_v; /**< Order in the u direction. */
    int order_w; /**< Order in the u direction. */

    NurbsVector3* cp_stream; /**< Stream of data of the control points. */

    NurbsVector3*** cp; /**< Array of pointers that allows the sweet notation */
                        /**< cp[u][v][w].{xyzw} */

    /** A label or small description of the entity. */
    char label[DOMINO_NURBS_LABEL_LEN];

    int basis_equation; /**< 0.- NURBS 1.- Bezier */

    /** In an array, pointers to the next element. */
    struct NurbsControlBox_* next;

}NurbsControlBox;


#endif /* _DOMINO_NURBS_CONTROLBOX_DATA_H */


/**/