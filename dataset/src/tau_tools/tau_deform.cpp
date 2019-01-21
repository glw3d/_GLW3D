/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Implements a mesh deformation for TAU grids 
*******************************************************************************/

#include <math.h>
#include <stdio.h>

#include "common/definitions.h"
#include "common/check_malloc.h"
#include "common/log.h"

#include "tau_tools.h"

static inline owDouble get_norma( const owVector3d pa, const owVector3d pb )
{
    owDouble dx = pa.x - pb.x;
    owDouble dy = pa.y - pb.y;
    owDouble dz = pa.z - pb.z;
    owDouble m = dx*dx + dy*dy + dz*dz;
    m = m*m;

    return  m;
}

static inline int check_marker( const int* wall_markers, const int marker )
{
    const int* pm = wall_markers;
    while (*pm != 0){
        if (*pm == marker){
            return 1;
        }
        pm++;
    }
    return 0;
}

/* Deforms the tau grid in 2d after a deformation 
 * def_grid is he deformed grid that will be modified, 
 * the original grid is required to calculate the normas,
 * and markers indicate the boundary surfaces (not the symmetry planes), 
 * including the farfield, and it must be 0 terminated */
double tau2d_deform_laplace_position
( owTauGrid* def_grid, const owTauGrid* orig_grid
, const int* markers, const int num_iterations, const double epsilon )
{
    bool* flag = nullptr;
    double* sum_norms = nullptr;
    owVector3d* deform = nullptr;

    if (def_grid == nullptr || orig_grid == nullptr){
        return 0;
    }

    _check_( flag = (bool*)_calloc_( orig_grid->points.length, sizeof( bool ) ) );
    if (flag == nullptr){
        /* Out of memory */
        goto END;
    }

    _check_( sum_norms = (owDouble*)_calloc_( orig_grid->points.length, sizeof( owDouble ) ) );
    if (sum_norms == nullptr){
        /* Out of memory */
        goto END;
    }

    /* Identify the boundary surface vertices and calculate the sum of the normas */
    size_t j = 0;
    for (size_t i = 0; i < orig_grid->surface_tri3.length;){
        int i0 = orig_grid->surface_tri3.stream[i]; i++;
        int i1 = orig_grid->surface_tri3.stream[i]; i++;
        int i2 = orig_grid->surface_tri3.stream[i]; i++;
        int marker = orig_grid->marker_triangles.stream[j]; j++;

        owVector3d p0 = orig_grid->points.stream[i0];
        owVector3d p1 = orig_grid->points.stream[i1];
        owVector3d p2 = orig_grid->points.stream[i2];

        /* Check if it is a boundary */
        if (check_marker( markers, marker ) != 0){
            flag[i0] = 1;
            flag[i1] = 1;
            flag[i2] = 1;
        }

        owDouble norma01 = get_norma( p0, p1 );
        owDouble norma12 = get_norma( p1, p2 );
        owDouble norma20 = get_norma( p2, p0 );

        sum_norms[i0] += 1. / norma01;
        sum_norms[i1] += 1. / norma12;
        sum_norms[i2] += 1. / norma20;
    }

    j = 0;
    for (size_t i = 0; i < orig_grid->surface_quad4.length;){
        int i0 = orig_grid->surface_quad4.stream[i]; i++;
        int i1 = orig_grid->surface_quad4.stream[i]; i++;
        int i2 = orig_grid->surface_quad4.stream[i]; i++;
        int i3 = orig_grid->surface_quad4.stream[i]; i++;
        int marker = orig_grid->marker_quads.stream[j]; j++;

        owVector3d p0 = orig_grid->points.stream[i0];
        owVector3d p1 = orig_grid->points.stream[i1];
        owVector3d p2 = orig_grid->points.stream[i2];
        owVector3d p3 = orig_grid->points.stream[i3];

        /* Check if it is a boundary */
        if (check_marker( markers, marker ) != 0){
            flag[i0] = 1;
            flag[i1] = 1;
            flag[i2] = 1;
            flag[i3] = 1;
        }

        owDouble norma01 = get_norma( p0, p1 );
        owDouble norma12 = get_norma( p1, p2 );
        owDouble norma23 = get_norma( p2, p3 );
        owDouble norma30 = get_norma( p3, p0 );

        sum_norms[i0] += 1. / norma01;
        sum_norms[i1] += 1. / norma12;
        sum_norms[i2] += 1. / norma23;
        sum_norms[i3] += 1. / norma30;
    }

    /* Main loop of the deformation algorithm */
    _check_( deform = (owVector3d*)_calloc_( orig_grid->points.length, sizeof( owVector3d ) ) );
    if (deform == nullptr){
        /* Out of memory */
        goto END;
    }

    int it = 0;
    owDouble max_residual = 1e9;
    while (max_residual > epsilon && it < num_iterations){
        max_residual = 0;
        it++;

        for (size_t i = 0; i < orig_grid->surface_tri3.length;){
            int i0 = orig_grid->surface_tri3.stream[i]; i++;
            int i1 = orig_grid->surface_tri3.stream[i]; i++;
            int i2 = orig_grid->surface_tri3.stream[i]; i++;
            int marker = orig_grid->marker_triangles.stream[j];

            owVector3d p0 = orig_grid->points.stream[i0];
            owVector3d p1 = orig_grid->points.stream[i1];
            owVector3d p2 = orig_grid->points.stream[i2];

            owVector3d q0 = def_grid->points.stream[i0];
            owVector3d q1 = def_grid->points.stream[i1];
            owVector3d q2 = def_grid->points.stream[i2];

            owDouble norma01 = get_norma( p0, p1 );
            owDouble norma12 = get_norma( p1, p2 );
            owDouble norma20 = get_norma( p2, p0 );

            deform[i0].x += (q1.x - p1.x) / norma01;
            deform[i0].y += (q1.y - p1.y) / norma01;
            deform[i0].z += (q1.z - p1.z) / norma01;

            deform[i1].x += (q2.x - p2.x) / norma12;
            deform[i1].y += (q2.y - p2.y) / norma12;
            deform[i1].z += (q2.z - p2.z) / norma12;

            deform[i2].x += (q0.x - p0.x) / norma20;
            deform[i2].y += (q0.y - p0.y) / norma20;
            deform[i2].z += (q0.z - p0.z) / norma20;
        }

        j = 0;
        for (size_t i = 0; i < orig_grid->surface_quad4.length;){
            int i0 = orig_grid->surface_quad4.stream[i]; i++;
            int i1 = orig_grid->surface_quad4.stream[i]; i++;
            int i2 = orig_grid->surface_quad4.stream[i]; i++;
            int i3 = orig_grid->surface_quad4.stream[i]; i++;
            int marker = orig_grid->marker_quads.stream[j];

            owVector3d p0 = orig_grid->points.stream[i0];
            owVector3d p1 = orig_grid->points.stream[i1];
            owVector3d p2 = orig_grid->points.stream[i2];
            owVector3d p3 = orig_grid->points.stream[i3];

            owVector3d q0 = def_grid->points.stream[i0];
            owVector3d q1 = def_grid->points.stream[i1];
            owVector3d q2 = def_grid->points.stream[i2];
            owVector3d q3 = def_grid->points.stream[i3];

            owDouble norma01 = get_norma( p0, p1 );
            owDouble norma12 = get_norma( p1, p2 );
            owDouble norma23 = get_norma( p2, p3 );
            owDouble norma30 = get_norma( p3, p0 );

            deform[i0].x += (q1.x - p1.x) / norma01;
            deform[i0].y += (q1.y - p1.y) / norma01;
            deform[i0].z += (q1.z - p1.z) / norma01;

            deform[i1].x += (q2.x - p2.x) / norma12;
            deform[i1].y += (q2.y - p2.y) / norma12;
            deform[i1].z += (q2.z - p2.z) / norma12;

            deform[i2].x += (q3.x - p3.x) / norma23;
            deform[i2].y += (q3.y - p3.y) / norma23;
            deform[i2].z += (q3.z - p3.z) / norma23;

            deform[i3].x += (q0.x - p0.x) / norma30;
            deform[i3].y += (q0.y - p0.y) / norma30;
            deform[i3].z += (q0.z - p0.z) / norma30;
        }

        for (size_t i = 0; i < orig_grid->points.length; i++){
            if (flag[i] == 0){
                owDouble x0 = def_grid->points.stream[i].x;
                owDouble y0 = def_grid->points.stream[i].y;
                owDouble z0 = def_grid->points.stream[i].z;

                owDouble x1 = deform[i].x / sum_norms[i] + orig_grid->points.stream[i].x;
                owDouble y1 = deform[i].y / sum_norms[i] + orig_grid->points.stream[i].y;
                owDouble z1 = deform[i].z / sum_norms[i] + orig_grid->points.stream[i].z;

                def_grid->points.stream[i].x = x1;
                def_grid->points.stream[i].y = y1;
                def_grid->points.stream[i].z = z1;

                deform[i].x = 0;
                deform[i].y = 0;
                deform[i].z = 0;

                owDouble res = ((x1 - x0)*(x1 - x0) 
                    + (y1 - y0)*(y1 - y0) + (z1 - z0)*(z1 - z0)) * sum_norms[i];

                if (res > max_residual){
                    max_residual = res;
                }
            }
        }
    }

END:
    free( flag );
    free( sum_norms );
    free( deform );

    return max_residual;
}

