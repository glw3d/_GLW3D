/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Imports TAU grids in NETCDF file format
*******************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "netcdf.h"

#include "common/definitions.h"
#include "common/check_malloc.h"
#include "common/log.h"

#include "tau_tools.h"


/* Data structure to store a NETCDF var and att names */
struct CNetCdfDataSet : ncDataSet
{
    CNetCdfDataSet()
    {
        num_vars = 0;
        num_atts = 0;
        var_names = nullptr;
        att_names = nullptr;
    }

    ~CNetCdfDataSet()
    {
        if (var_names != nullptr){
            free(var_names[0]);
        }
        free(var_names);

        if (att_names != nullptr){
            free(att_names[0]);
        }
        free(att_names);
    }
};

/* Extracts the vertices of the primary grid */
static owVector3d* extract_vertices(const int ncid, int* len)
{
    double* buffer_x_d = nullptr;
    double* buffer_y_d = nullptr;
    double* buffer_z_d = nullptr;
    float* buffer_x_f = nullptr;
    float* buffer_y_f = nullptr;
    float* buffer_z_f = nullptr;
    size_t len_x = 0, len_y = 0, len_z = 0;

    int status;
    int varid = 0;
    int dimids[8] = { 0 };
    nc_type typevar;

    *len = 0;

    status = nc_inq_varid(ncid, "points_xc", &varid);
    if (status == NC_NOERR){
        nc_inq_vardimid(ncid, varid, dimids);
        nc_inq_dimlen(ncid, dimids[0], &len_x);
        nc_inq_vartype(ncid, varid, &typevar);
        if (len_x > 0){
            if (typevar == NC_FLOAT){
                _check_( buffer_x_f = (float*)_malloc_( sizeof( float )*len_x ) );
                if (buffer_x_f != nullptr){
                    nc_get_var_float(ncid, varid, buffer_x_f);
                }
            }
            else if (typevar == NC_DOUBLE){
                _check_( buffer_x_d = (double*)_malloc_( sizeof( double )*len_x ) );
                if (buffer_x_d != nullptr){
                    nc_get_var_double(ncid, varid, buffer_x_d);
                }
            }
        }
    }

    status = nc_inq_varid(ncid, "points_yc", &varid);
    if (status == NC_NOERR){
        nc_inq_vardimid(ncid, varid, dimids);
        nc_inq_dimlen(ncid, dimids[0], &len_y);
        nc_inq_vartype(ncid, varid, &typevar);
        if (len_y > 0){
            if (typevar == NC_FLOAT){
                _check_( buffer_y_f = (float*)_malloc_( sizeof( float )*len_y ) );
                if (buffer_y_f != nullptr){
                    nc_get_var_float(ncid, varid, buffer_y_f);
                }
            }
            else if (typevar == NC_DOUBLE){
                _check_( buffer_y_d = (double*)_malloc_( sizeof( double )*len_y ) );
                if (buffer_y_d != nullptr){
                    nc_get_var_double(ncid, varid, buffer_y_d);
                }
            }
        }
    }

    status = nc_inq_varid(ncid, "points_zc", &varid);
    if (status == NC_NOERR){
        nc_inq_vardimid(ncid, varid, dimids);
        nc_inq_dimlen(ncid, dimids[0], &len_z);
        nc_inq_vartype(ncid, varid, &typevar);
        if (len_z > 0){
            if (typevar == NC_FLOAT){
                _check_(buffer_z_f = (float*)_malloc_(sizeof(float)*len_z));
                if (buffer_z_f != nullptr){
                    nc_get_var_float(ncid, varid, buffer_z_f);
                }
            }
            else if (typevar == NC_DOUBLE){
                _check_(buffer_z_d = (double*)_malloc_(sizeof(double)*len_z));
                if (buffer_z_d != nullptr){
                    nc_get_var_double(ncid, varid, buffer_z_d);
                }
            }
        }
    }

    size_t num_vert = 0;
    if (len_x > num_vert){
        num_vert = len_x;
    }
    if (len_y > num_vert){
        num_vert = len_y;
    }
    if (len_z > num_vert){
        num_vert = len_z;
    }
    
    owVector3d* point_stream;
    _check_(point_stream 
        = (owVector3d*)_calloc_(num_vert, sizeof(owVector3d)));

    if (point_stream != nullptr){
        if (buffer_x_d != nullptr){
            for (size_t i = 0; i < len_x; i++){
                point_stream[i].x = buffer_x_d[i];
            }
        }
        else if (buffer_x_f != nullptr){
            for (size_t i = 0; i < len_x; i++){
                point_stream[i].x = (double)buffer_x_f[i];
            }
        }

        if (buffer_y_d != nullptr){
            for (size_t i = 0; i < len_y; i++){
                point_stream[i].y = buffer_y_d[i];
            }
        }
        else if (buffer_y_f != nullptr){
            for (size_t i = 0; i < len_y; i++){
                point_stream[i].y = (double)buffer_y_f[i];
            }
        }

        if (buffer_z_d != nullptr){
            for (size_t i = 0; i < len_z; i++){
                point_stream[i].z = buffer_z_d[i];
            }
        }
        else if (buffer_z_f != nullptr){
            for (size_t i = 0; i < len_z; i++){
                point_stream[i].z = (double)buffer_z_f[i];
            }
        }
    }

    free(buffer_x_d);
    free(buffer_x_f);
    free(buffer_y_d);
    free(buffer_y_f);
    free(buffer_z_d);
    free(buffer_z_f);

    if (point_stream != nullptr){
        *len = (int)num_vert;
    }

    return point_stream;
}

