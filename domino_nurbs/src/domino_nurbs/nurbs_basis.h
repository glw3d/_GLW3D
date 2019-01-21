 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    Methods related with basis functions (only for NURBS curves and surfaces)

*******************************************************************************/


#ifndef _NURBS_BASIS_FUNCTIONS_H
#define _NURBS_BASIS_FUNCTIONS_H

#include "nurbs_definitions.h"

#ifdef  __cplusplus
  extern "C" {
#endif

/*******************************************************************************
*  Description:
*     Finds the knot interval a parameter belongs to, using a bisection algorit.
*  Return Values:
*    integer values
*    @return lower knot index of the knot interval.
*
*******************************************************************************/
int nurbs_basis_knot_index
    ( const NurbsFloat t        /** Parameter */
    , const NurbsFloat knots[]  /** Knot vector */
    , const int knots_length    /** Knot vector length */
    );


/*******************************************************************************
*  Description:
*     Calculates the basis functions for a parameter value.
*  Return Values:
*    integer values
*    @return lower knot index of the knot interval the parameter belongs to.
*
*******************************************************************************/
int nurbs_basis_function
    ( NurbsFloat basis[]        /** (out) Array of basis values */
    , const NurbsFloat t        /** Parameter value */
    , const int degree          /** Degree (degree = order - 1) */
    , const NurbsFloat knots[]  /** Knot vectors */
    , const int knots_length    /** Knot length */
    );


/*******************************************************************************
*  Description:
*     Gets the valid knot interval where the nurbs is defined.
*  Return Values:
*
*******************************************************************************/
void nurbs_basis_get_parameter_interval
    ( NurbsFloat* first         /** (out) Lower knot interval */
    , NurbsFloat* last          /** (out) Upper knot interval */
    , const NurbsFloat knots[]  /** Knot vector */
    , const int knots_length    /** Knot vector length */
    , const int degree          /** Degree (degree = order - 1) */
    );


/*******************************************************************************
*  Description:
*     Calculates the first derivate of the basis for a parameter value.
*  Return Values:
*    integer values
*    @return lower knot index of the knot interval the parameter belongs to.
*
*******************************************************************************/
int nurbs_basis_derivate_function
    ( NurbsFloat d_basis[]      /** (out) Derivative of the basis function */
    , NurbsFloat basis[]        /** (out) Basis functions */
    , const NurbsFloat t        /** Parameter */
    , const int degree          /** Degree */
    , const NurbsFloat knots[]  /** Knot vector */
    , const int knots_length    /** Knot vector length */
    );

/*******************************************************************************
*  Description:
*     Calculates the second derivate of the basis for a parameter value.
*  Return Values:
*    integer values
*    @return lower knot index of the knot interval the parameter belongs to.
*
*******************************************************************************/
int nurbs_basis_second_derivate_function
    ( NurbsFloat d2_basis[]     /** (out) Second derivative of the basis func */
    , NurbsFloat d_basis[]      /** (out) Derivative of the basis function */
    , NurbsFloat basis[]        /** (out) Basis functions */
    , const NurbsFloat t        /** Parameter */
    , const int degree          /** Degree */
    , const NurbsFloat knots[]  /** Knot vector */
    , const int knots_length    /** Knot vector length */
    );

/*******************************************************************************
*  Description:
*     Calculates the basis value (using the recursive algorithm).
*******************************************************************************/
NurbsFloat nurbs_basis_term
    ( const NurbsFloat x[]      /* Knot vector */
    , const NurbsFloat t        /* Parameter */
    , const int degree          /* Degree of the basis*/
    , const int iknot           /* knot interval: x[i] < k < x[i+1] */
    , const int index           /* Index of the basis vector */
    );

#ifdef  __cplusplus
  }
#endif

#endif /* _NURBS_BASIS_FUNCTIONS_H */


/**/