/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Implements the camera operations as were originally defined in GLU.
The camera is basically a 4x4 tranformation matrix.

*******************************************************************************/

#include <memory.h>
#include <math.h>

#include "gwdata.h"


template < typename T >
static inline T isqrt( T x )
{
    return (T)(1. / sqrt( (double)x ));
}

/* This is just a quriosity that I found in Internet.
* Calculates the inverse of the square root
* with a precission of 12 digits employing
* the magic number 0x5f375a86.
* There is also a 64 bit version. */
static inline float isqrt( float x )
{
    float xhalf = 0.5f * x;
    void* bridge = &x;              // This avoids the GCC warning "type-punned"
    int32_t i = *(int32_t*)bridge;  // get bits for floating value
    i = 0x5f375a86 - (i >> 1);      // gives initial guess y0
    bridge = &i;
    x = *(float*)bridge;            // convert bits back to float
    x = x * (1.5f - xhalf * x * x); // Newton step, repeating increases accuracy

    return x;
}

/* Normalizes a vector vec = {x, y, z} */
extern "C"
gwVector3f gwVector3f_norm( const gwVector3f* const vec )
{
    register gwVector3f result = *vec;
    float m2 = result.x * result.x
        + result.y * result.y
        + result.z * result.z;

    float imod = isqrt< gwFloat >( m2 );

    result.x *= imod;
    result.y *= imod;
    result.z *= imod;

    return result;
}

/* Provides the inverse of the module of the vector vec = {x, y, z} */
extern "C"
gwFloat gwVector3f_imod( const gwVector3f* const vec )
{
    register gwVector3f result = *vec;
    float m2 = result.x * result.x
        + result.y * result.y
        + result.z * result.z;

    float imod = isqrt< gwFloat >( m2 );

    return imod;
}

/* Provides the module of the vector vec = {x, y, z} */
extern "C"
gwFloat gwVector3f_mod( const gwVector3f* const vec )
{
    register gwVector3f result = *vec;
    float m2 = result.x * result.x
        + result.y * result.y
        + result.z * result.z;

    float mod = (float)sqrt( m2 );

    return mod;
}

/* Provides the square module of the vector vec = {x, y, z} */
extern "C"
gwFloat gwVector3f_mod2( const gwVector3f* const vec )
{
    register gwVector3f result = *vec;
    float m2 = result.x * result.x
        + result.y * result.y
        + result.z * result.z;

    return m2;
}

/* Multiplies a matrix and a vector.
The source code has been taken from the original GLU
http://www.opengl.org/wiki/GluProject_and_gluUnProject_code
*/
extern "C"
gwVector3f gwVector3f_mul
( const gwMatrix4f* const matrix, const gwVector3f* const vec3 )
{
    register const gwFloat v[3] = { vec3->x, vec3->y, vec3->z };
    const gwFloat* m = matrix->data;
    gwVector3f result;

    //#pragma omp parallel sections
    {
        //#pragma omp section
        {result.x = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12]; }
        //#pragma omp section
        {result.y = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13]; }
        //#pragma omp section
        {result.z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14]; }
    }

    return result;
}

/* Computes the cross product between two vectors */
extern "C"
gwVector3f gwVector3f_cross( const gwVector3f* const va, const gwVector3f* const vb )
{
    register const gwFloat a[3] = { va->x, va->y, va->z };
    register const gwFloat b[3] = { vb->x, vb->y, vb->z };
    gwVector3f result;

    result.x = a[1] * b[2] - a[2] * b[1]; 
    result.y = a[2] * b[0] - a[0] * b[2]; 
    result.z = a[0] * b[1] - a[1] * b[0]; 

    return result;
}

/* Computes the difference between two vectors */
extern "C"
gwVector3f gwVector3f_sub( const gwVector3f* const va, const gwVector3f* const vb )
{
    gwVector3f result;

    result.x = va->x - vb->x;
    result.y = va->y - vb->y;
    result.z = va->z - vb->z;

    return result;
}