/* Get the total size of a variable by multiplying its dimension values */
static size_t get_var_length( const int ncid, const int varid )
{
    size_t total_len = 0;
    int num_dim = 0;
    int dimids[8] = { 0 };
    size_t dim_len = 0;

    nc_inq_varndims( ncid, varid, &num_dim );
    nc_inq_vardimid( ncid, varid, dimids );
    nc_inq_dimlen( ncid, dimids[0], &dim_len );
    total_len = dim_len;
    for (int i = 1; i < num_dim; i++){
        nc_inq_dimlen( ncid, dimids[i], &dim_len );
        total_len *= dim_len;
    }

    return total_len;
}

/* Extracts a single variable stream data */
static double* extract_double(const int ncid, const char* var_name, int* len)
{
    double* stream = nullptr;
    int varid = -1;
    size_t total_len = 0;
    nc_type typevar;

    const char* nc_type_names[] =
        { "NC_NAT"	    /* Not A Type */
        , "NC_BYTE"     /* signed 1 byte integer */
        , "NC_CHAR"	    /* ISO/ASCII character */
        , "NC_SHORT"	/* signed 2 byte integer */
        , "NC_INT"      /* signed 4 byte integer */
        , "NC_LONG"     /* deprecated, but required for backward "ibility. */
        , "NC_INT"      /* deprecated, but required for backward "ibility. */
        , "NC_FLOAT"	/* single precision floating point number */
        , "NC_DOUBLE"	/* double precision floating point number */
        , "NC_UBYTE"	/* unsigned 1 byte int */
        , "NC_USHORT"	/* unsigned 2-byte int */
        , "NC_UINT"	    /* unsigned 4-byte int */
        , "NC_INT64"	/* signed 8-byte int */
        , "NC_UINT64"	/* unsigned 8-byte int */
        , "NC_STRING"	/* string */
        };

    *len = 0;

    int status = nc_inq_varid(ncid, var_name, &varid);
    if (status != NC_NOERR){
        _handle_error_("Var %s is not in the file!", var_name);
        return nullptr;
    }
    if (varid >= 0){
        nc_inq_vartype( ncid, varid, &typevar );

        total_len = get_var_length( ncid, varid );;
        if (total_len == 0){
            _handle_error_( "The stream is of size 0!" );
            return nullptr;
        }

        if (typevar == NC_FLOAT){
            float* buffer_f = nullptr;
            _check_( buffer_f = (float*)_malloc_( sizeof( float )*total_len ) );
            if (buffer_f != nullptr){
                nc_get_var_float( ncid, varid, buffer_f );
            }

            _check_( stream = (double*)_malloc_( sizeof( double )*total_len ) );
            if (stream != nullptr){
                for (size_t i = 0; i < total_len; i++){
                    stream[i] = (double)buffer_f[i];
                }
            }

            free( buffer_f );
        }
        else if (typevar == NC_INT){
            int32_t* buffer = nullptr;
            _check_( buffer = (int*)_malloc_( sizeof( int )*total_len ) );
            if (buffer != nullptr){
                nc_get_var_int( ncid, varid, buffer );
            }

            _check_( stream = (double*)_malloc_( sizeof( double )*total_len ) );
            if (stream != nullptr){
                for (size_t i = 0; i < total_len; i++){
                    stream[i] = (double)buffer[i];
                }
            }

            free( buffer );
        }
        else if (typevar == NC_DOUBLE){
            _check_( stream = (double*)_malloc_( sizeof( double )*total_len ) );
            if (stream != nullptr){
                nc_get_var_double(ncid, varid, stream);
            }
        }
        else{
            _warning_( "Var %s is of a type %s!", nc_type_names[typevar] );
        }
    }

    if (stream != nullptr){
        *len = (int)total_len;
    }

    return stream;
}

