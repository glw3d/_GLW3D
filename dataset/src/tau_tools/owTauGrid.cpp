/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Functions to handle Tau data structure

*******************************************************************************/

#include <stdlib.h>

#include "common/check_malloc.h"
#include "owTauGrid.h"

struct CTauPrimGrid : owTauGrid
{
    inline void init()
    {
        label[0] = '\0';

        points.stream = nullptr;
        points.length = 0;

        surface_tri3.stream = nullptr;
        surface_tri3.length = 0;

        surface_quad4.stream = nullptr;
        surface_quad4.length = 0;

        tetrahedrons4.stream = nullptr;
        tetrahedrons4.length = 0;

        pyramids5.stream = nullptr;
        pyramids5.length = 0;

        prisms6.stream = nullptr;
        prisms6.length = 0;

        hexaheders8.stream = nullptr;
        hexaheders8.length = 0;

        marker_triangles.stream = nullptr;
        marker_triangles.length = 0;

        marker_quads.stream = nullptr;
        marker_quads.length = 0;
    }

    CTauPrimGrid()
    {
        init();
    }

    ~CTauPrimGrid()
    {
        free( points.stream );
        free( surface_tri3.stream );
        free( surface_quad4.stream );
        free( tetrahedrons4.stream );
        free( pyramids5.stream );
        free( prisms6.stream );
        free( hexaheders8.stream );
        if (marker_triangles.stream != nullptr){
            free( marker_triangles.stream );
        }
        else{
            free( marker_quads.stream );
        }

        points.stream = nullptr;
        points.length = 0;

        surface_tri3.stream = nullptr;
        surface_tri3.length = 0;

        surface_quad4.stream = nullptr;
        surface_quad4.length = 0;

        tetrahedrons4.stream = nullptr;
        tetrahedrons4.length = 0;

        pyramids5.stream = nullptr;
        pyramids5.length = 0;

        prisms6.stream = nullptr;
        prisms6.length = 0;

        hexaheders8.stream = nullptr;
        hexaheders8.length = 0;

        marker_triangles.stream = nullptr;
        marker_triangles.length = 0;

        marker_quads.stream = nullptr;
        marker_quads.length = 0;
    }
};

extern "C"
owTauGrid* owTauGrid_create()
{
    return new CTauPrimGrid;
}

extern "C"
void owTauGrid_free( owTauGrid* obj )
{
    if (obj != nullptr){
        CTauPrimGrid* grid = (CTauPrimGrid*)obj;
        delete grid;
    }
}