/*  Computes the dot product between vectors */
extern "C"
gwFloat gwVector3f_dot( const gwVector3f* const va, const gwVector3f* const vb )
{
    register const gwFloat a[3] = { va->x, va->y, va->z };
    register const gwFloat b[3] = { vb->x, vb->y, vb->z };

    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}


extern "C"
gwMatrix4f gwMatrix4f_mul
( const gwMatrix4f* const matrix1, const gwMatrix4f* const matrix2 )
{
    register gwFloat m1[16];
    register gwFloat m2[16];
    memcpy( m1, matrix1, sizeof( gwFloat ) * 16 );
    memcpy( m2, matrix2, sizeof( gwFloat ) * 16 );
    gwMatrix4f result;

    {
        {
            result.data[0] = m1[0] * m2[0]
                + m1[4] * m2[1]
                + m1[8] * m2[2]
                + m1[12] * m2[3];

            result.data[1] = m1[1] * m2[0]
                + m1[5] * m2[1]
                + m1[9] * m2[2]
                + m1[13] * m2[3];

            result.data[2] = m1[2] * m2[0]
                + m1[6] * m2[1]
                + m1[10] * m2[2]
                + m1[14] * m2[3];

            result.data[3] = m1[3] * m2[0]
                + m1[7] * m2[1]
                + m1[11] * m2[2]
                + m1[15] * m2[3];
        }
        {
            result.data[4] = m1[0] * m2[4]
                + m1[4] * m2[5]
                + m1[8] * m2[6]
                + m1[12] * m2[7];

            result.data[5] = m1[1] * m2[4]
                + m1[5] * m2[5]
                + m1[9] * m2[6]
                + m1[13] * m2[7];

            result.data[6] = m1[2] * m2[4]
                + m1[6] * m2[5]
                + m1[10] * m2[6]
                + m1[14] * m2[7];

            result.data[7] = m1[3] * m2[4]
                + m1[7] * m2[5]
                + m1[11] * m2[6]
                + m1[15] * m2[7];
        }
        {
            result.data[8] = m1[0] * m2[8]
                + m1[4] * m2[9]
                + m1[8] * m2[10]
                + m1[12] * m2[11];

            result.data[9] = m1[1] * m2[8]
                + m1[5] * m2[9]
                + m1[9] * m2[10]
                + m1[13] * m2[11];

            result.data[10] = m1[2] * m2[8]
                + m1[6] * m2[9]
                + m1[10] * m2[10]
                + m1[14] * m2[11];

            result.data[11] = m1[3] * m2[8]
                + m1[7] * m2[9]
                + m1[11] * m2[10]
                + m1[15] * m2[11];
        }
        {
            result.data[12] = m1[0] * m2[12]
                + m1[4] * m2[13]
                + m1[8] * m2[14]
                + m1[12] * m2[15];

            result.data[13] = m1[1] * m2[12]
                + m1[5] * m2[13]
                + m1[9] * m2[14]
                + m1[13] * m2[15];

            result.data[14] = m1[2] * m2[12]
                + m1[6] * m2[13]
                + m1[10] * m2[14]
                + m1[14] * m2[15];

            result.data[15] = m1[3] * m2[12]
                + m1[7] * m2[13]
                + m1[11] * m2[14]
                + m1[15] * m2[15];
        }
    }
    return result;
}

/*  Returns the identity matrix */
extern "C"
gwMatrix4f gwMatrix4f_identity()
{
    const gwFloat identity[] =
    { 1, 0, 0, 0
    , 0, 1, 0, 0
    , 0, 0, 1, 0
    , 0, 0, 0, 1
    };
    gwMatrix4f matrix;
    memcpy( matrix.data, identity, sizeof( gwFloat ) * 16 );

    return matrix;
}

