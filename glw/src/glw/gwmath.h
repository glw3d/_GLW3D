/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Implements the matrix operations as were originally defined in GLU.
These are used for the camera, which is basically a 4x4 tranformation matrix.

*******************************************************************************/

#ifndef GW_MATH_H
#define GW_MATH_H

#include "gwdata.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Normalizes a vector vec = {x, y, z} */
gwVector3f gwVector3f_norm( const gwVector3f* vec );

/* Provides the inverse of the module of the vector vec = {x, y, z} */
gwFloat gwVector3f_imod( const gwVector3f* vec );

/* Provides the module of the vector vec = {x, y, z} */
gwFloat gwVector3f_mod( const gwVector3f* vec );

/* Provides the square module of the vector vec = {x, y, z} */
gwFloat gwVector3f_mod2( const gwVector3f* vec );

/* Multiplies a matrix and a vector.
The source code has been taken from the original GLU
http://www.opengl.org/wiki/GluProject_and_gluUnProject_code
*/
gwVector3f gwVector3f_mul
    ( const gwMatrix4f* matrix
    , const gwVector3f* vec3
    );

/* Computes the cross product between two vectors */
gwVector3f gwVector3f_cross( const gwVector3f* va, const gwVector3f* vb );

/* Computes the dot product between vectors */
gwFloat gwVector3f_dot( const gwVector3f* va, const gwVector3f* vb );

/* Computes the difference between two vectors */
gwVector3f gwVector3f_sub( const gwVector3f* va, const gwVector3f* vb );

gwMatrix4f gwMatrix4f_mul
    ( const gwMatrix4f* matrix1, const gwMatrix4f* matrix2 );

/*  Returns the identity matrix */
gwMatrix4f gwMatrix4f_identity();

/*  Loads this transformation matrix
|x^2(1-c)+c   xy(1-c)-zs   xz(1-c)+ys   0|
|yx(1-c)+zs   y^2(1-c)+c   yz(1-c)-xs   0|
|xz(1-c)-ys   yz(1-c)+xs   z^2(1-c)+c   0|
|0            0            0            1|
*/
gwMatrix4f gwMatrix4f_rotation
    ( const double angle_degrees
    , const double axis_x
    , const double axis_y
    , const double axis_z
    );

/*  Loads this transformation matrix
|1 0 0 0|
|0 1 0 0|
|0 0 1 0|
|x y z 1|
*/
gwMatrix4f gwMatrix4f_translation
    ( const double x
    , const double y
    , const double z
    );


/*  Performs this transformation matrix
|x 0 0 0|
|0 y 0 0|
|0 0 z 0|
|0 0 0 1|
*/
gwMatrix4f gwMatrix4f_scale
    ( const double x
    , const double y
    , const double z
    );


/* Loads a transformation matrix.
The rotation defined by the Euler angles in Z Y' X'' sequence.
They are the same as the nautical angles.
|  c1搾2*sx   c1新2新3-c3新1       s1新3+c1搾3新2  tx  |
|  c2新1      (c1搾3+s1新2新3)*sy  c3新1新2-c1新3  ty  |
|  -s2        c2新3                c2搾3*sz        tz  |
|  0          0                    0               1   |

This is equivalent to perform a rotation first and then a translation.
Performing a translation and then a rotation obtains a different result.
*/
gwMatrix4f gwMatrix4f_transform
    ( const double position_x, const double position_y, const double position_z
    , const double yaw, const double pitch, const double roll
    , const double scale_x, const double scale_y, const double scale_z
    );

/* This is extracted from the original gluLookAt source code
http://www.opengl.org/wiki/GluLookAt_code
*/
gwMatrix4f gwMatrix4f_lookat
    ( const double eye_x, const double eye_y, const double eye_z
    , const double target_x, const double target_y, const double target_z
    , const double lookup_x, const double lookup_y, const double lookup_z
    );

/* This works exactly the same as the original glOrtho2D
* Notice that Ortho2D(left, right, bottom, top) is equivalent to
* Ortho(left, right, bottom, top, -1.0, 1.0)*/
gwMatrix4f gwMatrix4f_ortho2d
    ( const double left
    , const double right
    , const double botton
    , const double top
    );


/* This works exactly the same as the original glOrtho */
gwMatrix4f gwMatrix4f_ortho
    ( const double left
    , const double right
    , const double botton
    , const double top
    , const double nearClip
    , const double farClip
    );


/* This is extracted from the original gluPerspective source code
http://www.opengl.org/wiki/GluPerspective_code
*/
gwMatrix4f gwMatrix4f_fustrum
    ( const double left
    , const double right
    , const double bottom
    , const double top
    , const double nearClip
    , const double farClip
    );

/* This is extracted from the original glFustrum source code
http://www.opengl.org/wiki/GluPerspective_code
*/
gwMatrix4f gwMatrix4f_perspective
    ( const double field_angle /* in degrees */
    , const double aspect
    , const double fnear
    , const double ffar
    );

/* Invertts a 4x4 matrix.*/
gwMatrix4f gwMatrix4f_invert( const gwMatrix4f matrix );

#if defined(__cplusplus)
}
#endif

#endif /*GW_MATH_H*/
