/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Data structure to represent TAU's primary grid
It is designed to easy plug the pointers from TAU.

*******************************************************************************/

#ifndef DSTAUPRIMGRID_H
#define DSTAUPRIMGRID_H

#include "dataset/dataset_arrays.h"

/* In unstructured grids, elements of the grid, such as tetrahedrons, are
* defined as a list of connectivities between grid vertices
* Bonundaries are defined with markers associated to surface elements:
* triangles and quads. */
typedef struct _owTauGrid
{
    /** Typically the original filename of the grid */
    char label[1024];

    /** Vertex of the primary grid. */
    owVector3dStream points;

    /** Surface triangle elements. */
    owIntStream surface_tri3;

    /** Surface quadrilateral elements */
    owIntStream surface_quad4;

    /** Volumetric tetrahedron elements. */
    owIntStream tetrahedrons4;

    /** Volumetric pyramid connectivities. */
    owIntStream pyramids5;

    /** Volumetric prisms connectivities. */
    owIntStream prisms6;

    /** Volumetric hexahedron elements. */
    owIntStream hexaheders8;

    /** Markers of the surface triangles employed by NETCDF unstructured grids,
    * associated with panels and boundary conditions. */
    owIntStream marker_triangles;

    /** Markers of the surface quads employed by NETCDF unstructured grids,
    * associated with panels and boundary conditions. */
    owIntStream marker_quads;

} owTauGrid;

#ifdef  __cplusplus
extern "C" {
#endif

    owTauGrid* owTauGrid_create();
    void owTauGrid_free( owTauGrid* obj );

#ifdef  __cplusplus
}
#endif

#endif /* DSTAUPRIMGRID_H */