/*  Loads this transformation matrix
|x^2(1-c)+c   xy(1-c)-zs   xz(1-c)+ys   0|
|yx(1-c)+zs   y^2(1-c)+c   yz(1-c)-xs   0|
|xz(1-c)-ys   yz(1-c)+xs   z^2(1-c)+c   0|
|0            0            0            1|
*/
extern "C"
gwMatrix4f gwMatrix4f_rotation
( const double angle_degrees
, const double axis_x
, const double axis_y
, const double axis_z
)
{
    const gwFloat pi = (gwFloat)3.14169265;

    /* Normalize the axis */
    gwFloat mod = gwFloat( axis_x * axis_x + axis_y * axis_y + axis_z * axis_z );
    gwFloat f = isqrt< gwFloat >( mod );
    gwFloat x = gwFloat( -axis_x * f );
    gwFloat y = gwFloat( -axis_y * f );
    gwFloat z = gwFloat( -axis_z * f );

    /* Get the angle in radians */
    double a = (pi * angle_degrees) / 180;
    gwFloat c = (gwFloat)cos( a );
    gwFloat s = (gwFloat)sin( a );
    gwFloat c1 = 1 - c;

    gwFloat tx = x * c1;
    gwFloat ty = y * c1;
    gwFloat tz = z * c1;

    gwMatrix4f matrix;

    // rotation matrix
    matrix.data[0] = x * tx + c;
    matrix.data[1] = x * ty - z*s;
    matrix.data[2] = x * tz + y*s;
    matrix.data[3] = 0;
    matrix.data[4] = y * tx + z*s;
    matrix.data[5] = y * ty + c;
    matrix.data[6] = y * tz - x*s;
    matrix.data[7] = 0;
    matrix.data[8] = z * tx - y*s;
    matrix.data[9] = z * ty + x*s;
    matrix.data[10] = z * tz + c;
    matrix.data[11] = 0;
    matrix.data[12] = 0;
    matrix.data[13] = 0;
    matrix.data[14] = 0;
    matrix.data[15] = 1;

    return matrix;
}

/*  Loads this transformation matrix
|1 0 0 0|
|0 1 0 0|
|0 0 1 0|
|x y z 1|
*/
extern "C"
gwMatrix4f gwMatrix4f_translation
( const double x
, const double y
, const double z
)
{
    const gwFloat identity[] =
    { 1, 0, 0, 0
    , 0, 1, 0, 0
    , 0, 0, 1, 0
    , gwFloat( x ), gwFloat( y ), gwFloat( z ), 1
    };
    gwMatrix4f matrix;

    memcpy( matrix.data, identity, sizeof( gwFloat ) * 16 );
    return matrix;
}

/*  Performs this transformation matrix
|x 0 0 0|
|0 y 0 0|
|0 0 z 0|
|0 0 0 1|
*/
extern "C"
gwMatrix4f gwMatrix4f_scale
( const double x
, const double y
, const double z
)
{
    const gwFloat temp[] =
    { gwFloat( x ), 0, 0, 0
    , 0, gwFloat( y ), 0, 0
    , 0, 0, gwFloat( z ), 0
    , 0, 0, 0, 1
    };
    gwMatrix4f matrix;

    memcpy( matrix.data, temp, sizeof( gwFloat ) * 16 );

    return matrix;
}

