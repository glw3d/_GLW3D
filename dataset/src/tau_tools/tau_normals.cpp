/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Calculates the surface normals
*******************************************************************************/

#include "stdlib.h"

#include "common/definitions.h"
#include "common/log.h"
#include "common/check_malloc.h"

#include "dataset/dataset_arrays.h"
#include "tau_tools/owTauGrid.h"

static inline owVector3d vector_cross
( const owVector3d a, const owVector3d b )
{
    owVector3d c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;

    return c;
}

static inline owVector3d vector_sub
( const owVector3d a, const owVector3d b )
{
    owVector3d c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;

    return c;
}

/* Calculates the max vertex index in the walls */
static size_t get_bc_stream_len( const owTauGrid* grid )
{
    int len = 0;

    for (size_t i = 0; i < grid->surface_tri3.length; i++){
        if (grid->surface_tri3.stream[i] > len){
            len = grid->surface_tri3.stream[i];
        }
    }

    for (size_t i = 0; i < grid->surface_quad4.length; i++){
        if (grid->surface_quad4.stream[i] > len){
            len = grid->surface_quad4.stream[i];
        }
    }

    return len + 1;
}

/* Calculates the surface normals */
extern "C"
owVector3dStream owTauGrid_surf_normals
( const owTauGrid* grid      /* Tau's primary grid */
){
    owVector3dStream normals_array;
    normals_array.stream = nullptr;
    normals_array.length = 0;

    if (grid == nullptr){
        _warning_( "NULL pointer!" );
        return normals_array;
    }

    size_t len = get_bc_stream_len( grid );

    _check_( normals_array.stream = (owVector3d*)_calloc_
        ( len, sizeof( owVector3d ) ) );
    normals_array.length = len;
    owVector3d* normals = normals_array.stream;

    if (normals == nullptr){
        /* Out of memory */
        return normals_array;
    }

    for (size_t i = 0; i < grid->surface_tri3.length;){
        int i0 = grid->surface_tri3.stream[i]; i++;
        int i1 = grid->surface_tri3.stream[i]; i++;
        int i2 = grid->surface_tri3.stream[i]; i++;

        owVector3d p0 = grid->points.stream[i0];
        owVector3d p1 = grid->points.stream[i1];
        owVector3d p2 = grid->points.stream[i2];

        owVector3d p10 = vector_sub( p1, p0 );
        owVector3d p20 = vector_sub( p2, p0 );
        owVector3d n = vector_cross( p20, p10 );

        n.x /= 6;
        n.y /= 6;
        n.z /= 6;

        normals[i0].x += n.x;
        normals[i0].y += n.y;
        normals[i0].z += n.z;

        normals[i1].x += n.x;
        normals[i1].y += n.y;
        normals[i1].z += n.z;

        normals[i2].x += n.x;
        normals[i2].y += n.y;
        normals[i2].z += n.z;
    }

    for (size_t i = 0; i < grid->surface_quad4.length;){
        int i0 = grid->surface_quad4.stream[i]; i++;
        int i1 = grid->surface_quad4.stream[i]; i++;
        int i2 = grid->surface_quad4.stream[i]; i++;
        int i3 = grid->surface_quad4.stream[i]; i++;

        owVector3d p0 = grid->points.stream[i0];
        owVector3d p1 = grid->points.stream[i1];
        owVector3d p2 = grid->points.stream[i2];
        owVector3d p3 = grid->points.stream[i3];

        owVector3d p20 = vector_sub( p2, p0 );
        owVector3d p31 = vector_sub( p3, p1 );
        owVector3d n = vector_cross( p31, p20 );

        n.x /= 8;
        n.y /= 8;
        n.z /= 8;

        normals[i0].x += n.x;
        normals[i0].y += n.y;
        normals[i0].z += n.z;

        normals[i1].x += n.x;
        normals[i1].y += n.y;
        normals[i1].z += n.z;

        normals[i2].x += n.x;
        normals[i2].y += n.y;
        normals[i2].z += n.z;

        normals[i3].x += n.x;
        normals[i3].y += n.y;
        normals[i3].z += n.z;
    }

    return normals_array;
}