/* Extracts a single variable stream data. It identifies the type of variable
 * in the dataset and convert to integer. */
static int* extract_int(const int ncid, const char* var_name, int* len)
{
    int* stream = nullptr;
    int varid = -1;
    size_t total_len = 0;
    nc_type typevar;

    *len = 0;

    int status = nc_inq_varid(ncid, var_name, &varid);
    if (varid >= 0){
        nc_inq_vartype( ncid, varid, &typevar );
        if (typevar == NC_CHAR){
            _handle_error_( "The variable is of type char" );
            return nullptr;
        }
        if (typevar == NC_SHORT){
            _handle_error_( "The variable is of type short" );
            return nullptr;
        }
        if (typevar == NC_FLOAT){
            _handle_error_( "The variable is of type float" );
            return nullptr;
        }
        if (typevar == NC_DOUBLE){
            _handle_error_( "The variable is of type double" );
            return nullptr;
        }

        total_len = get_var_length( ncid, varid );
        if (total_len == 0){
            _handle_error_( "The stream is of size 0" );
            return nullptr;
        }

        if (total_len > 0 && typevar == NC_INT){
            _check_(stream = (int*)_malloc_(sizeof(int)*total_len));
            if (stream != nullptr){
                nc_get_var_int( ncid, varid, stream );
            }
        }
    }

    if (stream != nullptr){
        *len = (int)total_len;
    }

    return stream;
}

/* Releases the memory */
extern "C"
void ncDataSet_free( ncDataSet* dataset )
{
    ncDataSet* object = (CNetCdfDataSet*)dataset;
    delete(dataset);
}

static void extract_gridname( char* label, const char* filename )
{
    const char* p = filename;
    int len = 0;
    while (*p != '\0' && len < 1024){
        label[len] = *p;
        p++;
        len++;
    }
    label[len] = '\0';
}

/* Imports a TAU grid*/
extern "C"
owTauGrid* owTauGrid_import(const char* filename)
{
    int ncid;       /* Id of the dataset */
    int num_surface_elements;
    int* marker_stream;
    char* time_str = getlocaltime();

    _trace_( "\n--- %s", time_str );
    _trace_( "Loading grid %s\n", filename );

    if (nc_open(filename, 0, &ncid) != NC_NOERR){
        _handle_error_("Cannot open filename %s", filename);
        return nullptr;
    }

    owTauGrid* taugrid = owTauGrid_create();
    extract_gridname( taugrid->label, filename );

    int num_points = 0;
    taugrid->points.stream = extract_vertices( ncid, &num_points );
    taugrid->points.length = num_points;
    _trace_( "   num points: %i\n", taugrid->points.length );

    int len;
    taugrid->surface_tri3.stream = extract_int
        ( ncid, "points_of_surfacetriangles", &len );
    taugrid->surface_tri3.length = (size_t)len;
    _trace_( "   points_of_surfacetriangles: %i\n", len );

    taugrid->surface_quad4.stream = extract_int
        ( ncid, "points_of_surfacequadrilaterals", &len );
    taugrid->surface_quad4.length = (size_t)len;
    _trace_( "   points_of_surfacequadrilaterals: %i\n", len );

    taugrid->tetrahedrons4.stream = extract_int
        ( ncid, "points_of_tetraeders", &len );
    taugrid->tetrahedrons4.length = (size_t)len;
    _trace_( "   points_of_tetraeders: %i\n", len );

    taugrid->pyramids5.stream = extract_int
        ( ncid, "points_of_pyramids", &len );
    taugrid->pyramids5.length = (size_t)len;
    _trace_( "   points_of_pyramids: %i\n", len );

    taugrid->prisms6.stream = extract_int
        ( ncid, "points_of_prisms", &len );
    taugrid->prisms6.length = (size_t)len;
    _trace_( "   points_of_prisms: %i\n", len );

    taugrid->hexaheders8.stream = extract_int
        ( ncid, "points_of_hexaeders", &len );
    taugrid->hexaheders8.length = (size_t)len;
    _trace_( "   points_of_hexaeders: %i\n", len );

    marker_stream = extract_int
        ( ncid, "boundarymarker_of_surfaces", &(num_surface_elements) );

    /* The boundary markers are stored in a single grid. 
     * First are the markers for the triangles, and then the quadrilaterala */
    int tri_marker_len = 0;
    if (taugrid->surface_tri3.length > 0){
        taugrid->marker_triangles.stream = marker_stream;
        tri_marker_len = int( taugrid->surface_tri3.length / 3 );
        taugrid->marker_triangles.length = tri_marker_len;
    }
    if (taugrid->surface_quad4.length > 0){
        taugrid->marker_quads.stream = &(marker_stream[tri_marker_len]);
        taugrid->marker_quads.length = num_surface_elements - tri_marker_len;
    }

    nc_close(ncid);

    return taugrid;
}

