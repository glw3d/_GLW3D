/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Initializers and releases CGNS grid data structures.

*******************************************************************************/

#include <stdlib.h>

#include "common/definitions.h"
#include "cgnsGrid.h"

/* Releases the memory */
static void free_sections( cgnsGrid* grid )
{
    size_t i;

    if (grid != nullptr && grid->section != nullptr){
        for (i = 0; i < grid->num_sections; i++){
            if (grid->section[i].bar2_stream != nullptr){
                free( grid->section[i].bar2_stream );
            }
            if (grid->section[i].tri3_stream != nullptr){
                free( grid->section[i].tri3_stream );
            }
            if (grid->section[i].quad4_stream != nullptr){
                free( grid->section[i].quad4_stream );
            }
            if (grid->section[i].tetra4_stream != nullptr){
                free( grid->section[i].tetra4_stream );
            }
            if (grid->section[i].pyra5_stream != nullptr){
                free( grid->section[i].pyra5_stream );
            }
            if (grid->section[i].penta6_stream != nullptr){
                free( grid->section[i].penta6_stream );
            }
            if (grid->section[i].hexa8_stream != nullptr){
                free( grid->section[i].hexa8_stream );
            }
        }

        free( grid->section );
        grid->section = nullptr;
        grid->num_sections = 0;
    }

    grid->num_sections = 0;
    grid->section = nullptr;
}

/* Releases the memory */
static void dispose_grid( cgnsGrid* grid )
{
    if (grid != nullptr){
        free( grid->point_x.stream );
        free( grid->point_y.stream );
        free( grid->point_z.stream );
        grid->point_x.length = 0;
        grid->point_y.length = 0;
        grid->point_z.length = 0;
        grid->num_points = 0;

        free_sections( grid );
    }
}

/* Releases memory */
void cgnsGrid_free( cgnsGrid* grid, const int num_grids )
{
    int i;

    if (grid != nullptr){
        for (i = 0; i < num_grids; i++){
            dispose_grid( &(grid[i]) );
        }
        free( grid );
    }
}