static void calculate_angle_zeta
( owDouble* r
, owDouble* cos_zeta, owDouble* sin_zeta
, const owVector3d p0, const owVector3d p1, const owVector3d p2 )
{
    owVector3d r21 = { p2.x - p1.x, 0, p2.z - p1.z };
    owDouble m21 = sqrt( (r21.x * r21.x) + (r21.z * r21.z) );
    owVector3d r01 = { p0.x - p1.x, 0, p0.z - p1.z };
    owDouble m01 = sqrt( (r01.x * r01.x) + (r01.z * r01.z) );

    *r = m01;
    double cb = r21.x / m21;
    double sb = r21.z / m21;

    owVector3d k01 = { r01.x*cb + r01.z*sb, 0, -r01.x*sb + r01.z*cb };
    owDouble n01 = sqrt( (k01.x * k01.x) + (k01.z * k01.z) );

    *cos_zeta = k01.x / n01;
    *sin_zeta = k01.z / n01;
}

static void calculate_angle_beta
( owDouble* cos_beta, owDouble* sin_beta
, const owVector3d p0, const owVector3d p1, const owVector3d p2 )
{
    owVector3d r21 = { p2.x - p1.x, 0, p2.z - p1.z };
    owDouble m21 = sqrt( (r21.x * r21.x) + (r21.z * r21.z) );
    owVector3d r01 = { p0.x - p1.x, 0, p0.z - p1.z };
    owDouble m01 = sqrt( (r01.x * r01.x) + (r01.z * r01.z) );

    double cb = r21.x / m21;
    double sb = r21.z / m21;
    *cos_beta = cb;
    *sin_beta = sb;

    owVector3d k01 = { r01.x*cb + r01.z*sb, 0, -r01.x*sb + r01.z*cb };
    owDouble n01 = sqrt( (k01.x * k01.x) + (k01.z * k01.z) );
}

