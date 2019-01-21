/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Some basic geometries.
There are usually in two modalities: shape (solid/surface) and wireframe. 
*******************************************************************************/

#ifndef _GW_SHAPES_H
#define _GW_SHAPES_H

#include "gwdata.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Creates a 1 by 1 quad */
gwMesh* gwShape_quad( const gwFloat size );
gwMesh* gwWireframe_quad( const gwFloat size );

/* Creates a [0,1] quad.
* This is convenient to render to the full screen without any transformation.
* Used for postprocessing and multi-pass techniques. */
extern "C"
gwMesh* gwShape_quad01();

/* Creates a quad for blitting, indicating the dimensions in pixels. */
gwMesh* gwShape_sprite( const int width, const int height );
gwMesh* gwWireframe_sprite( const int width, const int height );

/* Creates an icosahedron of radius 1 */
gwMesh* gwShape_icosahedron();

/* Creates a cube of size 1x1x1 */
gwMesh* gwShape_cube();
gwMesh* gwWireframe_cube();

/* Creates an sphere of radius 1 */
gwMesh* gwShape_sphere( const int refinement );

/* Creates a tile grid. Useful for a ground, e.g. */
gwMesh* gwShape_tiles( const int width, const int height );

/* Creates a cloud of points */
gwMesh* gwShape_cloudpoint( const int number_points );

/* Creates a line connecting points */
gwMesh* gwShape_line( const int number_points );

/* Creates a cilinder, or a cone if one of the top_base_ratio is zero */
gwMesh* gwShape_cilinder
( const int number_points
, const double top_base_ratio
, const double angle_offset
);

/* Loads a shape from a obj file format */
gwMesh* gwShapeLoad_obj( const char* filename );

/* Loads a shape from a stl file format */
gwMesh* gwShapeLoad_stl( const char* filename );

#if defined(__cplusplus)
}
#endif

#endif /*_GW_SHAPES_H*/