/* Imports the surface TAU grid*/
extern "C"
owTauGrid* owTauSurfGrid_import( const char* filename )
{
    int ncid;       /* Id of the dataset */
    int num_surface_elements;
    int* marker_stream;
    char* time_str = getlocaltime();

    _trace_( "\n---Import TAU grid--- %s\n", time_str );

    if (nc_open( filename, 0, &ncid ) != NC_NOERR){
        _handle_error_( "Cannot open filename %s", filename );
        return nullptr;
    }

    owTauGrid* taugrid = owTauGrid_create();
    extract_gridname( taugrid->label, filename );

    int num_points = 0;
    taugrid->points.stream = extract_vertices( ncid, &num_points );
    taugrid->points.length = num_points;
    _trace_( "num points: %i\n", taugrid->points.length );

    int len;
    taugrid->surface_tri3.stream = extract_int
        ( ncid, "points_of_surfacetriangles", &len );
    taugrid->surface_tri3.length = (size_t)len;
    _trace_( "points_of_surfacetriangles: %i\n", len );

    taugrid->surface_quad4.stream = extract_int
        ( ncid, "points_of_surfacequadrilaterals", &len );
    taugrid->surface_quad4.length = (size_t)len;
    _trace_( "points_of_surfacequadrilaterals: %i\n", len );

    marker_stream = extract_int
        ( ncid, "boundarymarker_of_surfaces", &(num_surface_elements) );

    /* The boundary markers are stored in a single stream.
    * First are the markers for the triangles, and then the quadrilaterala */
    if (taugrid->surface_tri3.length > 0){
        taugrid->marker_triangles.stream = marker_stream;
        taugrid->marker_triangles.length = size_t(taugrid->surface_tri3.length/3);
    }
    if (taugrid->surface_quad4.length > 0){
        taugrid->marker_quads.stream = &(marker_stream[taugrid->marker_triangles.length]);
        taugrid->marker_quads.length = size_t(taugrid->surface_quad4.length / 4);
    }

    nc_close( ncid );

    return taugrid;
}

/* Checks if the marker is in the stream */
static int check_marker( const owIntStream* markers, const int marker )
{
    for (size_t i = 0; i < markers->length; i++){
        if (markers->stream[i] == marker){
            return 1;
        }
    }

    return 0;
}

static inline owVector3d vector_cross
( const owVector3d a, const owVector3d b )
{
    owVector3d c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;

    return c;
}

static inline owVector3d calculate_normal
( const owVector3d a, const owVector3d b, const owVector3d c )
{
    owVector3d c1 = { c.x - a.x, c.y - a.y, c.z - a.z };
    owVector3d c2 = { b.x - a.x, b.y - a.y, b.z - a.z };
    owVector3d n = vector_cross( c1, c2 );

    return n;
}