/* Loads a transformation matrix. 
   The rotation defined by the Euler angles in Z Y' X'' sequence.
   They are the same as the nautical angles.
|  c1搾2*sx   c1新2新3-c3新1       s1新3+c1搾3新2  tx  |
|  c2新1      (c1搾3+s1新2新3)*sy  c3新1新2-c1新3  ty  |
|  -s2        c2新3                c2搾3*sz        tz  |
|  tx         ty                   tz              1   |

This is equivalent to perform a rotation first and then a translation.
Performing a translation and then a rotation obtains a different result.
*/
extern "C"
gwMatrix4f gwMatrix4f_transform
( const double position_x, const double position_y, const double position_z
, const double roll, const double pitch, const double yaw
, const double scale_x, const double scale_y, const double scale_z
)
{
    const gwFloat pi = (gwFloat)3.14169265;

    gwFloat sx = (gwFloat)scale_x;
    gwFloat sy = (gwFloat)scale_y;
    gwFloat sz = (gwFloat)scale_z;

    double a1 = (pi * yaw) / 180;
    gwFloat c1 = (gwFloat)cos( a1 );
    gwFloat s1 = (gwFloat)sin( a1 );

    double a2 = (pi * pitch) / 180;
    gwFloat c2 = (gwFloat)cos( a2 );
    gwFloat s2 = (gwFloat)sin( a2 );

    double a3 = (pi * roll) / 180;
    gwFloat c3 = (gwFloat)cos( a3 );
    gwFloat s3 = (gwFloat)sin( a3 );

    gwMatrix4f matrix;

    matrix.data[0] = c1*c2 * sx;          
    matrix.data[1] = (c1*s2*s3 - c3*s1) * sx;  
    matrix.data[2] = (s1*s3 + c1*c3*s2) * sx; 
    matrix.data[3] = 0;
    matrix.data[4] = c2*s1 * sy;            
    matrix.data[5] = (c1*c3 + s1*s2*s3) * sy;
    matrix.data[6] = (c3*s1*s2 - c1*s3) * sy;        
    matrix.data[7] = 0;
    matrix.data[8] = -s2 * sz;           
    matrix.data[9] = c2*s3 * sz;          
    matrix.data[10] = c2*c3 * sz;        
    matrix.data[11] = 0;
    matrix.data[12] = (gwFloat)position_x;
    matrix.data[13] = (gwFloat)position_y;
    matrix.data[14] = (gwFloat)position_z;
    matrix.data[15] = 1;

    return matrix;
}

/* This is extracted from the original gluLookAt source code
http://www.opengl.org/wiki/GluLookAt_code
*/
extern "C"
gwMatrix4f gwMatrix4f_lookat
( const double eye_x, const double eye_y, const double eye_z
, const double target_x, const double target_y, const double target_z
, const double lookup_x, const double lookup_y, const double lookup_z
)
{
    const gwVector3f lookup =
    { gwFloat( lookup_x )
    , gwFloat( lookup_y )
    , gwFloat( lookup_z )
    };

    const gwVector3f eye =
    { gwFloat( eye_x )
    , gwFloat( eye_y )
    , gwFloat( eye_z )
    };

    gwVector3f forward =
    { gwFloat( target_x - eye_x )
    , gwFloat( target_y - eye_y )
    , gwFloat( target_z - eye_z )
    };

    gwVector3f side, up;
    gwMatrix4f matrix;

    /* Normalize */
    forward = gwVector3f_norm( &forward );

    /* Side = forward x up */
    side = gwVector3f_cross( &forward, &lookup );
    side = gwVector3f_norm( &side );

    /* Recompute up as: up = side x forward */
    up = gwVector3f_cross( &side, &forward );
    up = gwVector3f_norm( &up );

    /*------------------*/
    matrix.data[0] = side.x;
    matrix.data[4] = side.y;
    matrix.data[8] = side.z;
    matrix.data[12] = 0;
    /*------------------*/
    matrix.data[1] = up.x;
    matrix.data[5] = up.y;
    matrix.data[9] = up.z;
    matrix.data[13] = 0;
    /*------------------*/
    matrix.data[2] = -forward.x;
    matrix.data[6] = -forward.y;
    matrix.data[10] = -forward.z;
    matrix.data[14] = 0;
    /*------------------*/
    matrix.data[3] = 0;
    matrix.data[7] = 0;
    matrix.data[11] = 0;
    matrix.data[15] = 1;

    /* Translate to the eye position */
    matrix.data[12] = -(matrix.data[0] * eye.x + matrix.data[4] * eye.y + matrix.data[8] * eye.z);
    matrix.data[13] = -(matrix.data[1] * eye.x + matrix.data[5] * eye.y + matrix.data[9] * eye.z);
    matrix.data[14] = -(matrix.data[2] * eye.x + matrix.data[6] * eye.y + matrix.data[10] * eye.z);

    return matrix;
}


