/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Methods for importing NURBS curves and surfaces 

*******************************************************************************/

#ifndef _DOMINO_NURBS_IO_H
#define _DOMINO_NURBS_IO_H

#include <stdio.h>

#include "nurbs_curve_data.h"
#include "nurbs_surface_data.h"
#include "nurbs_controlbox_data.h"

#ifdef  __cplusplus
  extern "C" {
#endif


#ifndef SWIG 

  /** Reads the NURBS curves and surfaces from a ASCII IGES file */
void nurbs_import_iges
    ( const char* filename /**< IGES file name */
    , NurbsCurve** p_curve_array      /**< (out) array with the curves */
    , int* num_curves                 /**< Number of curves */
    , NurbsSurface** p_surface_array  /**< (out) array with the surfaces */
    , int* num_surfaces               /**< Number of surfaces */
);

/** Reads all NURBS entities from an ASCII file. */
void nurbs_import_ascii
    ( const char *filename
    , NurbsCurve** p_nurbs_curve
    , int* num_curves
    , NurbsSurface** p_nurbs_surface
    , int* num_surfaces
    , NurbsControlBox** p_nurbs_controlbox
    , int* num_controlbox
    );

#endif

/* Imports just the nurbs surfaces */
NurbsSurface* nurbs_surface_import_iges
    ( const char *filename  /** file name */
    , int* OUTPUT     /* number of NURBS read in the file */
    );

/** Read all NURBS curves from an ASCII file. */
NurbsCurve* nurbs_curve_import_ascii
    ( const char *filename
    , int* OUTPUT         /* Number of returned NURBS curves */
    );


/** Read all NURBS surfaces from an ASCII file. */
NurbsSurface* nurbs_surface_import_ascii
    ( const char *filename
    , int* OUTPUT         /* Number of returned NURBS surfaces */
    );


/** Read all NURBS control boxes from an ASCII file. */
NurbsControlBox* nurbs_controlbox_import_ascii
    ( const char *filename
    , int* OUTPUT       /* Number of returned Control Boxes */
    );


/* Writes all NURBS entities to an ASCII file */
void nurbs_export_ascii
    ( const char* filename
    , const NurbsCurve *curve_array
    , const int num_curves 
    , const NurbsSurface *surface_array
    , const int num_surfaces 
    , const NurbsControlBox *cb_array
    , const int num_cb 
    );


/* Writes NURBS curves to an ASCII file */
void nurbs_curve_export_ascii
    ( const char* filename
    , const NurbsCurve *curve_array
    , const int num_curves 
    );


/* Writes NURBS surfaces to an ASCII file */
void nurbs_surface_export_ascii
    ( const char* filename
    , const NurbsSurface *surface_array
    , const int num_surfaces 
    );


/** Writes a control box NURBS to an ASCII file */
void nurbs_controlbox_export_ascii
    ( const char* filename
    , const NurbsControlBox *cb_array
    , const int num_cb 
    );

#ifdef  __cplusplus
}
#endif

#endif /* _DOMINO_NURBS_IO_H */

/**/