void get_wall_distances( const owTauGrid* grid, int* flag, const int* wall_markers )
{
    register int changed = 1;
    while (changed != 0){
        changed = 0;
        for (size_t i = 0; i < grid->surface_tri3.length;){
            int i0 = grid->surface_tri3.stream[i]; i++;
            int i1 = grid->surface_tri3.stream[i]; i++;
            int i2 = grid->surface_tri3.stream[i]; i++;

            int f0 = flag[i0];
            int f1 = flag[i1];
            int f2 = flag[i2];

            if (f0 > 0){
                if (f1 == 0){
                    f1 = f0 + 1;
                    changed = 1;
                }
                if (f2 == 0){
                    f2 = f0 + 1;
                    changed = 1;
                }
            }
            if (f1 > 0){
                if (f0 == 0){
                    f0 = f1 + 1;
                    changed = 1;
                }
                if (f2 == 0){
                    f2 = f1 + 1;
                    changed = 1;
                }
            }
            if (f2 > 0){
                if (f0 == 0){
                    f0 = f2 + 1;
                    changed = 1;
                }
                if (f1 == 0){
                    f1 = f2 + 1;
                    changed = 1;
                }
            }

            flag[i0] = f0;
            flag[i1] = f1;
            flag[i2] = f2;
        }

        for (size_t i = 0; i < grid->surface_quad4.length;){
            int i0 = grid->surface_quad4.stream[i]; i++;
            int i1 = grid->surface_quad4.stream[i]; i++;
            int i2 = grid->surface_quad4.stream[i]; i++;
            int i3 = grid->surface_quad4.stream[i]; i++;

            int f0 = flag[i0];
            int f1 = flag[i1];
            int f2 = flag[i2];
            int f3 = flag[i3];

            if (f0 > 0){
                if (f1 == 0){
                    f1 = f0 + 1;
                    changed = 1;
                }
                if (f3 == 0){
                    f3 = f0 + 1;
                    changed = 1;
                }
            }
            if (f1 > 0){
                if (f0 == 0){
                    f0 = f1 + 1;
                    changed = 1;
                }
                if (f2 == 0){
                    f2 = f1 + 1;
                    changed = 1;
                }
            }
            if (f2 > 0){
                if (f1 == 0){
                    f1 = f2 + 1;
                    changed = 1;
                }
                if (f3 == 0){
                    f3 = f2 + 1;
                    changed = 1;
                }
            }
            if (f3 > 0){
                if (f0 == 0){
                    f0 = f3 + 1;
                    changed = 1;
                }
                if (f2 == 0){
                    f2 = f3 + 1;
                    changed = 1;
                }
            }

            flag[i0] = f0;
            flag[i1] = f1;
            flag[i2] = f2;
            flag[i3] = f3;
        }
    }
}