/* Exports a TAU grid into a stl file */
extern "C"
int owTauGrid_export_stl
( const owTauGrid* grid, const char* filename, const owIntStream* markers)
{
    if (grid == nullptr){
        return 1;
    }

    FILE* fh = fopen( filename, "wt" );

    if (fh == nullptr){
        _handle_error_( "Cannot create file %s", filename );
        return 1;
    }

    /* Create the header */
    fprintf( fh, "solid %s\n", grid->label );

    /* Loop through the elements */
    for (size_t i = 0; i < grid->marker_triangles.length; i++){

        /* Work with the triangles */
        if (check_marker( markers, grid->marker_triangles.stream[i] ) != 0){

            /* Extract the vertices */
            int j = i * 3;
            int i1 = grid->surface_tri3.stream[j];
            int i2 = grid->surface_tri3.stream[j + 1];
            int i3 = grid->surface_tri3.stream[j + 2];

            owVector3d v1 = grid->points.stream[i1];
            owVector3d v2 = grid->points.stream[i2];
            owVector3d v3 = grid->points.stream[i3];

            /* Calculate surface normal */
            owVector3d n = calculate_normal( v1, v2, v3 );

            /* Print the vertex coordinates */
            fprintf( fh, "facet normal %g\t%g\t%g\n", n.x, n.y, n.z );
            fprintf( fh, "\touter loop\n" );
            fprintf( fh, "\t\tvertex %g\t%g\t%g\n", v1.x, v1.y, v1.z );
            fprintf( fh, "\t\tvertex %g\t%g\t%g\n", v2.x, v2.y, v2.z );
            fprintf( fh, "\t\tvertex %g\t%g\t%g\n", v3.x, v3.y, v3.z );
            fprintf( fh, "\tendloop\n" );
            fprintf( fh, "endfacet\n" );
        }
    }
        /* Work with the quadrilaterals */
    for (size_t i = 0; i < grid->marker_quads.length; i++){
        if (check_marker( markers, grid->marker_quads.stream[i] ) != 0){
            /* Extract the vertices */
            int j = i * 4;
            int i1 = grid->surface_quad4.stream[j];
            int i2 = grid->surface_quad4.stream[j + 1];
            int i3 = grid->surface_quad4.stream[j + 2];
            int i4 = grid->surface_quad4.stream[j + 3];

            owVector3d v1 = grid->points.stream[i1];
            owVector3d v2 = grid->points.stream[i2];
            owVector3d v3 = grid->points.stream[i3];
            owVector3d v4 = grid->points.stream[i4];

            /* Convert the quadrilateral into two triangles */
            owVector3d n1 = calculate_normal( v1, v2, v3 );
            fprintf( fh, "facet normal %g\t%g\t%g\n", n1.x, n1.y, n1.z );
            fprintf( fh, "\touter loop\n" );
            fprintf( fh, "\t\tvertex %g\t%g\t%g\n", v1.x, v1.y, v1.z );
            fprintf( fh, "\t\tvertex %g\t%g\t%g\n", v2.x, v2.y, v2.z );
            fprintf( fh, "\t\tvertex %g\t%g\t%g\n", v3.x, v3.y, v3.z );
            fprintf( fh, "\tendloop\n" );
            fprintf( fh, "endfacet\n" );

            owVector3d n2 = calculate_normal( v3, v4, v1 );
            fprintf( fh, "facet normal %g\t%g\t%g\n", n2.x, n2.y, n2.z );
            fprintf( fh, "\touter loop\n" );
            fprintf( fh, "\t\tvertex %g\t%g\t%g\n", v3.x, v3.y, v3.z );
            fprintf( fh, "\t\tvertex %g\t%g\t%g\n", v4.x, v4.y, v4.z );
            fprintf( fh, "\t\tvertex %g\t%g\t%g\n", v1.x, v1.y, v1.z );
            fprintf( fh, "\tendloop\n" );
            fprintf( fh, "endfacet\n" );
        }
    }

    fprintf( fh, "endsolid %s\n", grid->label );
    fclose( fh );

    return 0;
}

/* Imports a TAU solution */
extern "C"
owDoubleStream* nc_import_dbl( const char* filename, const char* var_name )
{
    owDoubleStream* out = new owDoubleStream;
    double* sol_stream = nullptr;
    int ncid;
    int ilen;

    out->stream = nullptr;
    out->length = 0;

    if (nc_open(filename, 0, &ncid) != NC_NOERR){
        _handle_error_("Cannot open filename %s", filename);
        return out;
    }

    sol_stream = extract_double(ncid, var_name, &ilen);
    if (sol_stream == nullptr){
        _handle_error_("Failed to extract var %s from file %s", var_name, filename);
    }
    else{
        out->stream = sol_stream;
        out->length = ilen;
    }

    nc_close(ncid);

    return out;
}

