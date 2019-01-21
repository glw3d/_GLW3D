/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Exports TAU solutions in NETCDF file format
*******************************************************************************/

#include <stdlib.h>
#include "netcdf.h"

#include "tau_tools.h"

#include "common/definitions.h"
#include "common/check_malloc.h"
#include "common/log.h"

extern "C"
int nc_export_dbl
( const char* filename
, const char* var_name
, const double* stream
, const size_t length
)
{
    int ncid = 0;
    int idp = 0;
    int varid = 0;

    /* Try to create the file */
    int status;
    status = nc_open( filename, NC_WRITE, &ncid );
    if (status != NC_NOERR){
        /* File does not exits */
        status = nc_create( filename, NC_CLOBBER, &ncid );
    }
    if (status != NC_NOERR){
        _handle_error_( "Cannot create NETCDF file %s", filename );
        return 1;
    }

    status = nc_redef( ncid );

    status = nc_inq_dimid( ncid, "no_of_points", &idp );
    if (status != 0){
        status = nc_def_dim( ncid, "no_of_points", length, &idp );
    }
    status = nc_inq_varid( ncid, var_name, &varid );
    if (status != 0){
        status = nc_def_var( ncid, var_name, NC_DOUBLE, 1, &idp, &varid );
    }
    status = nc_enddef( ncid );
    status = nc_inq_varid( ncid, var_name, &varid );
    status = nc_put_var_double( ncid, varid, stream );

    nc_close( ncid );

    return 0;
}

static int check_marker( const int* const markers, const int marker )
{
    const int* pm = markers;
    while (*pm != 0){
        if (*pm == marker){
            return 1;
        }
        pm++;
    }

    return 0;
}

extern "C"
/* Wallmarkers is a null terminated string markers; e.g. {3, 6, 0} */
int taudeform_export( const char* filename, const owTauGrid* grid, const int* wall_markers )
{
    int status = 0;
    int ncid;
    int varidp_x, varidp_y, varidp_z, varidp_gloabalid, dimidp_total_length;
    size_t i, j;
    int i0, i1, i2, i3;
    int* flag = nullptr;
    int* gid_stream = nullptr;
    double* stream = nullptr;
    size_t npoints = 0;
    int marker;
    
    _check_( flag = (int*)_calloc_( grid->points.length, sizeof( int ) ) );

    /* Identify the bounday vertices */
    j = 0;
    for (i = 0; i < grid->marker_triangles.length; i++){
        marker = grid->marker_triangles.stream[i];
        if (check_marker( wall_markers, marker ) != 0){
            i0 = grid->surface_tri3.stream[j]; j++;
            i1 = grid->surface_tri3.stream[j]; j++;
            i2 = grid->surface_tri3.stream[j]; j++;
            flag[i0] = 1;
            flag[i1] = 1;
            flag[i2] = 1;
        }
        else{
            j += 3;
        }
    }

    j = 0;
    for (i = 0; i < grid->marker_quads.length; i++){
        marker = grid->marker_quads.stream[i];
        if (check_marker( wall_markers, marker ) != 0){
            i0 = grid->surface_quad4.stream[j]; j++;
            i1 = grid->surface_quad4.stream[j]; j++;
            i2 = grid->surface_quad4.stream[j]; j++;
            i3 = grid->surface_quad4.stream[j]; j++;

            flag[i0] = 1;
            flag[i1] = 1;
            flag[i2] = 1;
            flag[i3] = 1;
        }
        else{
            j += 4;
        }
    }

    /* Count the size of the arrays */
    for (i = 0; i < grid->points.length; i++){
        if (flag[i] != 0){
            npoints++;
        }
    }

    status = nc_create( filename, 0, &ncid );

    if (status != NC_NOERR){
        _handle_error_( "Cannot create netcdf file! %s", filename );
        return status;
    }

    /* Create a new file. Any previous file is erased */
    status = nc_def_dim( ncid, "no_of_points", npoints, &dimidp_total_length );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: no_of_points  code:%i", status );
        return status;
    }

    status = nc_def_var( ncid, "global_id", NC_INT, 1, &dimidp_total_length, &varidp_gloabalid );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: global_id  code:%i", status );
        return status;
    }

    status = nc_def_var( ncid, "x", NC_DOUBLE, 1, &dimidp_total_length, &varidp_x );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: x  code:%i", status );
        return status;
    }
    status = nc_def_var( ncid, "y", NC_DOUBLE, 1, &dimidp_total_length, &varidp_y );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: y  code:%i", status );
        return status;
    }
    status = nc_def_var( ncid, "z", NC_DOUBLE, 1, &dimidp_total_length, &varidp_z );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: z  code:%i", status );
        return status;
    }
    status = nc_close( ncid );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: nc_close()  code:%i", status );
        return status;
    }

    /* Save the streams */
    status = nc_open( filename, NC_WRITE, &ncid );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: nc_open()  code:%i", status );
        return status;
    }

    _check_( stream = (double*)_malloc_( sizeof( double )*npoints ) );
    j = 0;
    for (i = 0; i < grid->points.length; i++){
        if (flag[i] != 0){
            stream[j] = grid->points.stream[i].x;
            j++;
        }
    }
    status = nc_put_var_double( ncid, varidp_x, stream );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: nc_put_var()  code:%i", status );
        return status;
    }

    j = 0;
    for (i = 0; i < grid->points.length; i++){
        if (flag[i] != 0){
            stream[j] = grid->points.stream[i].y;
            j++;
        }
    }
    status = nc_put_var_double( ncid, varidp_y, stream );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: nc_put_var()  code:%i", status );
        return status;
    }

    j = 0;
    for (i = 0; i < grid->points.length; i++){
        if (flag[i] != 0){
            stream[j] = grid->points.stream[i].z;
            j++;
        }
    }
    status = nc_put_var_double( ncid, varidp_z, stream );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: nc_put_var()  code:%i", status );
        return status;
    }

    free( stream );

    _check_( gid_stream = (int*)_malloc_( sizeof( int )*npoints ) );
    j = 0;
    for (i = 0; i < grid->points.length; i++){
        if (flag[i] != 0){
            gid_stream[j] = i;
            j++;
        }
    }
    status = nc_put_var_int( ncid, varidp_gloabalid, gid_stream );
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: nc_put_var()  code:%i", status );
        return status;
    }

    free( gid_stream );

    if (ncid >= 0){
        status = nc_close( ncid );
    }
    if (status != NC_NOERR){
        _handle_error_( "netcdf error: nc_close()  code:%i", status );
        return status;
    }

    _log_( "File generated: %s\n", filename );
    return 0;
}


extern "C"
int taugrid_export(const char* filename, const owTauGrid* grid)
{
    /* NOT IMPLEMENTED */
    _handle_error_( "NOT IMPLEMENTED!" );
    return 1;
}