/* This works exactly the same as the original glOrtho2D
* Notice that Ortho2D(left, right, bottom, top) is equivalent to
* Ortho(left, right, bottom, top, -1.0, 1.0)*/
extern "C"
gwMatrix4f gwMatrix4f_ortho2d
( const double left
, const double right
, const double botton
, const double top
)
{
    gwMatrix4f matrix;

    matrix.data[0] = gwFloat( 2. / (right - left) );  /* 0,0 */
    matrix.data[1] = 0;                   /* 1,0 */
    matrix.data[2] = 0;                   /* 2,0 */
    matrix.data[3] = 0;                   /* 3,0 */

    matrix.data[4] = 0;                   /* 1,0 */
    matrix.data[5] = gwFloat( 2. / (top - botton) );  /* 1,1 */
    matrix.data[6] = 0;                   /* 1,2 */
    matrix.data[7] = 0;                   /* 1,3 */

    matrix.data[8] = 0;                   /* 2,0 */
    matrix.data[9] = 0;                   /* 2,1 */
    matrix.data[10] = 1;                  /* 2,2 */
    matrix.data[11] = 0;                  /* 2,3 */

    matrix.data[12] = gwFloat( -(right + left) / (right - left) );   /* 3,0 */
    matrix.data[13] = gwFloat( (top + botton) / (top - botton) );    /* 3,1 */
    matrix.data[14] = 0;                  /* 3,2 */
    matrix.data[15] = 1;                  /* 3,3 */

    return matrix;
}


/* This works exactly the same as the original glOrtho */
extern "C"
gwMatrix4f gwMatrix4f_ortho
( const double left
, const double right
, const double botton
, const double top
, const double nearClip
, const double farClip
)
{
    gwMatrix4f matrix;

    matrix.data[0] = gwFloat( 2 / (left - right) ); /* 0,0 */
    matrix.data[1] = 0;                   /* 1,0 */
    matrix.data[2] = 0;                   /* 2,0 */
    matrix.data[3] = 0;                   /* 3,0 */

    matrix.data[4] = 0;                   /* 1,0 */
    matrix.data[5] = gwFloat( 2 / (top - botton) ); /* 1,1 */
    matrix.data[6] = 0;                   /* 1,2 */
    matrix.data[7] = 0;                   /* 1,3 */

    matrix.data[8] = 0;                   /* 2,0 */
    matrix.data[9] = 0;                   /* 2,1 */
    matrix.data[10] = gwFloat( -2 / (farClip - nearClip) );  /* 2,2 */
    matrix.data[11] = 0;                  /* 2,3 */

    matrix.data[12] = gwFloat( -(left + right) / (left - right) );   /* 3,0 */
    matrix.data[13] = gwFloat( (top + botton) / (top - botton) );    /* 3,1 */
    matrix.data[14] = gwFloat( -(farClip + nearClip) / (farClip - nearClip) ); /* 3,2 */
    matrix.data[15] = 1;    /* 3,3 */

    return matrix;
}


/* This is extracted from the original gluPerspective source code
http://www.opengl.org/wiki/GluPerspective_code
*/
extern "C"
gwMatrix4f gwMatrix4f_fustrum
( const double left
, const double right
, const double bottom
, const double top
, const double nearClip
, const double farClip
)
{
    gwMatrix4f matrix;

    matrix.data[0] = gwFloat( (2 * nearClip) / (right - left) );  /* 0,0 */
    matrix.data[1] = 0;                   /* 1,0 */
    matrix.data[2] = 0;                   /* 2,0 */
    matrix.data[3] = 0;                   /* 3,0 */

    matrix.data[4] = 0;                   /* 1,0 */
    matrix.data[5] = gwFloat( (2 * nearClip) / (top - bottom) );  /* 1,1 */
    matrix.data[6] = 0;                   /* 1,2 */
    matrix.data[7] = 0;                   /* 1,3 */

    matrix.data[8] = 0;                   /* 2,0 */
    matrix.data[9] = 0;                   /* 2,1 */
    matrix.data[10] = gwFloat( (farClip + nearClip) / (farClip - nearClip) );  /* 2,2 */
    matrix.data[11] = 1;                 /* 2,3 */

    matrix.data[12] = 0;                  /* 3,0 */
    matrix.data[13] = 0;                  /* 3,1 */
    matrix.data[14] = gwFloat( -(2 * farClip * nearClip) / (farClip - nearClip) );  /* 3,2 */
    matrix.data[15] = 1;                  /* 3,3 */

    return matrix;
}


