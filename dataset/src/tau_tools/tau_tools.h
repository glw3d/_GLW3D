/***
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#ifndef TAU_TOOLS_H
#define TAU_TOOLS_H

#include "stdlib.h"

#include "dataset/dataset_arrays.h"
#include "owTauGrid.h"

#define ADP_FILL_INT (-2147483647L)

typedef struct
{
    /** Number of variables in the NETCDF file */
    int num_vars;

    /** Number of attributes in the NETCDF file */
    int num_atts;

    /** Variable names in the NETCDF file */
    char** var_names;

    /** Attribute names in the NETCDF file */
    char** att_names;

}ncDataSet;

#ifdef  __cplusplus
extern "C" {
#endif

    /* Imports a tau grid in NETCDF file format */
    owTauGrid* owTauGrid_import( const char* filename );

    /* Imports the surface TAU grid*/
    owTauGrid* owTauSurfGrid_import( const char* filename );

    /* Reads the attribute in the netcdf file. Returns 0 if success. */
    double nc_getatt
    ( const char* filename /* TAU solution filename */
    , const char* att_name /* Attribute name */
    );

    /* Inquiries the variables and attributes in a netcdf file */
    ncDataSet* nc_query( const char* filename );

    /* Checks if a stream variable is in the file */
    int nc_inquiry_var( const char* filename, const char* var_name );

    /* Releases memory */
    void ncDataSet_free( ncDataSet* dataset );

    /* Imports a TAU solution */
    owDoubleStream* nc_import_dbl
    ( const char* filename      /* TAU solution filename */
    , const char* var_name      /* Variable name */
    );

    /* Imports an integer value in a TAU solution */
    owIntStream* nc_import_int
    ( const char* filename  /* TAU solution filename */
    , const char* var_name /* Variable name */
    );

    /* Exports a single stream data into a netcdf data file. */
    int nc_export_dbl
    ( const char* filename
    , const char* var_name
    , const double* stream
    , const size_t length
    );

    /* Calculates the surface normals */
    owVector3dStream* owTauGrid_surf_normals
    ( const owTauGrid* grid      /* Tau's primary grid */
    );

    /* Exports the surface for the TAU deformation */
    int taudeform_export
        ( const char* filename, const owTauGrid* grid, const int* wall_markers );

    /* Deformates the a 2d grid using a Laplacian deformation.
     * The wall markers includes the farfield and ust be zero terminated; e.g. {3, 6, 0} */
    double tau2d_deform_laplace_position
        ( owTauGrid* def_grid
        , const owTauGrid* orig_grid
        , const int* wall_markers
        , const int num_iterations
        , const double epsilon
        );

    /* Deformates the a 2d grid using a Laplacian deformation
     * and orthogonality correction.
     * The wall markers includes the farfield and ust be zero terminated; e.g. {3, 6, 0} */
    double tau2d_deform_laplace_ortho
        ( owTauGrid* def_grid
        , const owTauGrid* orig_grid
        , const int* wall_markers
        , const int num_iterations
        , const double epsilon
        , const double beta
        );

    /* Exports the surface grid into a stl file */
    int owTauGrid_export_stl
        ( const owTauGrid* grid, const char* filename, const owIntStream* markers );

#ifdef  __cplusplus
}
#endif

#endif /* TAU_TOOLS_H */