void order_elements( int* order_tri, int* order_quad
    , const owTauGrid* grid, const int* flag )
{
    int dist = 1;
    size_t index_tri = 0;
    size_t index_quad = 0;
    int* elm_tri = (int*)_calloc_( grid->marker_triangles.length, sizeof( int ) );
    int* elm_quad = (int*)_calloc_( grid->marker_quads.length, sizeof( int ) );

    while (index_tri < grid->surface_tri3.length){
        size_t j = 0;
        for (size_t i = 0; i < grid->surface_tri3.length;){
            int i0 = grid->surface_tri3.stream[i]; i++;
            int i1 = grid->surface_tri3.stream[i]; i++;
            int i2 = grid->surface_tri3.stream[i]; i++;

            int f0 = flag[i0];
            int f1 = flag[i1];
            int f2 = flag[i2];

            if (elm_tri[j] == 0){
                if (f0 == dist || f1 == dist || f2 == dist){
                    order_tri[index_tri] = i0; index_tri++;
                    order_tri[index_tri] = i1; index_tri++;
                    order_tri[index_tri] = i2; index_tri++;

                    elm_tri[j] = 1;
                }
            }
            j++;
        }
        dist++;
    }

    dist = 1;
    while (index_quad < grid->surface_quad4.length){
        size_t j = 0;
        for (size_t i = 0; i < grid->surface_quad4.length;){
            int i0 = grid->surface_quad4.stream[i]; i++;
            int i1 = grid->surface_quad4.stream[i]; i++;
            int i2 = grid->surface_quad4.stream[i]; i++;
            int i3 = grid->surface_quad4.stream[i]; i++;

            int f0 = flag[i0];
            int f1 = flag[i1];
            int f2 = flag[i2];
            int f3 = flag[i3];

            if (elm_quad[j] == 0){
                if (f0 == dist || f1 == dist || f2 == dist || f3 == dist){
                    order_quad[index_quad] = i0; index_quad++;
                    order_quad[index_quad] = i1; index_quad++;
                    order_quad[index_quad] = i2; index_quad++;
                    order_quad[index_quad] = i3; index_quad++;

                    elm_quad[j] = 1;
                }
            }
            j++;
        }
        dist++;
    }

    free( elm_tri );
    free( elm_quad );
}