/* This is extracted from the original glFustrum source code
http://www.opengl.org/wiki/GluPerspective_code
*/
extern "C"
gwMatrix4f gwMatrix4f_perspective
( const double field_angle /* in degrees */
, const double aspect
, const double fnear
, const double ffar
)
{
    double pi = 4 * atan( 1.0 );
    double range = fnear * tan( (pi / 180)*(field_angle / 2) );
    double left = range * aspect;
    double right = -(range * aspect);
    double bottom = -range;
    double top = range;

    return gwMatrix4f_fustrum( left, right, bottom, top, fnear, ffar );
}

/* Invertts the transformation matrix.
* Taken from INTEL's "Streaming SIMD Extensions - Inverse of 4x4 Matrix" */
extern "C"
gwMatrix4f gwMatrix4f_invert( const gwMatrix4f matrix )
{
    float tmp[12]; /* temp array for pairs */
    float src[16]; /* array of transpose source matrix */
    float det; /* determinant */
    gwMatrix4f inv;
    const float* mat = matrix.data;
    float* dst = inv.data;

    /* transpose matrix */
    for (int i = 0; i < 4; i++){
        src[i] = mat[i * 4];
        src[i + 4] = mat[i * 4 + 1];
        src[i + 8] = mat[i * 4 + 2];
        src[i + 12] = mat[i * 4 + 3];
    }

    /* calculate pairs for first 8 elements (cofactors) */
    tmp[0] = src[10] * src[15];
    tmp[1] = src[11] * src[14];
    tmp[2] = src[9] * src[15];
    tmp[3] = src[11] * src[13];
    tmp[4] = src[9] * src[14];
    tmp[5] = src[10] * src[13];
    tmp[6] = src[8] * src[15];
    tmp[7] = src[11] * src[12];
    tmp[8] = src[8] * src[14];
    tmp[9] = src[10] * src[12];
    tmp[10] = src[8] * src[13];
    tmp[11] = src[9] * src[12];

    /* calculate first 8 elements (cofactors) */
    dst[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
    dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
    dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
    dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
    dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
    dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
    dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
    dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
    dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
    dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
    dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
    dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
    dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
    dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
    dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
    dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

    /* calculate pairs for second 8 elements (cofactors) */
    tmp[0] = src[2] * src[7];
    tmp[1] = src[3] * src[6];
    tmp[2] = src[1] * src[7];
    tmp[3] = src[3] * src[5];
    tmp[4] = src[1] * src[6];
    tmp[5] = src[2] * src[5];
    tmp[6] = src[0] * src[7];
    tmp[7] = src[3] * src[4];
    tmp[8] = src[0] * src[6];
    tmp[9] = src[2] * src[4];
    tmp[10] = src[0] * src[5];
    tmp[11] = src[1] * src[4];

    /* calculate second 8 elements (cofactors) */
    dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
    dst[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
    dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
    dst[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
    dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
    dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
    dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
    dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
    dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
    dst[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
    dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
    dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
    dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
    dst[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
    dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
    dst[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];

    /* calculate determinant */
    det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];

    /* calculate matrix inverse */
    det = 1.0f / det;
    for (int j = 0; j < 16; j++){
        dst[j] *= det;
    }

    return inv;
}

