/***
Author: Mario J. Martin <dominonurbs$gmail.com>

This module implements methods related with camera transformations and views.
It is somehow an extension of of the math module.

*******************************************************************************/

#include <math.h>

#include "defines.h"
#include "gwmath.h"
#include "glw.h"


static gwMatrix4f calculate_transformation_axis( const gwCamera camera )
{
    gwMatrix4f temp, matrix;

    matrix = gwMatrix4f_identity();

    temp = gwMatrix4f_rotation( camera.rotation_angles.z, 1.0f, 0.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera.rotation_angles.x, 0.0f, 0.0f, 1.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera.rotation_angles.y, 0.0f, -1.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    return matrix;
}

static gwMatrix4f calculate_transformation_cad( const gwCamera camera )
{
    gwMatrix4f matrix, temp;

    matrix = gwMatrix4f_identity();
    temp = gwMatrix4f_translation( 0.0f, 0.0f, -camera.nearClip );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_scale
        ( camera.scale.x * camera.zoom
        , camera.scale.y * camera.zoom
        , (camera.scale.z * camera.zoom) / camera.farClip
        );

    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera.rotation_angles.z, 1.0f, 0.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera.rotation_angles.x, 0.0f, 0.0f, 1.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera.rotation_angles.y, 0.0f, -1.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_translation
        ( camera.eye.x, camera.eye.y, camera.eye.z );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    return matrix;
}

static gwMatrix4f calculate_transformation_lookat( const gwCamera camera )
{
    //gwVector3f target = { 1, 0, 0 };
    //gwMatrix4f rot_matrix = gwMatrix4f_transform
    //    ( 0, 0, 0
    //    , camera.rotation_angles.x
    //    , camera.rotation_angles.y
    //    , camera.rotation_angles.z 
    //    , 1, 1, 1
    //    );
    //target = gwVector3f_mul( &rot_matrix, &target );
    //target.x += camera.eye.x;
    //target.y += camera.eye.y;
    //target.z += camera.eye.z;

    gwMatrix4f matrix = gwMatrix4f_lookat
        ( camera.eye.x, camera.eye.y, camera.eye.z
        , camera.target.x, camera.target.y, camera.target.z
        , camera.lookup.x, camera.lookup.y, camera.lookup.z
        );

    return matrix;
}

static gwMatrix4f calculate_transformation_blit
( const gwCamera camera, const int screen_width, const int screen_height )
{
    gwMatrix4f matrix = gwMatrix4f_identity();
    matrix.data[0] = (gwFloat)(2.0 / screen_height);
    matrix.data[5] = -(gwFloat)(2.0 / screen_height);
    matrix.data[10] = (gwFloat)0.999;
    matrix.data[12] = -(gwFloat)screen_width / screen_height;
    matrix.data[13] = -1;

    return matrix;
}

/* Calculates the transformation matrix based on the position, rotation, ... */
extern "C"
gwMatrix4f gwCamera_calculate_matrix
( const gwCamera* camera, const int screen_width, const int screen_height )
{
    gwMatrix4f camera_matrix;

    /* Calculate the view matrix */
    if ((camera->type & GW_CAMERA_VIEW_CAD) != 0){
        camera_matrix = calculate_transformation_cad( *camera );
    }
    else if ((camera->type & GW_CAMERA_VIEW_AXIS) != 0){
        camera_matrix = calculate_transformation_axis( *camera );
    }
    else if ((camera->type == GW_CAMERA_VIEW_BLIT) != 0){
        camera_matrix = calculate_transformation_blit
            ( *camera, screen_width, screen_height );
    }
    else if ((camera->type == GW_CAMERA_VIEW_LOOKAT) != 0){
        camera_matrix = calculate_transformation_lookat( *camera );
    }
    else{
        camera_matrix = gwMatrix4f_identity();
    }

    /* Adds perspective */
    if ((camera->type & GW_CAMERA_VIEW_FLAG_PERSPECTIVE) != 0){
        gwMatrix4f fustrum = gwMatrix4f_perspective
            ( camera->field_angle, 1, camera->nearClip, camera->farClip );
        camera_matrix = gwMatrix4f_mul( &fustrum, &camera_matrix );
    }

    /* Correct the screen aberration */
    if ((camera->type & GW_CAMERA_VIEW_FLAG_NO_SCREEN_RATIO) == 0){
        const double screen_ratio = (double)screen_height / screen_width;
        gwMatrix4f mscale = gwMatrix4f_scale( screen_ratio, 1, 1 );
        camera_matrix = gwMatrix4f_mul( &mscale, &camera_matrix );
    }

    return camera_matrix;
}

/* Inverts the transformation of the camera to calculate
* the displacement in screen coordinates to world coordinates.
* The displacement in screen coordinates must be unitary (0,1); 
* in other worlds, divide the displacement in pixels by the screen height */
extern "C"
gwVector3f gwCamera_calculate_cad_displacement
( const gwCamera* camera, const gwFloat dx, const gwFloat dy, const gwFloat dz )
{
    /* Performs the same rotations, but in inverse order */
    gwMatrix4f temp;
    gwFloat iz = (gwFloat)1 / camera->zoom;
    gwFloat ix = iz * (gwFloat)1 / camera->scale.x;
    gwFloat iy = iz * (gwFloat)1 / camera->scale.y;

    gwMatrix4f matrix = gwMatrix4f_identity();

    temp = gwMatrix4f_rotation( camera->rotation_angles.y, 0.0f, 1.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera->rotation_angles.x, 0.0f, 0.0f, -1.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera->rotation_angles.z, -1.0f, 0.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( 180.0f, 0.0f, 1.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_scale( ix, iy, iz * camera->farClip );
    matrix = gwMatrix4f_mul( &matrix, &temp );
    
    gwVector3f vec = { dx, dy, dz };
    gwVector3f delta = gwVector3f_mul( &matrix, &vec );

    return delta;
}

/******************************************************************************/

template <class mgfloat>
static inline
mgfloat mg_reflection_delta( const mgfloat nx, const mgfloat ny, const mgfloat nz )
{
    return nx * nx * nx + nx * (ny * ny + nz * nz);
}

template <class mgfloat>
static inline mgfloat mg_reflection_x( const mgfloat nx, const mgfloat ny, const mgfloat nz
    , const mgfloat ax, const mgfloat ay, const mgfloat az
    , const mgfloat mx, const mgfloat my, const mgfloat mz )
{
    mgfloat r = ax * nx * nx * nx;
    r += (ay - my) * nx * nx * ny;
    r += (az - mz) * nx * nx * nz;
    r += mx * nx * (ny * ny + nz * nz);

    return r;
}

template <class mgfloat>
static inline mgfloat mg_reflection_y( const mgfloat nx, const mgfloat ny, const mgfloat nz
    , const mgfloat ax, const mgfloat ay, const mgfloat az
    , const mgfloat mx, const mgfloat my, const mgfloat mz )
{
    mgfloat r = my * nx * nx * nx;
    r += (ax - mx) * nx * nx * ny;
    r += (az - mz) * nx * ny * nz;
    r += ay * nx * ny * ny;
    r += my * nx * nz * nz;

    return r;
}

template <class mgfloat>
static inline mgfloat mg_reflection_z( const mgfloat nx, const mgfloat ny, const mgfloat nz
    , const mgfloat ax, const mgfloat ay, const mgfloat az
    , const mgfloat mx, const mgfloat my, const mgfloat mz )
{
    mgfloat r = mz * nx * nx * nx;
    r += (ax - mx) * nx * nx * nz;
    r += (ay - my) * nx * ny * nz;
    r += az * nx * nz * nz;
    r += mz * nx * ny * ny;

    return r;
}

/* Function to calculate the reflection point R in a flat mirror
A       C       R
*-------*.......*
 \      |      /
   \    |N   /
     \  |  /
       \|/
==================
*/
template <class mgVector3, class mgfloat>
static inline
mgVector3 mirror_reflection
( const mgVector3 &normal			/* Normal of the mirror (it is not neccesary to be normalized) */
, const mgVector3 &camera			/* Position of the camera */
, const mgVector3 &mirror			/* The position of the mirror the camera is looking at */
, const mgfloat threshold = 0.001f	/* Threshold for the normal; if it is normalized this is not neccesary. And usually is fine with its default value */
)
{
    mgVector3 point;    // The reflected point
    mgfloat delta, rx, ry, rz, temp;
    int shift = 0;
    mgfloat ax = camera.x;
    mgfloat ay = camera.y;
    mgfloat az = camera.z;
    mgfloat mx = mirror.x;
    mgfloat my = mirror.y;
    mgfloat mz = mirror.z;
    mgfloat nx = normal.x;
    mgfloat ny = normal.y;
    mgfloat nz = normal.z;

    if (nx < threshold && nx > -threshold){
        // shift coordinates
        shift++;
        ax = camera.y;
        ay = camera.z;
        az = camera.x;
        mx = mirror.y;
        my = mirror.z;
        mz = mirror.x;
        nx = normal.y;
        ny = normal.z;
        nz = normal.x;
    }

    if (nx < threshold && nx > -threshold){
        // shift coordinates
        shift++;
        ax = camera.z;
        ay = camera.x;
        az = camera.y;
        mx = mirror.z;
        my = mirror.x;
        mz = mirror.y;
        nx = normal.z;
        ny = normal.x;
        nz = normal.y;
    }

    if (nx < threshold && nx > -threshold){
        // the normal is not well defined. We cannot calculate the reflection
        point.x = camera.x;
        point.y = camera.y;
        point.z = camera.z;
        return point;
    }

    delta = mg_reflection_delta( nx, ny, nz );
    rx = mg_reflection_x( nx, ny, nz, ax, ay, az, mx, my, mz ) / delta;
    ry = mg_reflection_y( nx, ny, nz, ax, ay, az, mx, my, mz ) / delta;
    rz = mg_reflection_z( nx, ny, nz, ax, ay, az, mx, my, mz ) / delta;

    if (shift == 1){
        temp = rx;
        rx = rz;
        rz = ry;
        ry = temp;
    }
    else if (shift == 2){
        temp = rx;
        rx = ry;
        ry = rz;
        rz = temp;
    }

    point.x = 2 * rx - camera.x;
    point.y = 2 * ry - camera.y;
    point.z = 2 * rz - camera.z;

    return point;
}
/* Calculates the raytrace of the reflected point in a flat mirror */
gwVector3f gwCamera_reflection
( const gwVector3f* normal  /* Normal of the mirror */
, const gwVector3f* camera  /* Position of the camera */
, const gwVector3f* mirror  /* The position of the mirror. */
)
{
    return mirror_reflection<gwVector3f, float>( *normal, *camera, *mirror );
}

/* Intersection of a line and plane

     --------  /{a,b,c}
Q    \       \/
*--------x····\----> {Nx, Ny, Nz}
       \ R     \
        --------*P

The equations of the plane are in the form
a*(x-Px)+b*(y-Py)+c*(z-Pz) = 0

The equations of the line are in the form
(x-Qx)/Nx = (y-Qy)/Ny = (z-Qz)/Nz
*/
template <class mgVector3, class mgfloat>
static inline
mgVector3 calculateIntersectionLinePlane( const mgVector3& line_direction
    , const mgVector3& line_point
    , const mgVector3& plane_normal
    , const mgVector3& plane_point
    )
{
    mgVector3 r;

    mgfloat nx = line_direction.x;
    mgfloat ny = line_direction.y;
    mgfloat nz = line_direction.z;

    mgfloat qx = line_point.x;
    mgfloat qy = line_point.y;
    mgfloat qz = line_point.z;

    mgfloat a = plane_normal.x;
    mgfloat b = plane_normal.y;
    mgfloat c = plane_normal.z;

    mgfloat px = plane_point.x;
    mgfloat py = plane_point.y;
    mgfloat pz = plane_point.z;

    mgfloat t = (a * px + b * py + c * pz - a * qx - b * qy - c * qz);
    t /= (a * nx + b * ny + c * nz);

    r.x = qx + nx * t;
    r.y = qy + ny * t;
    r.z = qz + nz * t;

    return r;
}

gwVector3f calculateIntersectionLinePlane
( const gwVector3f* line_direction
    , const gwVector3f* line_point
    , const gwVector3f* plane_normal
    , const gwVector3f* plane_point
    )
{
    return calculateIntersectionLinePlane<gwVector3f, float>
        ( *line_direction, *line_point, *plane_normal, *plane_point );
}
