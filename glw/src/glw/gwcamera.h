/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Implements the different camera transformations, when update is called.

The screen camera is just the identity.
The CAD camera is an ortographic transformation (without perception of depth)

*******************************************************************************/

#ifndef _HGW_CAMERA
#define _HGW_CAMERA

#include "gwdata.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Calculates the transformation matrix based on the position, rotation, ... */
gwMatrix4f gwCamera_calculate_matrix
( const gwCamera* camera, const int screen_width, const int screen_height );

/* Inverts the transformation of the camera to calculate
 * the displacement in screen coordinates to world coordinates */
gwVector3f gwCamera_calculate_cad_displacement
( const gwCamera* camera, const gwFloat dx, const gwFloat dy, const gwFloat dz );

/* Autoadjust the view based on the entities attached to the camera */
void gwCamera_autofit( gwCamera* camera );

/* Assign the camera callbacks inputs fot CAD-like applications */
void gwCamera_behaviour_cad( gwCamera* camera );

/* Calculates the raytrace of the reflected point in a flat mirror */
gwVector3f gwCamera_reflection
( const gwVector3f* normal  /* Normal of the mirror */
, const gwVector3f* camera  /* Position of the camera */
, const gwVector3f* mirror  /* The position of the mirror. */
);

/* Calculates the intersection of a line with a plane */
gwVector3f calculateIntersectionLinePlane
( const gwVector3f* line_direction
, const gwVector3f* line_point
, const gwVector3f* plane_normal
, const gwVector3f* plane_point
);

#if defined(__cplusplus)
}
#endif

#endif /* _HGW_CAMERA */