/* Imports a TAU solution */
extern "C"
owIntStream* nc_import_int( const char* filename, const char* var_name )
{
    owIntStream* out = new owIntStream;
    int ncid;
    int ilen;

    out->stream = nullptr;
    out->length = 0;

    if (nc_open( filename, 0, &ncid ) != NC_NOERR){
        _handle_error_( "Cannot open filename %s", filename );
        return out;
    }

    out->stream = extract_int( ncid, var_name, &ilen );
    if (out->stream == nullptr){
        _handle_error_( "Failed to extract var %s from file %s", var_name, filename );
    }
    else{
        out->length = ilen;
    }

    nc_close( ncid );

    return out;
}

/* Gets an attribute from a TAU solution. Returns 0 if success. */
extern "C"
double nc_getatt( const char* filename, const char* att_name )
{
    int ncid, attid = 0;
    nc_type type;
    double out = ADP_FILL_INT;

    if (nc_open(filename, 0, &ncid) != NC_NOERR){
        _handle_error_("Cannot open filename %s", filename);
        return out;
    }

    nc_inq_attid(ncid, NC_GLOBAL, att_name, &attid);
    if (attid == 0){
        _handle_error_("Cannot read attribute %s in file %s"
            , att_name, filename);
        return out;
    }

    nc_inq_atttype(ncid, NC_GLOBAL, att_name, &type);

    if (type == NC_DOUBLE){
        nc_get_att_double(ncid, NC_GLOBAL, att_name, &out);
    }
    else if (type == NC_FLOAT){
        float value;
        nc_get_att_float(ncid, NC_GLOBAL, att_name, &value);
        out = (double)value;
    }
    else if (type == NC_INT){
        int value;
        nc_get_att_int(ncid, NC_GLOBAL, att_name, &value);
        out = (double)value;
    }
    else{
        _handle_error_("Attribute %s type is not supported in file %s"
            , att_name, filename);
        nc_close( ncid );
        return out;
    }

    nc_close(ncid);

    return out;
}


/* Inquiries the variables in the netcdf file */
extern "C"
ncDataSet* nc_query( const char* filename )
{
    char* var_names_stream = nullptr;
    char* att_names_stream = nullptr;
    int ncid = 0;
    int ndim = 0;   /* Number of dimensions */
    int nvar = 0;   /* Number of variables */
    int natt = 0;   /* Number of attributes */
    int unlimdim = 0;  /* Id of the unlimmited dimension (not really used) */

    int status = nc_open( filename, NC_NOWRITE, &ncid );
    if (status != NC_NOERR){
        _handle_error_( "Cannot open filename %s", filename );
        return nullptr;
    }

    CNetCdfDataSet* out = new CNetCdfDataSet();

    nc_inq( ncid, &ndim, &nvar, &natt, &unlimdim );

    if (nvar > 0){
        _check_( var_names_stream
            = (char*)_calloc_( sizeof( char ), NC_MAX_NAME * nvar ) );

        _check_( out->var_names = (char**)_malloc_( sizeof( char* ) * nvar ) );
        out->num_vars = nvar;

        if (var_names_stream != nullptr && out->var_names != nullptr){
            for (int i = 0; i < nvar; i++){
                out->var_names[i] = var_names_stream + i * NC_MAX_NAME;
                nc_inq_varname( ncid, i, out->var_names[i] );
            }
        }
    }

    if (natt > 0){
        _check_( att_names_stream
            = (char*)_calloc_( sizeof( char ), NC_MAX_NAME * natt ) );

        _check_( out->att_names = (char**)_malloc_( sizeof( char* ) * natt ) );
        out->num_atts = natt;

        if (att_names_stream != nullptr && out->att_names != nullptr){
            for (int i = 0; i < natt; i++){
                out->att_names[i] = att_names_stream + i * NC_MAX_NAME;
                nc_inq_attname( ncid, NC_GLOBAL, i, out->att_names[i] );
            }
        }
    }

    nc_close( ncid );

    return out;
}

/* Checks if a stream variable is in the file */
extern "C"
int nc_inquiry_var( const char* filename, const char* var_name )
{
    int ncid = 0;
    int varid = 0;
    int status = nc_open( filename, NC_NOWRITE, &ncid );
    if (status != NC_NOERR){
        _handle_error_( "Cannot open filename %s", filename );
        return 0;
    }

    CNetCdfDataSet* out = new CNetCdfDataSet();

    status = nc_inq_varid( ncid, var_name, &varid );
    if (status != NC_NOERR){
        return 0;
    }

    nc_close( ncid );

    return 1;
}