/* Deforms the tau grid in 2d after a deformation
* def_grid is he deformed grid that will be modified,
* the original grid is required to calculate the normas,
* and markers indicate the boundary surfaces (not the symmetry planes),
* including the farfield, and it must be 0 terminated; e.g. {3, 6, 0} */
double tau2d_deform_laplace_ortho
( owTauGrid* def_grid
, const owTauGrid* orig_grid
, const int* wall_markers
, const int num_iterations
, const double stop_condition
, const double beta
)
{
    /* Used to indicate the wall vertices and its distance to the wall */
    int* flag = nullptr;
    double epsilon = stop_condition * stop_condition;

    /* Used to reorder the elements, so those close to the wall are computed first */
    int* order_tri = nullptr;
    int* order_quad = nullptr;
    double* sum_norms = nullptr;
    owVector3d* deform_shape = nullptr;
    owVector3d* deform_coord = nullptr;

    if (def_grid == nullptr || orig_grid == nullptr){
        return 0;
    }

    _check_( flag = (int*)_calloc_( orig_grid->points.length, sizeof( int ) ) );
    _check_( order_tri = (int*)_calloc_( orig_grid->surface_tri3.length, sizeof( int ) ) );
    _check_( order_quad = (int*)_calloc_( orig_grid->surface_quad4.length, sizeof( int ) ) );
    _check_( sum_norms = (owDouble*)_calloc_( orig_grid->points.length, sizeof( owDouble ) ) );

    /* Identify the boundary surface vertices and calculate the sum of the normas */
    size_t j = 0;
    for (size_t i = 0; i < orig_grid->surface_tri3.length;){
        int i0 = orig_grid->surface_tri3.stream[i]; i++;
        int i1 = orig_grid->surface_tri3.stream[i]; i++;
        int i2 = orig_grid->surface_tri3.stream[i]; i++;
        int marker = orig_grid->marker_triangles.stream[j]; j++;

        owVector3d p0 = orig_grid->points.stream[i0];
        owVector3d p1 = orig_grid->points.stream[i1];
        owVector3d p2 = orig_grid->points.stream[i2];

        /* Check if it is a boundary */
        const int* pm = wall_markers;
        while (*pm != 0){
            if (*pm == marker){
                flag[i0] = 1;
                flag[i1] = 1;
                flag[i2] = 1;
            }
            pm++;
        }

        sum_norms[i0] += 1. / get_norma( p0, p1 );
        sum_norms[i1] += 1. / get_norma( p1, p2 );
        sum_norms[i2] += 1. / get_norma( p2, p0 );
    }

    j = 0;
    for (size_t i = 0; i < orig_grid->surface_quad4.length;){
        int i0 = orig_grid->surface_quad4.stream[i]; i++;
        int i1 = orig_grid->surface_quad4.stream[i]; i++;
        int i2 = orig_grid->surface_quad4.stream[i]; i++;
        int i3 = orig_grid->surface_quad4.stream[i]; i++;
        int marker = orig_grid->marker_quads.stream[j]; j++;

        owVector3d p0 = orig_grid->points.stream[i0];
        owVector3d p1 = orig_grid->points.stream[i1];
        owVector3d p2 = orig_grid->points.stream[i2];
        owVector3d p3 = orig_grid->points.stream[i3];

        /* Check if it is a boundary */
        const int* pm = wall_markers;
        while (*pm != 0){
            if (*pm == marker){
                flag[i0] = 1;
                flag[i1] = 1;
                flag[i2] = 1;
                flag[i3] = 1;
            }
            pm++;
        }

        sum_norms[i0] += 1. / get_norma( p0, p1 );
        sum_norms[i1] += 1. / get_norma( p1, p2 );
        sum_norms[i2] += 1. / get_norma( p2, p3 );
        sum_norms[i3] += 1. / get_norma( p3, p0 );
    }

    /* Arrange the points by its distance to the wall */
    get_wall_distances( orig_grid, flag, wall_markers );
    order_elements( order_tri, order_quad, orig_grid, flag );

    /* There are two deformation functions. One keep the relative
    * positions of the vertices, the second fix the orthogonlity of the elements*/
    _check_( deform_shape = (owVector3d*)_calloc_
        ( orig_grid->points.length, sizeof( owVector3d ) ) );
    _check_( deform_coord = (owVector3d*)_calloc_
        ( orig_grid->points.length, sizeof( owVector3d ) ) );

    /* Main loop of the deformation algorithm */
    int it = 0;
    owDouble max_residual = 1e9;
    while (max_residual > epsilon && it < num_iterations){
        max_residual = 0;
        it++;

        for (size_t i = 0; i < orig_grid->surface_tri3.length;){
            int i0 = order_tri[i]; i++;
            int i1 = order_tri[i]; i++;
            int i2 = order_tri[i]; i++;

            //int i0 = orig_grid->surface_tri3.stream[i]; i++;
            //int i1 = orig_grid->surface_tri3.stream[i]; i++;
            //int i2 = orig_grid->surface_tri3.stream[i]; i++;

            owVector3d p0 = orig_grid->points.stream[i0];
            owVector3d p1 = orig_grid->points.stream[i1];
            owVector3d p2 = orig_grid->points.stream[i2];

            owVector3d q0 = def_grid->points.stream[i0];
            owVector3d q1 = def_grid->points.stream[i1];
            owVector3d q2 = def_grid->points.stream[i2];

            owDouble norma01 = get_norma( p0, p1 );
            owDouble norma12 = get_norma( p1, p2 );
            owDouble norma20 = get_norma( p2, p0 );

            owDouble cz0, sz0, r0, cb1, sb1;

            calculate_angle_zeta( &r0, &cz0, &sz0, p0, p1, p2 );
            calculate_angle_beta( &cb1, &sb1, q0, q1, q2 );
            deform_shape[i0].x += 0.5 * (q1.x + r0*(cb1*cz0 - sb1*sz0)) / norma01;
            deform_shape[i0].z += 0.5 * (q1.z + r0*(cb1*sz0 + sb1*cz0)) / norma01;

            calculate_angle_zeta( &r0, &cz0, &sz0, p0, p2, p1 );
            calculate_angle_beta( &cb1, &sb1, q0, q2, q1 );
            deform_shape[i0].x += 0.5 * (q2.x + r0*(cb1*cz0 - sb1*sz0)) / norma20;
            deform_shape[i0].z += 0.5 * (q2.z + r0*(cb1*sz0 + sb1*cz0)) / norma20;


            calculate_angle_zeta( &r0, &cz0, &sz0, p1, p2, p0 );
            calculate_angle_beta( &cb1, &sb1, q1, q2, q0 );
            deform_shape[i1].x += 0.5 * (q2.x + r0*(cb1*cz0 - sb1*sz0)) / norma12;
            deform_shape[i1].z += 0.5 * (q2.z + r0*(cb1*sz0 + sb1*cz0)) / norma12;

            calculate_angle_zeta( &r0, &cz0, &sz0, p1, p0, p2 );
            calculate_angle_beta( &cb1, &sb1, q1, q0, q2 );
            deform_shape[i1].x += 0.5 * (q0.x + r0*(cb1*cz0 - sb1*sz0)) / norma01;
            deform_shape[i1].z += 0.5 * (q0.z + r0*(cb1*sz0 + sb1*cz0)) / norma01;


            calculate_angle_zeta( &r0, &cz0, &sz0, p2, p0, p1 );
            calculate_angle_beta( &cb1, &sb1, q2, q0, q1 );
            deform_shape[i2].x += 0.5 * (q0.x + r0*(cb1*cz0 - sb1*sz0)) / norma20;
            deform_shape[i2].z += 0.5 * (q0.z + r0*(cb1*sz0 + sb1*cz0)) / norma20;

            calculate_angle_zeta( &r0, &cz0, &sz0, p2, p1, p0 );
            calculate_angle_beta( &cb1, &sb1, q2, q1, q0 );
            deform_shape[i2].x += 0.5 * (q1.x + r0*(cb1*cz0 - sb1*sz0)) / norma12;
            deform_shape[i2].z += 0.5 * (q1.z + r0*(cb1*sz0 + sb1*cz0)) / norma12;


            deform_coord[i0].x += (q1.x - p1.x) / norma01;
            deform_coord[i0].y += (q1.y - p1.y) / norma01;
            deform_coord[i0].z += (q1.z - p1.z) / norma01;

            deform_coord[i1].x += (q2.x - p2.x) / norma12;
            deform_coord[i1].y += (q2.y - p2.y) / norma12;
            deform_coord[i1].z += (q2.z - p2.z) / norma12;

            deform_coord[i2].x += (q0.x - p0.x) / norma20;
            deform_coord[i2].y += (q0.y - p0.y) / norma20;
            deform_coord[i2].z += (q0.z - p0.z) / norma20;
        }

        j = 0;
        for (size_t i = 0; i < orig_grid->surface_quad4.length;){
            int i0 = order_quad[i]; i++;
            int i1 = order_quad[i]; i++;
            int i2 = order_quad[i]; i++;
            int i3 = order_quad[i]; i++;

            //int i0 = orig_grid->surface_quad4.stream[i]; i++;
            //int i1 = orig_grid->surface_quad4.stream[i]; i++;
            //int i2 = orig_grid->surface_quad4.stream[i]; i++;
            //int i3 = orig_grid->surface_quad4.stream[i]; i++;

            owVector3d p0 = orig_grid->points.stream[i0];
            owVector3d p1 = orig_grid->points.stream[i1];
            owVector3d p2 = orig_grid->points.stream[i2];
            owVector3d p3 = orig_grid->points.stream[i3];

            owVector3d q0 = def_grid->points.stream[i0];
            owVector3d q1 = def_grid->points.stream[i1];
            owVector3d q2 = def_grid->points.stream[i2];
            owVector3d q3 = def_grid->points.stream[i3];

            owDouble cz0, sz0, r0, cb1, sb1;

            owDouble norma01 = get_norma( p0, p1 );
            owDouble norma12 = get_norma( p1, p2 );
            owDouble norma23 = get_norma( p2, p3 );
            owDouble norma30 = get_norma( p3, p0 );

            calculate_angle_zeta( &r0, &cz0, &sz0, p0, p1, p2 );
            calculate_angle_beta( &cb1, &sb1, q0, q1, q2 );
            deform_shape[i0].x += 0.5 * (q1.x + r0*(cb1*cz0 - sb1*sz0)) / norma01;
            deform_shape[i0].z += 0.5 * (q1.z + r0*(cb1*sz0 + sb1*cz0)) / norma01;

            calculate_angle_zeta( &r0, &cz0, &sz0, p0, p3, p2 );
            calculate_angle_beta( &cb1, &sb1, q0, q3, q2 );
            deform_shape[i0].x += 0.5 * (q3.x + r0*(cb1*cz0 - sb1*sz0)) / norma30;
            deform_shape[i0].z += 0.5 * (q3.z + r0*(cb1*sz0 + sb1*cz0)) / norma30;


            calculate_angle_zeta( &r0, &cz0, &sz0, p1, p2, p3 );
            calculate_angle_beta( &cb1, &sb1, q1, q2, q3 );
            deform_shape[i1].x += 0.5 * (q2.x + r0*(cb1*cz0 - sb1*sz0)) / norma12;
            deform_shape[i1].z += 0.5 * (q2.z + r0*(cb1*sz0 + sb1*cz0)) / norma12;

            calculate_angle_zeta( &r0, &cz0, &sz0, p1, p0, p3 );
            calculate_angle_beta( &cb1, &sb1, q1, q0, q3 );
            deform_shape[i1].x += 0.5 * (q0.x + r0*(cb1*cz0 - sb1*sz0)) / norma01;
            deform_shape[i1].z += 0.5 * (q0.z + r0*(cb1*sz0 + sb1*cz0)) / norma01;


            calculate_angle_zeta( &r0, &cz0, &sz0, p2, p3, p0 );
            calculate_angle_beta( &cb1, &sb1, q2, q3, q0 );
            deform_shape[i2].x += 0.5 * (q3.x + r0*(cb1*cz0 - sb1*sz0)) / norma23;
            deform_shape[i2].z += 0.5 * (q3.z + r0*(cb1*sz0 + sb1*cz0)) / norma23;

            calculate_angle_zeta( &r0, &cz0, &sz0, p2, p1, p0 );
            calculate_angle_beta( &cb1, &sb1, q2, q1, q0 );
            deform_shape[i2].x += 0.5 * (q1.x + r0*(cb1*cz0 - sb1*sz0)) / norma12;
            deform_shape[i2].z += 0.5 * (q1.z + r0*(cb1*sz0 + sb1*cz0)) / norma12;


            calculate_angle_zeta( &r0, &cz0, &sz0, p3, p0, p1 );
            calculate_angle_beta( &cb1, &sb1, q3, q0, q1 );
            deform_shape[i3].x += 0.5 * (q0.x + r0*(cb1*cz0 - sb1*sz0)) / norma30;
            deform_shape[i3].z += 0.5 * (q0.z + r0*(cb1*sz0 + sb1*cz0)) / norma30;

            calculate_angle_zeta( &r0, &cz0, &sz0, p3, p2, p1 );
            calculate_angle_beta( &cb1, &sb1, q3, q2, q1 );
            deform_shape[i3].x += 0.5 * (q2.x + r0*(cb1*cz0 - sb1*sz0)) / norma23;
            deform_shape[i3].z += 0.5 * (q2.z + r0*(cb1*sz0 + sb1*cz0)) / norma23;


            deform_coord[i0].x += (q1.x - p1.x) / norma01;
            deform_coord[i0].y += (q1.y - p1.y) / norma01;
            deform_coord[i0].z += (q1.z - p1.z) / norma01;

            deform_coord[i1].x += (q2.x - p2.x) / norma12;
            deform_coord[i1].y += (q2.y - p2.y) / norma12;
            deform_coord[i1].z += (q2.z - p2.z) / norma12;

            deform_coord[i2].x += (q3.x - p3.x) / norma23;
            deform_coord[i2].y += (q3.y - p3.y) / norma23;
            deform_coord[i2].z += (q3.z - p3.z) / norma23;

            deform_coord[i3].x += (q0.x - p0.x) / norma30;
            deform_coord[i3].y += (q0.y - p0.y) / norma30;
            deform_coord[i3].z += (q0.z - p0.z) / norma30;
        }

        for (size_t i = 0; i < orig_grid->points.length; i++){
            if (flag[i] != 1){
                const double alpha = 1 - 0.5 * beta;
                owDouble xe = def_grid->points.stream[i].x;
                owDouble ye = def_grid->points.stream[i].y;
                owDouble ze = def_grid->points.stream[i].z;

                owDouble x1 = beta*deform_shape[i].x / sum_norms[i];
                x1 += (1 - beta)*(deform_coord[i].x) / sum_norms[i];
                x1 += (1 - beta)*orig_grid->points.stream[i].x;

                owDouble y1 = orig_grid->points.stream[i].y;

                owDouble z1 = beta*deform_shape[i].z / sum_norms[i];
                z1 += (1 - beta)*(deform_coord[i].z) / sum_norms[i];
                z1 += (1 - beta)*orig_grid->points.stream[i].z;

                def_grid->points.stream[i].x = (1 - alpha)*def_grid->points.stream[i].x + alpha*x1;
                def_grid->points.stream[i].y = (1 - alpha)*def_grid->points.stream[i].y + alpha*y1;
                def_grid->points.stream[i].z = (1 - alpha)*def_grid->points.stream[i].z + alpha*z1;

                deform_shape[i].x = 0;
                deform_shape[i].y = 0;
                deform_shape[i].z = 0;

                deform_coord[i].x = 0;
                deform_coord[i].y = 0;
                deform_coord[i].z = 0;

                owDouble res = ((x1 - xe)*(x1 - xe)
                    + (y1 - ye)*(y1 - ye)
                    + (z1 - ze)*(z1 - ze)) * sum_norms[i];

                if (res > max_residual){
                    max_residual = res;
                }
            }
        }
    }

    free( flag );
    free( sum_norms );
    free( deform_coord );
    free( deform_shape );
    free( order_tri );
    free( order_quad );

    return max_residual;
}

