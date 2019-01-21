/*
* Author: Mario J. Martin <dominonurbs$gmail.com>
*
* Implements a Cubic Hermite Spline interpolation.
* This can be used for a smooth move of scene objects or the camera.
* The input are the coordinates of control points with three spatial coordinates
* and a weight that typically is 1.
*
* Tangents are calculated by finite differences as c*(Pk+1 - Pk-1)/2
* With c == 0, there is no tangent, and the vertex becomes a kink.
* At the first and last segment, there are several possibilities:
* - just consider a zero tangent,
* - the tangent is calculated as c*(Pk+1 - Pk) or c*(4*Pk+1 - 3*Pk - Pk+2)/2,
* - force a zero second derivative at the end point.
* We use the last option, which is known as natural splines.
*
* This library is self-independant, and can be compiled outside of GW.
*/

#ifndef _H_GW_SPLINE_H
#define _H_GW_SPLINE_H

#include "math.h"

#include "common/log.h"
#include "gwdata.h"

extern "C"
gwVector3f gwspline
( const gwVector4f* cps /* Array of control points */
, const int ncp         /* Number of control points */
, const float s         /* Indp. parameter */
)
{
    gwVector3f v; /* returned coordinates */
    float t = s*ncp;
    int i0 = (int)floor( t );
    t = t - i0;

    if (ncp <= 0){
        _handle_error_( "The spline has no control points!" );
    }
    else if (ncp == 1){
        /* There is only one control point */
        v.x = cps[0].x;
        v.y = cps[0].y;
        v.z = cps[0].z;
    }
    else if (ncp == 2){
        /* Only two control points are defined.
         * Perform a linear interpolation */
        gwVector4f p0 = cps[0];
        gwVector4f p1 = cps[1];

        v.x = (1 - t)*p0.x + t*p1.x;
        v.y = (1 - t)*p0.y + t*p1.y;
        v.z = (1 - t)*p0.z + t*p1.z;
    }
    else{
        if (s < 0){
            /* The parameter is out of the lower limit. */
            gwVector4f p0 = cps[0];

            v.x = p0.x;
            v.y = p0.y;
            v.z = p0.z;
        }
        else if (i0 >= ncp - 1){
            /* The parameter is out of the upper limit. */
            gwVector4f p1 = cps[ncp - 1];

            v.x = p1.x;
            v.y = p1.y;
            v.z = p1.z;
        }
        else if (i0 == 0){
            /* The first spline segment. Impose zero curvature on p0 */
            gwVector4f p0 = cps[0];
            gwVector4f p1 = cps[1];
            gwVector4f p2 = cps[2];

            gwVector4f m1;
            m1.x = (p2.x - p0.x) / 2;
            m1.y = (p2.y - p0.y) / 2;
            m1.z = (p2.z - p0.z) / 2;

            float a0 = 1 - (3 * t) / 2 + (t*t*t) / 2;
            float a1 = (3 * t) / 2 - (t*t*t) / 2;
            float b1 = ((t*t*t) / 2 - t / 2)*p1.w;


            v.x = a0*p0.x + a1*p1.x + b1*m1.x;
            v.y = a0*p0.y + a1*p1.y + b1*m1.y;
            v.z = a0*p0.z + a1*p1.z + b1*m1.z;
        }
        else if (i0 == ncp - 2){
            /* Last spline segment */
            gwVector4f p0 = cps[i0];
            gwVector4f p1 = cps[i0 + 1];
            gwVector4f p_1 = cps[i0 - 1];

            gwVector3f m0;
            m0.x = (p1.x - p_1.x) / 2;
            m0.y = (p1.y - p_1.y) / 2;
            m0.z = (p1.z - p_1.z) / 2;

            float a0 = 1 - (3 * t*t) / 2 + (t*t*t) / 2;
            float a1 = (3 * t*t) / 2 - (t*t*t) / 2;
            float b0 = (t - t*t) * p0.w;

            v.x = a0*p0.x + a1*p1.x + b0*m0.x;
            v.y = a0*p0.y + a1*p1.y + b0*m0.y;
            v.z = a0*p0.z + a1*p1.z + b0*m0.z;
        }
        else{
            // General case
            gwVector4f p0 = cps[i0];
            gwVector4f p1 = cps[i0 + 1];
            gwVector4f p2 = cps[i0 + 2];
            gwVector4f p_1 = cps[i0 - 1];

            gwVector3f m0;
            m0.x = (p1.x - p_1.x) / 2;
            m0.y = (p1.y - p_1.y) / 2;
            m0.z = (p1.z - p_1.z) / 2;

            gwVector3f m1;
            m1.x = (p2.x - p0.x) / 2;
            m1.y = (p2.y - p0.y) / 2;
            m1.z = (p2.z - p0.z) / 2;

            float a0 = (t - 1)*(t - 1)*(2 * t + 1);
            float a1 = (3 - 2 * t)*t*t;
            float b0 = (t - 1)*(t - 1)*t*p0.w;
            float b1 = (t - 1)*t*t*p1.w;

            v.x = a0*p0.x + a1*p1.x + b0*m0.x + b1*m1.x;
            v.y = a0*p0.y + a1*p1.y + b0*m0.y + b1*m1.y;
            v.z = a0*p0.z + a1*p1.z + b0*m0.z + b1*m1.z;
        }
    }

    return v;
}

#endif /* _H_GW_SPLINE_H */

