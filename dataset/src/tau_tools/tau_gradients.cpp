/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Calculates the gradients in a TAU grid.
*******************************************************************************/

#include <memory.h>
#include <stdlib.h>
#include <math.h>

#include "common/definitions.h"
#include "common/log.h"
#include "common/check_malloc.h"

#include "dataset/dataset_arrays.h"
#include "tau_tools/owTauGrid.h"

#include "tau_tools.h"

typedef struct
{
    double a[9];
}owMatrix3x3;

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

owMatrix3x3 owMatrix3x3_invert( const owMatrix3x3 matrix )
{
    double a[9];
    memcpy( a, matrix.a, sizeof( double ) * 9 );

    owMatrix3x3 inv;

    double det = a[0] * a[4] * a[8];
    det += a[3] * a[7] * a[2];
    det += a[1] * a[5] * a[6];
    det -= a[2] * a[4] * a[6];
    det -= a[1] * a[3] * a[8];
    det -= a[5] * a[7] * a[0];

    inv.a[0] = (a[4] * a[8] - a[5] * a[7]) / det;
    inv.a[1] = (a[2] * a[7] - a[1] * a[8]) / det;
    inv.a[2] = (a[1] * a[5] - a[2] * a[4]) / det;
    inv.a[3] = (a[5] * a[6] - a[3] * a[8]) / det;
    inv.a[4] = (a[0] * a[8] - a[2] * a[6]) / det;
    inv.a[5] = (a[2] * a[3] - a[0] * a[5]) / det;
    inv.a[6] = (a[3] * a[7] - a[4] * a[6]) / det;
    inv.a[7] = (a[1] * a[6] - a[0] * a[7]) / det;
    inv.a[8] = (a[0] * a[4] - a[1] * a[3]) / det;

    return inv;
}

static inline void swap( double* a, double* b )
{
    double s = *a;
    *a = *b;
    *b = s;
}

static inline owMatrix3x3 adpMatrix3x3_transpose( const owMatrix3x3 matrix )
{
    owMatrix3x3 t;
    
    t.a[0] = matrix.a[0];
    t.a[1] = matrix.a[3];
    t.a[2] = matrix.a[6];
    t.a[3] = matrix.a[1];
    t.a[4] = matrix.a[4];
    t.a[5] = matrix.a[7];
    t.a[6] = matrix.a[2];
    t.a[7] = matrix.a[5];
    t.a[8] = matrix.a[8];

    return t;
}

