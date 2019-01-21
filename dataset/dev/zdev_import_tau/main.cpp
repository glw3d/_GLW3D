/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Test bench for importing TAU grids and solutions in NETCDF file format.
*******************************************************************************/

#include <stdio.h>
#include "tau_tools/tau_tools.h"

int main(int argc, char *argv[])
{
    const char* grid_filename = "../test/naca0012/naca0012.grid";
    owTauGrid* grid = owTauGrid_import( grid_filename );
    printf( "label: %s\n", grid->label );
    printf( "num points: %i\n", grid->points.length );
    printf( "num triangles: %i\n", grid->surface_tri3.length );
    printf( "num quads: %i\n", grid->surface_quad4.length );
    printf( "\n" );
    owTauGrid_free( grid );

    printf("---Variables---\n");
    ncDataSet* dataset = nc_query
        ( "../test/naca0012/naca0012.solution.pval.gg.790" );
    if (dataset != nullptr){
        for (int i = 0; i < dataset->num_vars; i++){
            printf( "%s\n", dataset->var_names[i] );
        }
        printf( "\n---Attributes---\n" );
        for (int i = 0; i < dataset->num_atts; i++){
            printf( "%s\n", dataset->att_names[i] );
        }
    }
    ncDataSet_free( dataset );

    owDoubleStream* sol_pressure = nc_import_dbl
        ( "../test/naca0012/naca0012.solution.pval.gg.790", "cp" );
    owDoubleStream_free( sol_pressure );

    owIntStream* gid = nc_import_int
        ( "../test/naca0012/naca0012.solution.pval.gg.790", "global_id");
    owIntStream_free( gid );

    double clift = nc_getatt
        ( "../test/naca0012/naca0012.solution.pval.gg.790", "c_l" );

    getchar();

    return 0;
}
