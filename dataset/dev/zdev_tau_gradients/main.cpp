/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Test bench for calculating the gradients with TAU.
*******************************************************************************/

#include <memory.h>
#include <stdio.h>

#include "common/log.h"
#include "tau_tools/tau_tools.h"


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

double tetra_volumen
( const owVector3d x0
, const owVector3d x1
, const owVector3d x2
, const owVector3d x3
){
    owVector3d x10 = vector_sub( x1, x0 );
    owVector3d x20 = vector_sub( x2, x0 );
    owVector3d x30 = vector_sub( x3, x0 );
    owVector3d d012 = vector_cross( x10, x20 );
    double vol = x30.x * d012.x + x30.y * d012.y + x30.z * d012.z;

    return vol;
}

owTauPrimGrid* create_tetrahedra_test_grid( const size_t size )
{
    owTauPrimGrid* primgrid = owTauPrimGrid_create();
    primgrid->points.length = size*size*size;
    primgrid->points.stream
        = (owVector3d*)malloc( sizeof( owVector3d ) * size*size*size );

    size_t q = 0;
    for (size_t k = 0; k < size; k++){
        for (size_t j = 0; j < size; j++){
            for (size_t i = 0; i < size; i++){
                primgrid->points.stream[q].x = (double)i / (size - 1);
                primgrid->points.stream[q].y = (double)j / (size - 1);
                primgrid->points.stream[q].z = (double)k / (size - 1);
                q++;
            }
        }
    }

    primgrid->tetrahedrons4.length = 16 * (size - 1)*(size - 1)*(size - 1);
    primgrid->tetrahedrons4.stream
        = (int*)malloc( sizeof( int ) * 16 * (size - 1)*(size - 1)*(size - 1) );

    int* index = primgrid->tetrahedrons4.stream;
    q = 0;
    for (size_t k = 0; k < size - 1; k++){
        for (size_t j = 0; j < size - 1; j++){
            for (size_t i = 0; i < size - 1; i++){
                int i0 = k*size*size + j*size + i;
                int i1 = k*size*size + j*size + i + 1;
                int i2 = k*size*size + (j + 1)*size + i + 1;
                int i3 = k*size*size + (j + 1)*size + i;
                int i4 = (k + 1)*size*size + j*size + i;
                int i5 = (k + 1)*size*size + j*size + i + 1;
                int i6 = (k + 1)*size*size + (j + 1)*size + i + 1;
                int i7 = (k + 1)*size*size + (j + 1)*size + i;

                index[q] = i0; q++;
                index[q] = i3; q++;
                index[q] = i1; q++;
                index[q] = i4; q++;

                index[q] = i2; q++;
                index[q] = i1; q++;
                index[q] = i3; q++;
                index[q] = i6; q++;

                index[q] = i7; q++;
                index[q] = i4; q++;
                index[q] = i6; q++;
                index[q] = i3; q++;

                index[q] = i5; q++;
                index[q] = i1; q++;
                index[q] = i6; q++;
                index[q] = i4; q++;

                /* Check that the volume are positive */
                //adpVector3d x0, x1, x2, x3;
                //double v;
                //x0 = primgrid->points.stream[i0];
                //x1 = primgrid->points.stream[i1];
                //x2 = primgrid->points.stream[i3];
                //x3 = primgrid->points.stream[i4];
                //v = tetra_volumen( x0, x1, x2, x3 );
                //printf( "%f ", v );

                //x0 = primgrid->points.stream[i6];
                //x1 = primgrid->points.stream[i2];
                //x2 = primgrid->points.stream[i1];
                //x3 = primgrid->points.stream[i3];
                //v = tetra_volumen( x0, x1, x2, x3 );
                //printf( "%f ", v );

                //x0 = primgrid->points.stream[i7];
                //x1 = primgrid->points.stream[i6];
                //x2 = primgrid->points.stream[i4];
                //x3 = primgrid->points.stream[i3];
                //v = tetra_volumen( x0, x1, x2, x3 );
                //printf( "%f ", v );

                //x0 = primgrid->points.stream[i5];
                //x1 = primgrid->points.stream[i1];
                //x2 = primgrid->points.stream[i4];
                //x3 = primgrid->points.stream[i6];
                //v = tetra_volumen( x0, x1, x2, x3 );
                //printf( "%f ", v );

            }
        }
    }

    return primgrid;
}

