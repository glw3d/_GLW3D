/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Loads CGNS file formats.

*******************************************************************************/

#include <stdio.h>
#include "cgns_tools/cgns_tools.h"

int main( int argc, char *argv[] )
{
    int num_grids;

    cgnsGrid* grid1 = cgnsGrid_import( "../test/cgns/rae2822_coarse_upd.cgns", &num_grids );
    for (int i = 0; i < 10; i++){
        printf( "%f %f %f\n", grid1->point_x.stream[i], grid1->point_y.stream[i], grid1->point_z.stream[i] );
    }
    cgnsGrid_free( grid1, num_grids );

    cgnsGrid* grid2 = cgnsGrid_import( "../test/cgns/HeatingCoil.cgns", &num_grids );
    printf( "\n" );
    for (int i = 0; i < 10; i++){
        printf( "%f %f %f\n", grid2->point_x.stream[i], grid2->point_y.stream[i], grid2->point_z.stream[i] );
    }
    cgnsGrid_free( grid2, num_grids );

    cgnsGrid* grid3 = cgnsGrid_import( "../test/cgns/dpw_w1_Euler.cgns", &num_grids );
    printf( "\n" );
    for (int i = 0; i < 10; i++){
        printf( "%f %f %f\n", grid3->point_x.stream[i], grid3->point_y.stream[i], grid3->point_z.stream[i] );
    }
    cgnsGrid_free( grid3, num_grids );

    getchar();

    return 0;
}
