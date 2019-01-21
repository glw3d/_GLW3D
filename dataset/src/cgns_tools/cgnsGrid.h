/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Data structure designed to deal with grids in CGNS file format,
either unstructured and structured.

*******************************************************************************/

#ifndef DSCGNSGRID_H
#define DSCGNSGRID_H

#include "dataset/dataset_arrays.h"

typedef enum {
    owGridType_CGNS_UNKOWN = 0,
    owGridType_CGNS_STRUCT = 1,
    owGridType_CGNS_UNSTRUCT = 2,
}cgnsGridType;

/* In a section the elements of the grid that defines the connectivities
* of the vertices are defined, such as triangles, tetrahedrons, etc...
*
* Unstructures CGNS grid are divided into several sections and all elements are
* associated with a consecutive unique element index number.
*/
typedef struct _cgnsSection
{
    /** Description, short name, or just quick information */
    char label[1024];

    /** In wireframe grids is the number of lines */
    int num_lines;

    /** In unstructured grids is the number of triangle elements.
    In structured grids, the value is 0. */
    int num_triangles;

    /** In unstructured grids is the number of quadrilateral elements
    * In structured grids, the value is 0. */
    int num_quads;

    /** Number of tetrahedrons */
    int num_tetrahedrons;

    /** Number of pyramids */
    int num_pyramids;

    /** Number of pentahedrons */
    int num_pentahedrons;

    /** Number of hexahedrons */
    int num_hexahedrons;

    /*
    In CGNS, elements can be defined with additional vertices,
    although it is very rare.

    Interpolat.  linear      quadratic           cubic

    Line:        BAR_2       BAR_3               BAR_4
    Triangle:    TRI_3       TRI_6               TRI_9, TRI_10
    Quadrangle:  QUAD_4      QUAD_8, QUAD_9      QUAD_12, QUAD_16
    Tetrahedron: TETRA_4     TETRA_10            TETRA_16, TETRA_20
    Pyramid:     PYRA_5      PYRA_13, PYRA_14    PYRA_21, PYRA_29, PYRA_30
    Pentahedron: PENTA_6     PENTA_15, PENTA_18  PENTA_24, PENTA_38, PENTA_40
    Hexahedron:  HEXA_8      HEXA_20, HEXA_27    HEXA_32, HEXA_56, HEXA_64

    Additional nodes are located at the middle of the edges or
    the interior of the elements.
    */

    /* 1-D */
    int* bar2_stream;  /**< Lines or edges connectivities. */

    /* Surface elements */
    int* tri3_stream;  /**< Triangle indices connectivities for surface. */
    int* quad4_stream; /**< Quadrilaterals indices connectivities for surface. */

    /* Volumetric elements */
    int* tetra4_stream; /**< Tetrahedron connectivities. */
    int* pyra5_stream;  /**< Pyramid connectivities. */
    int* penta6_stream; /**< Pentahedron connectivities. */
    int* hexa8_stream;  /**< Hexahedron connectivities. */

    /** In arrays, targets the next element. */
    struct _cgnsSection* next;

} cgnsSection;

typedef struct _cgnsGrid
{
    /** Description, short name, or just quick information */
    char label[1024];

    /** Identifier numeric value */
    int id;

    /** An attribute in CGNS; a geometry can be splited into several grids */
    int base;

    /** A CGNS grid can be splitted into several zones.
    * In Overview each zone is an independent grid. */
    int zone;

    /** unstructured or structured */
    cgnsGridType type;

    /** In structured grids is the number of vertices in each dimension */
    int i_size, j_size, k_size;

    /** Especifies if the original grid is 1-d, 2-d, or 3-d.
    * This is only informative; for simplicity, grids are converted and loaded
    * as 3-dimensional entities anyway. */
    int dim;

    size_t num_points;  /**< Total number of vertices of the grid */

    /* Unlike TAU, in CGNS the coordinates of the vertices are stored in separated streams */
    owDoubleStream point_x;  /**< Coordinates of the vertices */
    owDoubleStream point_y;  /**< Coordinates of the vertices */
    owDoubleStream point_z;  /**< Coordinates of the vertices */

    /* In CGNS unstructured grids, the elements are defined in sections */
    cgnsSection* section;

    /* Number of unstructured sections. In structured grids, there are no sections */
    size_t num_sections;

    /** In arrays of grids targets to the next grid, or in CGNS the next zone */
    struct _cgnsGrid* next;
} cgnsGrid;

/* Releases memory */
#ifdef  __cplusplus
extern "C" {
#endif
    void cgnsGrid_free( cgnsGrid* grid, const int num_grids );
#ifdef  __cplusplus
}
#endif

#endif /* DSCGNSGRID_H */