owTauPrimGrid* create_hexahedra_test_grid( const size_t size )
{
    owTauPrimGrid* primgrid = owTauPrimGrid_create();
    primgrid->points.length = size*size*size;
    primgrid->points.stream
        = (owVector3d*)malloc( sizeof( owVector3d ) * size*size*size );

    size_t q = 0;
    for (size_t k = 0; k < size; k++){
        for (size_t j = 0; j < size; j++){
            for (size_t i = 0; i < size; i++){
                primgrid->points.stream[q].x = (double)i / (size - 1);
                primgrid->points.stream[q].y = (double)j / (size - 1);
                primgrid->points.stream[q].z = (double)k / (size - 1);
                q++;
            }
        }
    }

    primgrid->hexaheders8.length = 8 * (size - 1)*(size - 1)*(size - 1);
    primgrid->hexaheders8.stream
        = (int*)malloc( sizeof( int ) * 8 * (size - 1)*(size - 1)*(size - 1) );

    int* index = primgrid->hexaheders8.stream;
    q = 0;
    for (size_t k = 0; k < size - 1; k++){
        for (size_t j = 0; j < size - 1; j++){
            for (size_t i = 0; i < size - 1; i++){
                int i0 = k*size*size + j*size + i;
                int i1 = k*size*size + j*size + i + 1;
                int i2 = k*size*size + (j + 1)*size + i + 1;
                int i3 = k*size*size + (j + 1)*size + i;
                int i4 = (k + 1)*size*size + j*size + i;
                int i5 = (k + 1)*size*size + j*size + i + 1;
                int i6 = (k + 1)*size*size + (j + 1)*size + i + 1;
                int i7 = (k + 1)*size*size + (j + 1)*size + i;

                index[q] = i0; q++;
                index[q] = i1; q++;
                index[q] = i2; q++;
                index[q] = i3; q++;
                index[q] = i4; q++;
                index[q] = i5; q++;
                index[q] = i6; q++;
                index[q] = i7; q++;
            }
        }
    }

    return primgrid;
}

void validate_gradients()
{
    const int size = 11;
    owTauPrimGrid* grid = create_tetrahedra_test_grid( size );
    owDoubleStream* val = owDoubleStream_create( grid->points.length );

    for (size_t i = 0; i < grid->points.length; i++){
        double x = grid->points.stream[i].x;
        double y = grid->points.stream[i].y;
        double z = grid->points.stream[i].z;

        double v = x*(1 - x) + y*(1 - y) + z*(1 - z);
        val->stream[i] = v;
    }

    owVector3dStream* gu = owTauGrid_gradients_jacobian( grid, val );

    for (size_t k = 1; k < size; k++){
        for (size_t j = 1; j < size; j++){
            for (size_t i = 1; i < size; i++){
                size_t q = k*size*size + j*size + i;

                double x = grid->points.stream[q].x;
                double y = grid->points.stream[q].y;
                double z = grid->points.stream[q].z;

                printf( "%f == %f   %f == %f   %f == %f\n"
                    , gu->stream[q].x, 2 * x - 1
                    , gu->stream[q].y, 2 * y - 1
                    , gu->stream[q].z, 2 * z - 1
                    );
            }
        }
    }
}

int main( int argc, char *argv[] )
{
    validate_gradients();

    //adpTauPrimGrid* grid = create_hexahedra_test_grid( 10 );
    //adpTauPrimGrid* grid = create_tetrahedra_test_grid( 10 );
    //adpTauPrimGrid* grid = adpTauPrimGrid_import( "om6/ONERAM6.grid" );

    //adpDoubleStream* x_vel = adpTauSol_import
    //    ( "om6/om6.solution_central.gg.pval.719", "pressure" );

    //adpDoubleStream* gdx = adpTauSol_import
    //    ( "om6/om6.solution_central.gg.pval.719", "dpdx" );

    //adpDoubleStream* gdy = adpTauSol_import
    //    ( "om6/om6.solution_central.gg.pval.719", "dpdy" );

    //adpDoubleStream* gdz = adpTauSol_import
    //    ( "om6/om6.solution_central.gg.pval.719", "dpdz" );

    //tau_export_double( "om6/grad.pval", "gradx", gdx->stream, gdx->length );
    //tau_export_double( "om6/grad.pval", "grady", gdy->stream, gdx->length );
    //tau_export_double( "om6/grad.pval", "gradz", gdz->stream, gdx->length );

    getchar();

    return 0;
}