owMatrix3x3 adpMatrix3x3_mul( const owMatrix3x3 matrix1, const owMatrix3x3 matrix2 )
{
    double a[9];
    double b[9];
    memcpy( a, matrix1.a, sizeof( double ) * 9 );
    memcpy( b, matrix2.a, sizeof( double ) * 9 );

    owMatrix3x3 c;

    c.a[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
    c.a[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
    c.a[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];
    c.a[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
    c.a[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
    c.a[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];
    c.a[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
    c.a[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
    c.a[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];

    return c;
}

owVector3d adpMatrixVector_mul( const owMatrix3x3 matrix, const owVector3d vec3 )
{
    double a[9];
    memcpy( a, matrix.a, sizeof( double ) * 9 );

    owVector3d c;
    c.x = a[0] * vec3.x + a[1] * vec3.y + a[2] * vec3.z;
    c.y = a[3] * vec3.x + a[4] * vec3.y + a[5] * vec3.z;
    c.z = a[6] * vec3.x + a[7] * vec3.y + a[8] * vec3.z;

    return c;
}

/******************************************************************************/

static inline owVector3d wall_surface
( const owVector3d x0, const owVector3d x1, const owVector3d x2 )
{
    owVector3d ca = vector_sub( x1, x0 );
    owVector3d cb = vector_sub( x2, x0 );
    owVector3d normal = vector_cross( ca, cb );
    normal.x /= 3;
    normal.y /= 3;
    normal.z /= 3;

    return normal;
}

static inline owVector3d dual_surface
( const owVector3d br  // Barycenter
, const owVector3d ca  // Centroid of left face
, const owVector3d cb  // Centroid of right face
, const owVector3d x0  // Point coordinates
, const owVector3d x1  // Point coordinates
){
    /* Mid segement point */
    owVector3d m01;
    m01.x = (x0.x + x1.x) / 2;
    m01.y = (x0.y + x1.y) / 2;
    m01.z = (x0.z + x1.z) / 2;

    /* Calculate dual surface for segment 01 */
    owVector3d c1 = vector_sub( ca, br );
    owVector3d c2 = vector_sub( cb, br );
    owVector3d mb = vector_sub( m01, br );
    owVector3d n1 = vector_cross( c1, mb );
    owVector3d n2 = vector_cross( mb, c2 );
    owVector3d normal;
    normal.x = (n1.x + n2.x) / 2;
    normal.y = (n1.y + n2.y) / 2;
    normal.z = (n1.z + n2.z) / 2;

    return normal;
}

static void gradients_surf_triangle_gg
( const owTauGrid* grid
, const double* var
, owVector3d* grad
){
    for (size_t i = 0; i < grid->surface_tri3.length;){
        int i0 = grid->surface_tri3.stream[i]; i++;
        int i1 = grid->surface_tri3.stream[i]; i++;
        int i2 = grid->surface_tri3.stream[i]; i++;

        double v0 = var[i0];
        double v1 = var[i1];
        double v2 = var[i2];

        /* Calculate the normal */
        owVector3d p0 = grid->points.stream[i0];
        owVector3d p1 = grid->points.stream[i1];
        owVector3d p2 = grid->points.stream[i2];

        owVector3d p10 = vector_sub( p1, p0 );
        owVector3d p20 = vector_sub( p2, p0 );
        owVector3d n = vector_cross( p10, p20 );
        n.x /= 6;
        n.y /= 6;
        n.z /= 6;

        double f0 = (6 * v0 + v1 + v2) / 8;
        grad[i0].x += f0 * n.x;
        grad[i0].y += f0 * n.y;
        grad[i0].z += f0 * n.z;

        double f1 = (6 * v1 + v2 + v0) / 8;
        grad[i1].x += f1 * n.x;
        grad[i1].y += f1 * n.y;
        grad[i1].z += f1 * n.z;

        double f2 = (6 * v2 + v0 + v1) / 8;
        grad[i2].x += f2 * n.x;
        grad[i2].y += f2 * n.y;
        grad[i2].z += f2 * n.z;
    }
}

static void gradients_surf_quads_gg
( const owTauGrid* grid
, const double* var
, owVector3d* grad
){
    for (size_t i = 0; i < grid->surface_quad4.length;){
        int i0 = grid->surface_quad4.stream[i]; i++;
        int i1 = grid->surface_quad4.stream[i]; i++;
        int i2 = grid->surface_quad4.stream[i]; i++;
        int i3 = grid->surface_quad4.stream[i]; i++;

        double v0 = var[i0];
        double v1 = var[i1];
        double v2 = var[i2];
        double v3 = var[i3];

        /* Calculate the normal */
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

        double f0 = (6 * v0 + v1 + v3) / 8;
        grad[i0].x += f0 * n.x;
        grad[i0].y += f0 * n.y;
        grad[i0].z += f0 * n.z;

        double f1 = (6 * v1 + v0 + v2) / 8;
        grad[i1].x += f1 * n.x;
        grad[i1].y += f1 * n.y;
        grad[i1].z += f1 * n.z;

        double f2 = (6 * v2 + v1 + v3) / 8;
        grad[i2].x += f2 * n.x;
        grad[i2].y += f2 * n.y;
        grad[i2].z += f2 * n.z;

        double f3 = (6 * v3 + v0 + v2) / 8;
        grad[i3].x += f3 * n.x;
        grad[i3].y += f3 * n.y;
        grad[i3].z += f3 * n.z;
    }
}

static void gradients_tetra_gg
( const owTauGrid* grid
, const double* var
, double* cell_vol
, owVector3d* grad
){
    for (size_t i = 0; i < grid->tetrahedrons4.length;){
        int i0 = grid->tetrahedrons4.stream[i]; i++;
        int i1 = grid->tetrahedrons4.stream[i]; i++;
        int i2 = grid->tetrahedrons4.stream[i]; i++;
        int i3 = grid->tetrahedrons4.stream[i]; i++;

        owVector3d x0 = grid->points.stream[i0];
        owVector3d x1 = grid->points.stream[i1];
        owVector3d x2 = grid->points.stream[i2];
        owVector3d x3 = grid->points.stream[i3];

        double v0 = var[i0];
        double v1 = var[i1];
        double v2 = var[i2];
        double v3 = var[i3];

        /* Calculate dual cell volume */
        owVector3d x10 = vector_sub( x1, x0 );
        owVector3d x20 = vector_sub( x2, x0 );
        owVector3d x30 = vector_sub( x3, x0 );
        owVector3d d012 = vector_cross( x10, x20 );
        double vol = x30.x * d012.x + x30.y * d012.y + x30.z * d012.z;
        vol /= 24; 
        /* If the order of the vertices is correct, the volume must be positive */
        cell_vol[i0] += vol;
        cell_vol[i1] += vol;
        cell_vol[i2] += vol;
        cell_vol[i3] += vol;


        /* Barycenter of the tetrahedron */
        owVector3d br;
        br.x = (x0.x + x1.x + x2.x + x3.x) / 4;
        br.y = (x0.y + x1.y + x2.y + x3.y) / 4;
        br.z = (x0.z + x1.z + x2.z + x3.z) / 4;

        /* Centroids */
        owVector3d c012;
        c012.x = (x0.x + x1.x + x2.x) / 3;
        c012.y = (x0.y + x1.y + x2.y) / 3;
        c012.z = (x0.z + x1.z + x2.z) / 3;

        owVector3d c123;
        c123.x = (x1.x + x2.x + x3.x) / 3;
        c123.y = (x1.y + x2.y + x3.y) / 3;
        c123.z = (x1.z + x2.z + x3.z) / 3;

        owVector3d c013;
        c013.x = (x0.x + x1.x + x3.x) / 3;
        c013.y = (x0.y + x1.y + x3.y) / 3;
        c013.z = (x0.z + x1.z + x3.z) / 3;

        owVector3d c023;
        c023.x = (x0.x + x2.x + x3.x) / 3;
        c023.y = (x0.y + x2.y + x3.y) / 3;
        c023.z = (x0.z + x2.z + x3.z) / 3;

        /* Calculate fluxes through control surface */
        owVector3d g0 = { 0, 0, 0 };
        owVector3d g1 = { 0, 0, 0 };
        owVector3d g2 = { 0, 0, 0 };
        owVector3d g3 = { 0, 0, 0 };

        owVector3d n01 = dual_surface( br, c013, c012, x0, x1 );
        double v01 = (v0 + v1) / 2;
        g0.x += n01.x * v01;
        g0.y += n01.y * v01;
        g0.z += n01.z * v01;
        g1.x -= n01.x * v01;
        g1.y -= n01.y * v01;
        g1.z -= n01.z * v01;

        owVector3d n12 = dual_surface( br, c123, c012, x1, x2 );
        double v12 = (v1 + v2) / 2;
        g1.x += n12.x * v12;
        g1.y += n12.y * v12;
        g1.z += n12.z * v12;
        g2.x -= n12.x * v12;
        g2.y -= n12.y * v12;
        g2.z -= n12.z * v12;

        owVector3d n23 = dual_surface( br, c123, c023, x2, x3 );
        double v23 = (v2 + v3) / 2;
        g2.x += n23.x * v23;
        g2.y += n23.y * v23;
        g2.z += n23.z * v23;
        g3.x -= n23.x * v23;
        g3.y -= n23.y * v23;
        g3.z -= n23.z * v23;

        owVector3d n30 = dual_surface( br, c013, c023, x3, x0 );
        double v30 = (v3 + v0) / 2;
        g3.x += n30.x * v30;
        g3.y += n30.y * v30;
        g3.z += n30.z * v30;
        g0.x -= n30.x * v30;
        g0.y -= n30.y * v30;
        g0.z -= n30.z * v30;

        owVector3d n13 = dual_surface( br, c013, c123, x1, x3 );
        double v13 = (v1 + v3) / 2;
        g1.x += n13.x * v13;
        g1.y += n13.y * v13;
        g1.z += n13.z * v13;
        g3.x -= n13.x * v13;
        g3.y -= n13.y * v13;
        g3.z -= n13.z * v13;

        owVector3d n20 = dual_surface( br, c023, c012, x2, x0 );
        double v20 = (v2 + v0) / 2;
        g2.x += n20.x * v20;
        g2.y += n20.y * v20;
        g2.z += n20.z * v20;
        g0.x -= n20.x * v20;
        g0.y -= n20.y * v20;
        g0.z -= n20.z * v20;

        grad[i0].x += g0.x;
        grad[i0].y += g0.y;
        grad[i0].z += g0.z;

        grad[i1].x += g1.x;
        grad[i1].y += g1.y;
        grad[i1].z += g1.z;

        grad[i2].x += g2.x;
        grad[i2].y += g2.y;
        grad[i2].z += g2.z;

        grad[i3].x += g3.x;
        grad[i3].y += g3.y;
        grad[i3].z += g3.z;
    }
}

/* Calculates the gradients using Green Gauss */
owVector3dStream* adpTauGrid_gradients_gg
( const owTauGrid* grid      /* Tau's primary grid */
, const owDoubleStream* var      /* Stream of variables */
){
    owVector3dStream* grad_array = new owVector3dStream;
    grad_array->stream = nullptr;
    grad_array->length = 0;

    if (grid == nullptr){
        _warning_( "NULL pointer" );
        return grad_array;
    }

    if (grid->points.length <= 0){
        _warning_( "The grid is empty!" );
        return grad_array;
    }

    if (grid->points.length != var->length){
        _warning_( "The number of variables is not the number of grid points!" );
        return grad_array;
    }

    /* Allocate memory for the gradients */
    _check_( grad_array->stream
        = (owVector3d*)_calloc_( sizeof( owVector3d ), grid->points.length ) );
    grad_array->length = grid->points.length;
    owVector3d* grad = grad_array->stream;

    /* Dual cell volumen */
    double* cell_vol = nullptr;
    _check_( cell_vol = (double*)_calloc_
        ( sizeof( double ), grid->points.length ) );

    if (cell_vol == nullptr || grad == nullptr){
        /* Out of memory */
        free( grad_array->stream );
        grad_array->stream = nullptr;
        grad_array->length = 0;
        free( cell_vol );
        return grad_array;
    }

    gradients_surf_triangle_gg( grid, var->stream, grad );
    //gradients_surf_quads_gg( grid, var.stream, grad );
    gradients_tetra_gg( grid, var->stream, cell_vol, grad );
    /* OTHER ELEMENTS! */

    /* Divide by the cell volume */
    for (size_t i = 0; i < grid->points.length; i++){
        double vol = cell_vol[i];
        grad[i].x /= vol;
        grad[i].y /= vol;
        grad[i].z /= vol;
    }

    /* Release temporary buffers */
    free( cell_vol );

    return grad_array;
}

/******************************************************************************/

static owVector3d vertex_gradient
( const owVector3d x0
, const owVector3d x1
, const owVector3d x2
, const owVector3d x3
, const double a0
, const double a1
, const double a2
, const double a3
){
    owMatrix3x3 Q;
    owVector3d n10, n20, n30;
    n10.x = x1.x - x0.x;
    n10.y = x1.y - x0.y;
    n10.z = x1.z - x0.z;
    n20.x = x2.x - x0.x;
    n20.y = x2.y - x0.y;
    n20.z = x2.z - x0.z;
    n30.x = x3.x - x0.x;
    n30.y = x3.y - x0.y;
    n30.z = x3.z - x0.z;

    Q.a[0] = n10.x;
    Q.a[1] = n20.x;
    Q.a[2] = n30.x;
    Q.a[3] = n10.y;
    Q.a[4] = n20.y;
    Q.a[5] = n30.y;
    Q.a[6] = n10.z;
    Q.a[7] = n20.z;
    Q.a[8] = n30.z;

    owMatrix3x3 J = owMatrix3x3_invert( Q );

    owVector3d f;
    f.x = (a1 - a0);
    f.y = (a2 - a0);
    f.z = (a3 - a0);

    owVector3d g = adpMatrixVector_mul( J, f );

    //adpVector3d e1 = adpMatrixVector_mul( J, n10 ); /* Must be {1,0,0} */
    //adpVector3d e2 = adpMatrixVector_mul( J, n20 ); /* Must be {0,1,0} */
    //adpVector3d e3 = adpMatrixVector_mul( J, n30 ); /* Must be {0,0,1} */

    return g;
}

static void gradients_tetra_jacobian
( const owTauGrid* grid
, const double* var
, owVector3d* grad
){
    for (size_t i = 0, j = 0; i < grid->tetrahedrons4.length; j++){
        int i0 = grid->tetrahedrons4.stream[i]; i++;
        int i1 = grid->tetrahedrons4.stream[i]; i++;
        int i2 = grid->tetrahedrons4.stream[i]; i++;
        int i3 = grid->tetrahedrons4.stream[i]; i++;

        owVector3d x0 = grid->points.stream[i0];
        owVector3d x1 = grid->points.stream[i1];
        owVector3d x2 = grid->points.stream[i2];
        owVector3d x3 = grid->points.stream[i3];

        double a0 = var[i0];
        double a1 = var[i1];
        double a2 = var[i2];
        double a3 = var[i3];

        owVector3d g0 = vertex_gradient( x0, x1, x2, x3, a0, a1, a2, a3 );
        owVector3d g1 = vertex_gradient( x1, x2, x3, x0, a1, a2, a3, a0 );
        owVector3d g2 = vertex_gradient( x2, x3, x0, x1, a2, a3, a0, a1 );
        owVector3d g3 = vertex_gradient( x3, x0, x1, x2, a3, a0, a1, a2 );

        grad[j].x = (g0.x + g1.x + g2.x + g3.x) / 4;
        grad[j].y = (g0.y + g1.y + g2.y + g3.y) / 4;
        grad[j].z = (g0.z + g1.z + g2.z + g3.z) / 4;

        double x = (x0.x + x1.x + x2.x + x3.x) / 4;
        double y = (x0.y + x1.y + x2.y + x3.y) / 4;
        double z = (x0.z + x1.z + x2.z + x3.z) / 4;

        _debug_( "%f == %f   %f == %f    %f == %f\n"
            , grad[j].x, 1 - 2 * x
            , grad[j].y, 1 - 2 * y
            , grad[j].z, 1 - 2 * z
            );
    }
}

static void gradients_hexa_jacobian
( const owTauGrid* grid
, const double* var
, owVector3d* grad
){
    for (size_t i = 0, j = 0; i < grid->hexaheders8.length; j++){
        int i0 = grid->hexaheders8.stream[i]; i++;
        int i1 = grid->hexaheders8.stream[i]; i++;
        int i2 = grid->hexaheders8.stream[i]; i++;
        int i3 = grid->hexaheders8.stream[i]; i++;
        int i4 = grid->hexaheders8.stream[i]; i++;
        int i5 = grid->hexaheders8.stream[i]; i++;
        int i6 = grid->hexaheders8.stream[i]; i++;
        int i7 = grid->hexaheders8.stream[i]; i++;

        owVector3d x0 = grid->points.stream[i0];
        owVector3d x1 = grid->points.stream[i1];
        owVector3d x2 = grid->points.stream[i2];
        owVector3d x3 = grid->points.stream[i3];
        owVector3d x4 = grid->points.stream[i4];
        owVector3d x5 = grid->points.stream[i5];
        owVector3d x6 = grid->points.stream[i6];
        owVector3d x7 = grid->points.stream[i7];

        double a0 = var[i0];
        double a1 = var[i1];
        double a2 = var[i2];
        double a3 = var[i3];
        double a4 = var[i4];
        double a5 = var[i5];
        double a6 = var[i6];
        double a7 = var[i7];

        owVector3d g0 = vertex_gradient( x0, x1, x3, x4, a0, a1, a3, a4 );
        owVector3d g1 = vertex_gradient( x1, x0, x2, x5, a1, a0, a2, a5 );
        owVector3d g2 = vertex_gradient( x2, x1, x3, x6, a2, a1, a3, a6 );
        owVector3d g3 = vertex_gradient( x3, x2, x0, x7, a3, a2, a0, a7 );
        owVector3d g4 = vertex_gradient( x4, x7, x5, x0, a4, a7, a5, a0 );
        owVector3d g5 = vertex_gradient( x5, x4, x6, x1, a5, a4, a6, a1 );
        owVector3d g6 = vertex_gradient( x6, x2, x5, x7, a6, a2, a5, a7 );
        owVector3d g7 = vertex_gradient( x7, x6, x4, x3, a7, a6, a4, a3 );

        grad[j].x += g0.x + g1.x + g2.x + g3.x + g4.x + g5.x + g6.x + g7.x;
        grad[j].y += g0.y + g1.y + g2.y + g3.y + g4.y + g5.y + g6.y + g7.y;
        grad[j].z += g0.z + g1.z + g2.z + g3.z + g4.z + g5.z + g6.z + g7.z;

        grad[j].x /= 8;
        grad[j].y /= 8;
        grad[j].z /= 8;

        double x = (x0.x + x1.x + x2.x + x3.x + x4.x + x5.x + x6.x + x7.x) / 8;
        double y = (x0.y + x1.y + x2.y + x3.y + x4.y + x5.y + x6.y + x7.y) / 8;
        double z = (x0.z + x1.z + x2.z + x3.z + x4.z + x5.z + x6.z + x7.z) / 8;

        _debug_( "%f == %f   %f == %f   %f == %f\n"
            , grad[j].x, 1 - 2 * x
            , grad[j].y, 1 - 2 * y
            , grad[j].z, 1 - 2 * z
            );
    }
}

static void gradients_from_elem_to_points
( const owTauGrid* grid
, const owVector3d* grad_tetra
, const owVector3d* grad_hexa
, owVector3d* grad_point
){
    int* n_con = nullptr;
    _check_( n_con = (int*)_calloc_( sizeof( int ), grid->points.length ) );

    for (size_t i = 0; i < grid->tetrahedrons4.length;){
        int i0 = grid->tetrahedrons4.stream[i]; i++;
        int i1 = grid->tetrahedrons4.stream[i]; i++;
        int i2 = grid->tetrahedrons4.stream[i]; i++;
        int i3 = grid->tetrahedrons4.stream[i]; i++;

        owVector3d ge = grad_tetra[i];

        grad_point[i0].x += ge.x;
        grad_point[i0].y += ge.y;
        grad_point[i0].z += ge.z;
        
        grad_point[i1].x += ge.x;
        grad_point[i1].y += ge.y;
        grad_point[i1].z += ge.z;

        grad_point[i2].x += ge.x;
        grad_point[i2].y += ge.y;
        grad_point[i2].z += ge.z;

        grad_point[i3].x += ge.x;
        grad_point[i3].y += ge.y;
        grad_point[i3].z += ge.z;

        n_con[i0] += 1;
        n_con[i1] += 1;
        n_con[i2] += 1;
        n_con[i3] += 1;
    }

    for (size_t i = 0; i < grid->hexaheders8.length;){
        int i0 = grid->hexaheders8.stream[i]; i++;
        int i1 = grid->hexaheders8.stream[i]; i++;
        int i2 = grid->hexaheders8.stream[i]; i++;
        int i3 = grid->hexaheders8.stream[i]; i++;
        int i4 = grid->hexaheders8.stream[i]; i++;
        int i5 = grid->hexaheders8.stream[i]; i++;
        int i6 = grid->hexaheders8.stream[i]; i++;
        int i7 = grid->hexaheders8.stream[i]; i++;

        owVector3d ge = grad_hexa[i];

        grad_point[i0].x += ge.x;
        grad_point[i0].y += ge.y;
        grad_point[i0].z += ge.z;

        grad_point[i1].x += ge.x;
        grad_point[i1].y += ge.y;
        grad_point[i1].z += ge.z;

        grad_point[i2].x += ge.x;
        grad_point[i2].y += ge.y;
        grad_point[i2].z += ge.z;

        grad_point[i3].x += ge.x;
        grad_point[i3].y += ge.y;
        grad_point[i3].z += ge.z;

        grad_point[i4].x += ge.x;
        grad_point[i4].y += ge.y;
        grad_point[i4].z += ge.z;

        grad_point[i5].x += ge.x;
        grad_point[i5].y += ge.y;
        grad_point[i5].z += ge.z;

        grad_point[i6].x += ge.x;
        grad_point[i6].y += ge.y;
        grad_point[i6].z += ge.z;

        grad_point[i7].x += ge.x;
        grad_point[i7].y += ge.y;
        grad_point[i7].z += ge.z;

        n_con[i0] += 1;
        n_con[i1] += 1;
        n_con[i2] += 1;
        n_con[i3] += 1;
        n_con[i4] += 1;
        n_con[i5] += 1;
        n_con[i6] += 1;
        n_con[i7] += 1;
    }

    for (size_t i = 0; i < grid->points.length; i++){
        grad_point[i].x /= n_con[i];
        grad_point[i].y /= n_con[i];
        grad_point[i].z /= n_con[i];
    }
}

/* Alternative algorithm to calculate the gradients */
owVector3dStream* adpTauGrid_gradients_jacobian
( const owTauGrid* grid      /* Tau's primary grid */
, const owDoubleStream* var      /* Stream of variables */
){
    owVector3dStream* grad_point_array = new owVector3dStream;
    grad_point_array->stream = nullptr;
    grad_point_array->length = 0;

    if (grid == nullptr){
        _warning_( "NULL pointer" );
        return grad_point_array;
    }

    if (grid->points.length <= 0){
        _warning_( "The grid is empty!" );
        return grad_point_array;
    }

    if (grid->points.length != var->length){
        _warning_( "The number of variables is not the number of grid points!" );
        return grad_point_array;
    }

    /* Allocate memory for the gradients */
    owVector3dStream grad_tetra_array;
    _check_( grad_tetra_array.stream
        = (owVector3d*)_calloc_( sizeof( owVector3d ), grid->tetrahedrons4.length ) );
    grad_tetra_array.length = grid->tetrahedrons4.length;

    owVector3dStream grad_hexa_array;
    _check_( grad_hexa_array.stream
        = (owVector3d*)_calloc_( sizeof( owVector3d ), grid->hexaheders8.length ) );
    grad_hexa_array.length = grid->hexaheders8.length;

    _check_( grad_point_array->stream
        = (owVector3d*)_calloc_( sizeof( owVector3d ), grid->points.length ) );
    grad_point_array->length = grid->points.length;

    gradients_tetra_jacobian( grid, var->stream, grad_tetra_array.stream );
    gradients_hexa_jacobian( grid, var->stream, grad_hexa_array.stream );
    /* MORE ELEMENTS! */

    /* Average the gradients calculated at the elements to the vertices */
    gradients_from_elem_to_points
        ( grid
        , grad_tetra_array.stream
        , grad_hexa_array.stream
        , grad_point_array->stream
        );

    return grad_point_array;
}
