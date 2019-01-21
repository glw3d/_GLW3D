 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    NURBS surfaces

*******************************************************************************/ 

#ifndef _DOMINO_NURBS_SURFACE_H
#define _DOMINO_NURBS_SURFACE_H

#include <stdio.h>

#include "nurbs_basis.h"
#include "nurbs_surface_data.h"

#ifdef  __cplusplus
  extern "C" {
#endif


  /** Data estructure to store the intersections between two surface nurbs. */
typedef struct
{
    NurbsFloat u, v, m, n;
}NurbsIntersection;


/*******************************************************************************
*  Description:
*    Equivalent to a C++ default constructor. 
*******************************************************************************/
void nurbs_surface_init( NurbsSurface* surface );

/*******************************************************************************
*  Description:
*    Reserves the memory for a nurbs data structure. 
*  Return Values:
*    NurbsSurface pointer
*  @return the same pointer as the first parameter or a new pointer to a NURBS 
*    data structure if nullptr is pass. Returns nullptr if the memory is 
*    exhausted.
*******************************************************************************/
NurbsSurface* nurbs_surface_alloc
    ( NurbsSurface* surface /** nurbs surface pointer */
    , const int num_cp_u   /** number of control points in the u direction. */
    , const int num_cp_v   /** number of control points in the v direction. */
    , const int degree_u   /** degree of the nurbs surface in the u direction. */
    , const int degree_v   /** degree of the nurbs surface in the v direction. */
    );

/*******************************************************************************
*  Description:
*    Alias of nurbs_surface_alloc
*  Return Values:
*    NurbsSurface pointer
*  @return the same pointer as the first parameter or a new pointer to a NURBS 
*    data structure if nullptr is pass. Returns nullptr if the memory is 
*    exhausted.
*******************************************************************************/
NurbsSurface* nurbs_surface_create
    ( NurbsSurface* surface
    , const int num_cp_u  
    , const int num_cp_v
    , const int degree_u
    , const int degree_v
    );

/*******************************************************************************
*  Description:
*    Releases the memory. Equivalent to a C++ destructor. 
*    This method is meant to release a nurbs created in the stack memory
*    with nurbs_surface_create() or nurbs_surface_xxx_import()
*    (Releases the data, but not the pointer)
*  Return Values:
*    NurbsSurface pointer
*  @return pointer to a NURBS data structure created in the heap.   
*******************************************************************************/
void nurbs_surface_dispose(NurbsSurface* nurbs);

/*******************************************************************************
*  Description:
*    Releases the memory of a NURBS array. Equivalent to a C++ destructor. 
*    This method is meant to release a nurbs created in the heap memory
*    with nurbs_surface_init() or as a local variable.
*    (Releases the data and the pointer)
*  Return Values:
*    void
*******************************************************************************/
void nurbs_surface_free
    ( NurbsSurface* p_nurbs_array  /** Nurbs array pointer */
    , const int length              /** length of the array */
    );

/*******************************************************************************
*  Description:
*    Copies one nurbs. If the first parameter is nullptr, it clones the dato to
*    a new nurbs data structure in the heap.
*  Return Values:
*    NurbsSurface pointer
*  @return the same pointer as the first parameter or a new pointer to a NURBS 
*    data structure if nullptr is pass. Returns nullptr if the memory is 
*    exhausted
*******************************************************************************/
NurbsSurface* nurbs_surface_copy
    ( NurbsSurface* dest        /** destination nurbs pointer */
    , const NurbsSurface* orig  /** nurbs to be copied */
    );


/*******************************************************************************
*  Description:
*    Gets the space coordinates {x,y,z} of a point with parameters {u,v}.
*  Return Values:
*    Returns point coordinates {x, y, z}
*******************************************************************************/
NurbsVector3 nurbs_surface_get_point
    ( const NurbsSurface *nurbs /** nurbs surface pointer */
    , const NurbsFloat u        /** first parametric coordinate (etha) */
    , const NurbsFloat v        /** second parametric coordinate (chi) */
    );


/*******************************************************************************
*  Description:
*    Gets the derivates of the nurbs surface on {u,v}.
*  Return Values:
*    void
*******************************************************************************/
#ifndef SWIG 
void nurbs_surface_get_derivatives
    ( NurbsVector3* deriv_u     /** (out) first derivative vector {x,y,z} */
    , NurbsVector3* deriv_v     /** (out) second derivative vector {x,y,z} */
    , NurbsVector3* point       /** (out) space coordinates at {u,v} */
    , const NurbsSurface *nurbs /** nurbs surface pointer */
    , const NurbsFloat u              /** first parametric coordinate (etha) */
    , const NurbsFloat v              /** second parametric coordinate (chi) */
    );
#endif

/*******************************************************************************
*  Description:
*    Gets the second derivates of the nurbs surface on (u,v).
*  Return Values:
*    void
*******************************************************************************/
#ifndef SWIG 
void nurbs_surface_get_second_derivatives
    ( NurbsVector3* deriv_uu    /** (out) second derivate vector {x,y,z} */
    , NurbsVector3* deriv_uv    /** (out) second derivate vector {x,y,z} */
    , NurbsVector3* deriv_vv    /** (out) second derivate vector {x,y,z} */
    , NurbsVector3* deriv_u     /** (out) first derivate vector {x,y,z} */
    , NurbsVector3* deriv_v     /** (out) first derivate vector {x,y,z} */
    , NurbsVector3* point       /** (out) space coordinates at {u,v} */
    , const NurbsSurface *nurbs /** nurbs surface pointer */
    , NurbsFloat u              /** first parametric coordinate (etha) */
    , NurbsFloat v              /** second parametric coordinate (chi) */
    );
#endif

/*******************************************************************************
*  Description:
*    Calculates the inversion point using a first order method. 
*    The algorithm basically is Newton-Raphson projecting PQ onto the derivative
*    Must provide an initial quest in pu and pv, which are also used to return
*    the inversion solution. 
*  Return Values:
*    integer
*  @return 1 if one viable solution is found.
*******************************************************************************/
#ifndef SWIG 
int nurbs_surface_inversion_proj
    ( NurbsFloat *pu    /** (in) initial quest (out) solution of the inversion */
    , NurbsFloat *pv    /** (in) initial quest (out) solution of the inversion */
    , const NurbsVector3 *point  /** space coordinates of the pointer at {u,v} */
    , const NurbsSurface *surface /** nurbs surface pointer */
    , const NurbsFloat epsilon    /** stop condition e.g. epsilon = 1e-6  */
    );
#endif

/*******************************************************************************
*  Description:
*    Calculates the inversion point using a bilineal quad algorithm. 
*    Must provide an initial quest in pu and pv, which are also used to return
*    the inversion solution.*  Return Values:
*    NurbsSurface pointer
*  Return Values:
*    integer
*  @return 1 if one viable solution is found.
*******************************************************************************/
#ifndef SWIG 
int nurbs_surface_inversion_quad
    ( NurbsFloat *pu    /** (in) initial quest (out) solution of the inversion */
    , NurbsFloat *pv    /** (in) initial quest (out) solution of the inversion */
    , const NurbsVector3 *point /** space coordinates of the pointer at {u,v} */
    , const NurbsSurface *surface   /** nurbs surface pointer */
    , const NurbsFloat epsilon      /** stop condition e.g. epsilon = 1e-6  */
    );
#endif

/*******************************************************************************
*  Description:
*    Estimates the inversion solution using the intersection of the 
*    normal on a reduced second order nurbs. 
*  Return Values:
*    integer
*  @return greater than zero is at least one solution is found.
*******************************************************************************/
#ifndef SWIG 
int nurbs_surface_estimation_normal
    ( NurbsFloat *pu    /* returned solution */
    , NurbsFloat *pv    /* returned solution */
    , const NurbsSurface *surface_orig   /* non reduced NURBS surface */
    , const NurbsSurface *surface_order2 /* second order NURBS surface */
    , const NurbsVector3 *point  /* coordinates of the point {x, y, z} */
    , const NurbsVector3 *normal /* surface normal (should be normalized) */
    , const NurbsFloat gamma     /* relaxing factor used by the estimation */
    );
#endif
/*******************************************************************************
*  Description:
*    First, estimates the inversion solution using the intersection of the 
*    normal on a reduced second order nurbs. Then performs an iterative method.
*  Return Values:
*    integer
*  @return greater than zero is at least one solution is found.
*******************************************************************************/
#ifndef SWIG 
int nurbs_surface_inversion_normal
    ( NurbsFloat *pu    /** (in) initial quest (out) solution of the inversion*/
    , NurbsFloat *pv    /** (in) initial quest (out) solution of the inversion*/
    , const NurbsSurface *surface_orig   /** non reduced nurbs surface pointer*/
    , const NurbsSurface *surface_order2 /**second order nurbs surface pointer*/
    , const NurbsVector3 *point /** space coordinates of the point {x, y, z}  */
    , const NurbsVector3 *surface_normal /**surface normal direction {x, y, z}*/
    , const NurbsFloat epsilon           /**stop condition iterative (eg 1e-6)*/
    , const NurbsFloat gamma             /** relaxing factor (eg 0, 0.25, 0.5)*/
    );
#endif

/*******************************************************************************
*  Description:
*   Estimates an estimation solution for the inversion by comparing 
*   the distance with a subgrid of values.
*   Always gives a solution.
*  Return Values:
*    void
*******************************************************************************/
#ifndef SWIG 
void nurbs_surface_estimation_subgrid(NurbsFloat *pu, NurbsFloat *pv
    , const NurbsSurface *surface /** non reduced NURBS surface pointer       */
    , const NurbsVector3 *point   /** coordinates of the point {x, y, z} */
    , const int grid_density   /** number of subgrid divisions (1, 5,... 100) */
    );
#endif

/*******************************************************************************
*  Description:
*   Calculates an estimation solution for the inversion by comparing 
*   the distance with a subgrid of values. Then performs an iterative method.
*   Always gives a solution.
*  Return Values:
*    void
*******************************************************************************/
#ifndef SWIG 
void nurbs_surface_inversion_subgrid(NurbsFloat *pu, NurbsFloat *pv
    , const NurbsSurface *surface /** non reduced NURBS surface pointer       */
    , const NurbsVector3 *point   /** coordinates of the point {x, y, z} */
    , const NurbsFloat epsilon    /** stop condition for iterative (eg: 1e-6) */
    , const int grid_density   /** number of subgrid divisions (1, 5,... 100) */
    );
#endif

/*******************************************************************************
*  Description:
*    Estimates the inversion by calculating the surface minimun distance 
*    with an equivalent second order nurbs.
*  Return Values:
*    integer
*  @return greater than zero is at least one solution is found.
*******************************************************************************/
#ifndef SWIG 
int nurbs_surface_estimation_min_distance
    ( NurbsFloat *pu                     /** returned value */
    , NurbsFloat *pv                     /** returned value */
    , const NurbsSurface *surface_orig   /** non reduced NURBS surface */
    , const NurbsSurface *surface_order2 /** second order NURBS surface */
    , const NurbsVector3 *point  /** coordinates of the point {x, y, z} */
    , const NurbsFloat gamma     /** relaxing factor (eg 0, 0.25, 0.50) */
    );
#endif

/*******************************************************************************
*  Description:
*    Calculates the inversion by calculating the surface minimun distance 
*    with an equivalent second order nurbs. Then performs an iterative method.
*  Return Values:
*    integer
*  @return greater than zero is at least one solution is found.
*******************************************************************************/
#ifdef SWIG 
      %apply NurbsFloat* OUTPUT { NurbsFloat *pu, NurbsFloat *pv };
#endif
int nurbs_surface_inversion_min_distance
     ( NurbsFloat *pu, NurbsFloat *pv
     , const NurbsSurface *surface_orig 	/** non reduced NURBS */
     , const NurbsSurface *surface_order2 	/**second order NURBS */
     , const NurbsVector3 *point /** coordinates of the point {x, y, z} */
     , const NurbsFloat epsilon  /** stop condition (eg 1e-6) */
     , const NurbsFloat gamma    /** relaxing factor (eg 0, 0.25, 0.50) */
     );

/*******************************************************************************
*  Description:
*    Calculates the normal to the nurbs surface.
*    It is calculated as the cross of the derivatives. It may possible that one 
*    or both derivatives are zero, and the method fails
*  Return Values:
*    void
*******************************************************************************/
NurbsVector3 nurbs_surface_get_normal
    ( const NurbsSurface *nurbs /** nurbs surface pointer */
    , const NurbsFloat u        /** parametric coordinate (etha) */
    , const NurbsFloat v        /** parametric coordinate (chi) */
    );

/*******************************************************************************
*  Description:
*    Calculates the normal to the nurbs surface by finite diferences.
*  Return Values:
*    void
*******************************************************************************/
#ifndef SWIG 
NurbsVector3 nurbs_surface_get_normal_by_FD
    ( const NurbsSurface *nurbs
    , const NurbsFloat u
    , const NurbsFloat v
    );
#endif 

/*******************************************************************************
*  Description:
*    Reduces a nurbs to a second order. If nullptr is pass as first argument, 
*    creates a new nurbs
*  Return Values:
*    the pointer to the new nurbs
*******************************************************************************/
NurbsSurface* nurbs_surface_reduce_order2
    ( NurbsSurface* nurbs2          /* Use nullptr to create a new data structure */
    , const NurbsSurface* surface   /* Original NURBS */
    );

/**  
* A fast (and potentially low accurate) calculation of the intersection 
* between two nurbs. (EXPERIMENTAL; use at your own risk)
* Returns 1 if an intersection is detected or 0 if not
*/
#ifndef SWIG 
int nurbs_surface_intersection_fast
    ( NurbsIntersection* buffer
    , size_t* p_num_sol
    , const size_t max_sol
    , const NurbsSurface* nurbs_a
    , const NurbsSurface* nurbs_b
    , NurbsSurface* nurbs_2order_a
    , NurbsSurface* nurbs_2order_b
    , const NurbsFloat relaxation
    );
#endif

/**  Refines the intersection with an iterative method */
#ifndef SWIG 
void nurbs_surface_intersection_iterative
    ( NurbsFloat* u, NurbsFloat* v, NurbsFloat* m, NurbsFloat* n
    , const NurbsSurface* nurbs_a
    , const NurbsSurface* nurbs_b
    , const NurbsFloat epsilon
    );
#endif

#ifdef  __cplusplus
  }
#endif

#endif /* _DOMINO_NURBS_H */


/**/