 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    to calculate the inversion point on NURBS surface.
    There is no version for curves, yet.

*******************************************************************************/ 


#include <math.h>
#include <float.h>


#include "nurbs_basis.h"
#include "nurbs_surface.h"

#include "nurbs_internal.h"


#define NURBS_EPSILON FLT_EPSILON

/* Takes out the sign of a number. For some reason std::abs() gives problems */
static inline NurbsFloat absf( const NurbsFloat a )
{
    return (a < 0) ? -a : a;
}


/* Gets the first nurbs derivative using finite differences */
static void get_nurbs_derivates_byfd_u
    ( NurbsVector3 *du    /* (out) Derivative */
    , const NurbsFloat u  /* First parameter */
    , const NurbsFloat v  /* Second parameter */
    , const NurbsSurface *surface /* NURBS surface data pointer */
    )
{
    NurbsVector3 p0, p1;
    NurbsFloat epsilon, epsilon2;
    NurbsFloat min, max;

    nurbs_basis_get_parameter_interval( &min, &max
        , surface->knot_u, surface->knot_length_u, surface->degree_u);
    epsilon = (max - min)/1000;

    p0 = nurbs_surface_get_point(surface, u-epsilon, v);
    p1 = nurbs_surface_get_point(surface, u+epsilon, v);

    if (u-epsilon < min || u+epsilon > max){
        epsilon2 = epsilon;
    }
    else{
        epsilon2 = 2 * epsilon;
    }

    du->x = (p1.x - p0.x) / (epsilon2);
    du->y = (p1.y - p0.y) / (epsilon2);
    du->z = (p1.z - p0.z) / (epsilon2);
}

/* Gets the first nurbs derivative using finite differences */
static void get_nurbs_derivates_byfd_v
    ( NurbsVector3 *dv    /* (out) Derivative */
    , const NurbsFloat u  /* First parameter */
    , const NurbsFloat v  /* Second parameter */
    , const NurbsSurface *surface /* NURBS surface data pointer */
    )
{
    NurbsVector3 p0, p1;
    NurbsFloat epsilon, epsilon2;
    NurbsFloat min, max;

    nurbs_basis_get_parameter_interval( &min, &max
        , surface->knot_v, surface->knot_length_v, surface->degree_v);
    epsilon = (max - min)/1000;

    p0 = nurbs_surface_get_point(surface, u, v-epsilon);
    p1 = nurbs_surface_get_point(surface, u, v+epsilon);

    if (v-epsilon < min || v+epsilon > max){
        epsilon2 = epsilon;
    }
    else{
        epsilon2 = 2 * epsilon;
    }

    dv->x = (p1.x - p0.x) / (epsilon2);
    dv->y = (p1.y - p0.y) / (epsilon2);
    dv->z = (p1.z - p0.z) / (epsilon2);
}

/* Calculates the inversion point using an iterative first order method. 
 * Basically is a Newton-Raphson that projects PQ onto the derivative */
int nurbs_surface_inversion_proj_alt
    ( NurbsFloat *pu  /* (in) Quest estimation (out) first parameter solution */
    , NurbsFloat *pv  /* (in) Quest estimation (out) second parameter solution */
    , const NurbsVector3 *point   /* {x, y, z} coordinates of the point */
    , const NurbsSurface *surface /* NURBS surface data structure */
    , const NurbsFloat epsilon    /* Stop condition (eg 1e-6) */
    )
{
    NurbsFloat dist, dist1, err, mod, delta_u, delta_v, bg = 1;
    NurbsVector3 p0, p1, p, du, dv, q;
    NurbsFloat qpv, qpu, u2, v2, uv, d, uf, vf, u, v, best_u, best_v;
    NurbsFloat umin, vmin, umax, vmax;
    int adder;
    int it = 0;

    q.x = point->x;
    q.y = point->y;
    q.z = point->z;

    /* checking boundaries; if input parameter is out of the knot interval,
     * will get the maximun or the minimun value of the knots */
    nurbs_basis_get_parameter_interval(&umin, &umax
        , surface->knot_u, surface->knot_length_u, surface->degree_u);
    nurbs_basis_get_parameter_interval(&vmin, &vmax
        , surface->knot_v, surface->knot_length_v, surface->degree_v);

    if (*pu < umin){
        *pu = umin;
    }
    if (*pu > umax){
        *pu = umax;
    }
    if (*pv < vmin){
        *pv = vmin;
    }
    if (*pv > vmax){
        *pv = vmax;
    }

    uf = *pu;
    vf = *pv;
    u = uf;
    v = vf;

    /* Calculate derivatives */
    nurbs_surface_get_derivatives(&du, &dv, &p1, surface, uf, vf);
    p0.x = q.x;
    p0.y = q.y;
    p0.z = q.z;

    /* If the derivative is zero, recalculate it using finite diferences */
    mod = du.x * du.x + du.y * du.y + du.z * du.z;
    if (mod < FLT_EPSILON){
        get_nurbs_derivates_byfd_u(&du, uf, vf,  surface);
    }

    mod = dv.x * dv.x + dv.y * dv.y + dv.z * dv.z;
    if (mod < FLT_EPSILON){
        get_nurbs_derivates_byfd_v(&dv, uf, vf, surface);
    }

    /* Initial error */
    dist = (p1.x - q.x) * (p1.x - q.x) 
        + (p1.y - q.y) * (p1.y - q.y) 
        + (p1.z - q.z) * (p1.z - q.z);

    bg = 1;

    best_u = *pu;
    best_v = *pv;

    while (it < 100){
        adder = 1;

         /* Take the distance error */
        err = (q.x - p1.x) * (q.x - p1.x) 
            + (q.y - p1.y) * (q.y - p1.y) 
            + (q.z - p1.z) * (q.z - p1.z);

        /* Calculate the proyection onto the derivatives */
        qpu = (q.x - p1.x) * du.x 
            + (q.y - p1.y) * du.y 
            + (q.z - p1.z) * du.z;

        qpv = (q.x - p1.x) * dv.x 
            + (q.y - p1.y) * dv.y 
            + (q.z - p1.z) * dv.z;

        u2 = qpu*qpu;
        v2 = qpv*qpv;
        uv = qpu*qpv;

        d = (u2 * v2 - uv * uv) / err;

        delta_u = (qpu * v2 - qpv * uv) / (2*d);
        delta_v = (qpv * u2 - qpu * uv) / (2*d);

        u = uf + delta_u * bg;
        v = vf + delta_v * bg;

        /* Check boundaries */
        if (u < umin){
            u = umin;
        }
        if (u > umax){
            u = umax;
        }
        if (v < vmin){
            v = vmin;
        }
        if (v > vmax){
            v = vmax;
        }

        /* Move through u projection */
        p = nurbs_surface_get_point(surface, u, vf);
        dist1 = (p.x - q.x) * (p.x - q.x) 
            + (p.y - q.y) * (p.y - q.y) 
            + (p.z - q.z) * (p.z - q.z);

        /* Check if there is an improvement */
        if (dist > dist1){
            best_u = u;
            best_v = vf;
            dist = dist1;
            adder = 0;
            p1.x = p.x;
            p1.y = p.y;
            p1.z = p.z;
        }

        /* Move through v projection */
        p = nurbs_surface_get_point(surface, uf, v);
        dist1 = (p.x - q.x) * (p.x - q.x) 
            + (p.y - q.y) * (p.y - q.y) 
            + (p.z - q.z) * (p.z - q.z);

        /* Check if there is an improvement */
        if (dist > dist1){
            best_u = uf;
            best_v = v;
            vf = v;
            dist = dist1;
            adder = 0;
            p1.x = p.x;
            p1.y = p.y;
            p1.z = p.z;
        }

        /* Move through uv projection */
        p = nurbs_surface_get_point(surface, u, v);
        dist1 = (p.x - q.x) * (p.x - q.x) 
            + (p.y - q.y) * (p.y - q.y) 
            + (p.z - q.z) * (p.z - q.z);

        /* Check if there is an improvement */
        if (dist > dist1){
            best_u = uf;
            best_v = vf;
            dist = dist1;
            adder = 0;
            p1.x = p.x;
            p1.y = p.y;
            p1.z = p.z;
        }

        /* Take the distance error */
        err = (p0.x - p1.x) * (p0.x - p1.x) 
            + (p0.y - p1.y) * (p0.y - p1.y) 
            + (p0.z - p1.z) * (p0.z - p1.z);

        if (err < epsilon){
            /* Ok, this is maybe a good solution; stop */
            *pu = best_u;
            *pv = best_v;
            uf = best_u;
            vf = best_v;

            return 0;
        }

        /* Calculate derivative vectors */
        nurbs_surface_get_derivatives(&du, &dv, &p, surface, uf, vf);

        if (adder == 1){
            /* Reduce the movement each iteration the solution is not improved */
            bg *= 0.5f;
        }
        else{
            p0.x = p1.x;
            p0.y = p1.y;
            p0.z = p1.z;
        }

        /* If the derivatives is zero, recalculate it using finite diferences */
        mod = du.x * du.x + du.y * du.y + du.z * du.z;
        if (mod < NURBS_EPSILON){
            get_nurbs_derivates_byfd_u(&du, uf, vf,  surface);
        }

        mod = dv.x * dv.x + dv.y * dv.y + dv.z * dv.z;
        if (mod < NURBS_EPSILON){
            get_nurbs_derivates_byfd_v(&dv, uf, vf,  surface);
        }

        it += 1;
    }

    /* Maybe no solution is found, there are problems of convergence, who knows... */

    return 1;
}


/* Calculates the inversion point using an iterative first order method. 
 * Basically is a Newton-Raphson that projects PQ onto the derivative */
int nurbs_surface_inversion_proj
    ( NurbsFloat *pu  /* (in) Quest estimation (out) first parameter solution */
    , NurbsFloat *pv  /* (in) Quest estimation (out) second parameter solution */
    , const NurbsVector3 *point   /* {x, y, z} coordinates of the point */
    , const NurbsSurface *surface /* NURBS surface data structure */
    , const NurbsFloat epsilon    /* Stop condition (eg 1e-6) */
    )
{
    NurbsFloat qpv = 0, qpu = 0, u2 = 0, v2 = 0, uv = 0, d = 0, bg = 1;
    NurbsFloat uf, vf, u, v;
    NurbsFloat dist, dist1, mod, delta_u = 0, delta_v = 0;
    NurbsVector3 p, du, dv, q;
    NurbsFloat umin, vmin, umax, vmax;
    int fail = 0;
    int it_not_improved = 0;

    q.x = point->x;
    q.y = point->y;
    q.z = point->z;

    /* checking boundaries; if input parameter is out of the knot interval,
     * will get the maximun or the minimun value of the knots */
    nurbs_basis_get_parameter_interval(&umin, &umax
        , surface->knot_u, surface->knot_length_u, surface->degree_u);
    nurbs_basis_get_parameter_interval(&vmin, &vmax
        , surface->knot_v, surface->knot_length_v, surface->degree_v);

    if (*pu < umin){
        *pu = umin;
    }
    if (*pu > umax){
        *pu = umax;
    }
    if (*pv < vmin){
        *pv = vmin;
    }
    if (*pv > vmax){
        *pv = vmax;
    }

    uf = *pu;
    vf = *pv;
    u = uf;
    v = vf;

    /* Calculate derivatives */
    p = nurbs_surface_get_point( surface, uf, vf );

    /* Initial error */
    dist = (p.x - q.x) * (p.x - q.x) 
        + (p.y - q.y) * (p.y - q.y) 
        + (p.z - q.z) * (p.z - q.z);
    
    while (it_not_improved < 16)
    {
        if (fail == 0){
            /* Calculate derivative vectors */
            nurbs_surface_get_derivatives(&du, &dv, &p, surface, uf, vf);

            /* If the derivatives is zero, recalculate it using finite diferences */
            mod = du.x * du.x + du.y * du.y + du.z * du.z;
            if (mod < NURBS_EPSILON){
                get_nurbs_derivates_byfd_u(&du, uf, vf,  surface);
            }

            mod = dv.x * dv.x + dv.y * dv.y + dv.z * dv.z;
            if (mod < NURBS_EPSILON){
                get_nurbs_derivates_byfd_v(&dv, uf, vf,  surface);
            }

            /* Calculate the proyection onto the derivatives */
            qpu = (q.x - p.x) * du.x 
                + (q.y - p.y) * du.y 
                + (q.z - p.z) * du.z;

            qpv = (q.x - p.x) * dv.x 
                + (q.y - p.y) * dv.y 
                + (q.z - p.z) * dv.z;

            u2 = du.x * du.x + du.y * du.y + du.z * du.z;
            v2 = dv.x * dv.x + dv.y * dv.y + dv.z * dv.z;
            uv = du.x * dv.x + du.y * dv.y + du.z * dv.z;

            d = u2 * v2 - uv * uv;

            if (!(d<0 || d>0)){
                /* Probably the point is on the vertical of the NURBS  */
                return 0;
            }        
            delta_u = (qpu * v2 - qpv * uv) / d;
            delta_v = (qpv * u2 - qpu * uv) / d;
        }

        fail = 1;

        u = uf + delta_u * bg;
        v = vf + delta_v * bg;

        /* Check boundaries */
        if (u < umin){
            u = umin;
        }
        if (u > umax){
            u = umax;
        }
        if (v < vmin){
            v = vmin;
        }
        if (v > vmax){
            v = vmax;
        }

        /* Move through u projection */
        p = nurbs_surface_get_point(surface, u, vf);
        dist1 = (p.x - q.x) * (p.x - q.x) 
            + (p.y - q.y) * (p.y - q.y) 
            + (p.z - q.z) * (p.z - q.z);

        /* Check if there is an improvement */
        if (dist > dist1){
            *pu = u;
            *pv = vf;
            dist = dist1;
            fail = 0;
        }

        /* Move through v projection */
        p = nurbs_surface_get_point(surface, uf, v);
        dist1 = (p.x - q.x) * (p.x - q.x) 
            + (p.y - q.y) * (p.y - q.y) 
            + (p.z - q.z) * (p.z - q.z);

        /* Check if there is an improvement */
        if (dist > dist1){
            *pu = uf;
            *pv = v;
            dist = dist1;
            fail = 0;
        }

        /* Move through uv projection */
        p = nurbs_surface_get_point(surface, u, v);
        dist1 = (p.x - q.x) * (p.x - q.x) 
            + (p.y - q.y) * (p.y - q.y) 
            + (p.z - q.z) * (p.z - q.z);

        /* Check if there is an improvement */
        if (dist > dist1){
            *pu = u;
            *pv = v;
            dist = dist1;
            fail = 0;
        }

        if (sqrt(dist) < epsilon){
            /* Ok, this is a good solution; stop */
            return 0;
        }
        //if (sqrt(qpu*qpu + qpv*qpv) < eps2){
            /* The projection is almost vertical. The point is not exactly on the NURBS */
            //return 0;
        //}

        if (fail == 1){
            /* Reduce the movement when the solution is not improved */
            bg *= 0.5f;
            it_not_improved += 1;
        }
        else{
            bg = 1;
            uf = *pu;
            vf = *pv;
        }
    }

    /* Maybe no solution is found, maybe there are problems with the convergence, or who knows... */

    return 1;
}

/* Calculates the inversion using an iterative bilineal quad method */
int nurbs_surface_inversion_quad
    ( NurbsFloat *pu  /* (in) Quest estimation (out) first parameter solution  */
    , NurbsFloat *pv  /* (in) Quest estimation (out) second parameter solution */
    , const NurbsVector3 *point  /* {x, y, z} coordinates of the point */
    , const NurbsSurface *surface_orig  /* NURBS surface data structure */
    , const NurbsFloat epsilon          /* Stop condition (eg 1e-6) */
    )
{
    NurbsFloat dist, dist0, dist1;
    int iu, iv;
    int it = 1;
    NurbsFloat du = 0, dv = 0;
    NurbsFloat u0, u1, v0, v1;
    NurbsFloat u = *pu;
    NurbsFloat v = *pv;
    NurbsFloat umin, vmin, umax, vmax;
    NurbsVector3 p0, p1, pc, q;
    int adder;
    NurbsFloat bg = 1;

    q.x = point->x;
    q.y = point->y;
    q.z = point->z;

    p0 = nurbs_surface_get_point(surface_orig, *pu, *pv);
    dist = q.x * q.x + q.y * q.y + q.z * q.z;

    nurbs_basis_get_parameter_interval(&umin, &umax
        , surface_orig->knot_u
        , surface_orig->knot_length_u, surface_orig->degree_u);

    nurbs_basis_get_parameter_interval(&vmin, &vmax
        , surface_orig->knot_v
        , surface_orig->knot_length_v, surface_orig->degree_v);

    iu = nurbs_basis_knot_index(*pu, surface_orig->knot_u
        , surface_orig->knot_length_u);
    iv = nurbs_basis_knot_index(*pv, surface_orig->knot_v
        , surface_orig->knot_length_v);

    if (iu >= 0 && iu < surface_orig->knot_length_u - 1){
        du = (surface_orig->knot_u[iu+1] - surface_orig->knot_u[iu]) / 2;
    }
    else if (iu > 0){
        du = (surface_orig->knot_u[iu] - surface_orig->knot_u[iu-1]) / 2;
    }
    else{
        /* It is outside of the knot interval */
        return 0;
    }

    if (iv >= 0 && iv < surface_orig->knot_length_v - 1){
        dv = (surface_orig->knot_v[iv+1] - surface_orig->knot_v[iv]) / 2;
    }
    else if (iv > 0){
        dv = (surface_orig->knot_v[iv] - surface_orig->knot_v[iv-1]) / 2;
    }
    else{
        /* It is outside of the knot interval */
        return 0;
    }

    while (it < 100){
        adder = 1;

        /* u direction */
        u0 = u - du/bg;
        u1 = u + du/bg;

        if (u0 < umin){
            u0 = umin;
        }
        if (u1 > umax){
            u1 = umax;
        }

        p0 = nurbs_surface_get_point(surface_orig, u0, v);
        p1 = nurbs_surface_get_point(surface_orig, u1, v);
        pc = nurbs_surface_get_point(surface_orig, u, v);

        dist0 = (p0.x - q.x) * (p0.x - q.x) 
            + (p0.y - q.y) * (p0.y - q.y) 
            + (p0.z - q.z) * (p0.z - q.z);

        dist1 = (p1.x - q.x) * (p1.x - q.x) 
            + (p1.y - q.y) * (p1.y - q.y) 
            + (p1.z - q.z) * (p1.z - q.z);

        dist  = (pc.x - q.x) * (pc.x - q.x) 
            + (pc.y - q.y) * (pc.y - q.y) 
            + (pc.z - q.z) * (pc.z - q.z);

        if (dist0 < dist){
            u = u0;
            dist = dist0;
            adder = 0;
        }
        if (dist1 < dist){
            u = u1;
            dist = dist1;
            adder = 0;
        }

        /* v direction */
        v0 = v - dv * bg;
        v1 = v + dv * bg;

        if (v0 < vmin){
            v0 = vmin;
        }
        if (v1 > vmax){
            v1 = vmax;
        }

        p0 = nurbs_surface_get_point(surface_orig, u, v0);
        p1 = nurbs_surface_get_point(surface_orig, u, v1);
        pc = nurbs_surface_get_point(surface_orig, u, v);

        dist0 = (p0.x - q.x) * (p0.x - q.x) 
            + (p0.y - q.y) * (p0.y - q.y) 
            + (p0.z - q.z) * (p0.z - q.z);

        dist1 = (p1.x - q.x) * (p1.x - q.x) 
            + (p1.y - q.y) * (p1.y - q.y) 
            + (p1.z - q.z) * (p1.z - q.z);

        dist = (pc.x - q.x) * (pc.x - q.x) 
            + (pc.y - q.y) * (pc.y - q.y) 
            + (pc.z - q.z) * (pc.z - q.z);

        if (dist0 < dist){
            v = v0;
            dist = dist0;
            adder = 0;
        }
        if (dist1 < dist){
            v = v1;
            dist = dist1;
            adder = 0;
        }

        it += 1;
        if (adder != 0){
            /* Reduce the step each time there is no improvement */
            bg *= 0.5;
        }

        *pu = u;
        *pv = v;
    }

    /* It always returns 1 unless the initial input is outside the knot interval */
    return 1;
}

/* ------------------------------------------------------------------------- */


/* Calculates the roots of a square polinomial in the form a*x*x + b*x + c = 0. 
   Returns 0 if the roots are not imaginary */
static int root2
    ( NurbsFloat *r1              /* (out) First root */
    , NurbsFloat *r2              /* (out) Second root */
    , const NurbsFloat a          /* a x^2 */
    , const NurbsFloat b          /* b x */
    , const NurbsFloat c          /* c */
    , const NurbsFloat threshold  /* Used to identify it is not a second order    */
    )
{
    NurbsFloat q = b*b - 4*a*c;
    if (q < 0){
        /* Imaginary roots */
        return 1;
    }
    else{
        if ( absf(a)/absf(b+c) < threshold){
            /* It is unlikely a second order equation */
            *r1 = -c / b;
            *r2 = -c / b;
            return 0;
        }
        q = (NurbsFloat)sqrt(q);
        *r1 = (-b + q) / (2*a);
        *r2 = (-b - q) / (2*a);
        return 0;
    }
}

/* Solves an hiperbolic equation in the form a*x*y + b*x + c*y + e = 0 */
static int solveHiperboleEquation
    ( NurbsFloat *px0
    , NurbsFloat *px1
    , NurbsFloat *py0
    , NurbsFloat *py1
    , const NurbsFloat a1
    , const NurbsFloat a2
    , const NurbsFloat b1
    , const NurbsFloat b2
    , const NurbsFloat c1
    , const NurbsFloat c2
    , const NurbsFloat d1
    , const NurbsFloat d2
    )
{
    NurbsFloat x0, x1, y0, y1;
    NurbsFloat beta0, beta1;
    NurbsFloat a = a1*c2-a2*c1;
    NurbsFloat b = a1*d2-a2*d1+b1*c2-b2*c1;
    NurbsFloat c = b1*d2-b2*d1;

    int s = root2(&y0, &y1, a, b, c, 1e-6f);
    if (s!=0){
        /* Not a real solution */
        return 1;
    }
    else{
        /* First solution */
        beta0 = a1*y0+b1;
        beta1 = a2*y0+b2;
        /* Trying to avoid zeros in the division */
        if (absf(beta0) > absf(beta1)){
            x0 = (-d1-c1*y0)/beta0;
        }
        else{
            x0 = (-d2-c2*y0)/beta1;
        }

        /* Second solution */
        beta0 = a1*y1+b1;
        beta1 = a2*y1+b2;
        /* Trying to avoid zeros in the division */
        if (absf(beta0) > absf(beta1)){
            x1 = (-d1-c1*y1)/beta0;
        }
        else{
            x1 = (-d2-c2*y1)/beta1;
        }
    }

    *px0 = x0;
    *px1 = x1;
    *py0 = y0;
    *py1 = y1;

    return 0;
}


/* Solves an triperhiperbolic equation in the form 
 * a*x*y + b*x + c*y + d*z + e = 0 */
static int solveLineHiperboleIntersection
    ( NurbsFloat *px0, NurbsFloat *px1
    , NurbsFloat *py0, NurbsFloat *py1
    , NurbsFloat *pz0, NurbsFloat *pz1
    , const NurbsFloat A1, const NurbsFloat A2, const NurbsFloat A3
    , const NurbsFloat B1, const NurbsFloat B2, const NurbsFloat B3
    , const NurbsFloat C1, const NurbsFloat C2, const NurbsFloat C3
    , const NurbsFloat D1, const NurbsFloat D2, const NurbsFloat D3
    , const NurbsFloat E1, const NurbsFloat E2, const NurbsFloat E3
    )
{
    int status;
    NurbsFloat x0, x1, y0, y1, z0, z1;
    NurbsFloat a1 = A2-A1*(D2/D1);
    NurbsFloat a2 = A3-A1*(D3/D1);
    NurbsFloat b1 = B2-B1*(D2/D1);
    NurbsFloat b2 = B3-B1*(D3/D1);
    NurbsFloat c1 = C2-C1*(D2/D1);
    NurbsFloat c2 = C3-C1*(D3/D1);
    NurbsFloat e1 = E2-E1*(D2/D1);
    NurbsFloat e2 = E3-E1*(D3/D1);

    status = solveHiperboleEquation
        (&x0, &x1, &y0, &y1, a1, a2, b1, b2, c1, c2, e1, e2);

    if (status!=0){
        /* Not a real solution */
        return 1;
    }
    else{
        z0 = (-E1-A1*x0*y0-B1*x0-C1*y0)/D1;
        z1 = (-E1-A1*x1*y1-B1*x1-C1*y1)/D1;
    }

    *px0 = x0;
    *px1 = x1;
    *py0 = y0;
    *py1 = y1;
    *pz0 = z0;
    *pz1 = z1;

    return 0;
}


/* Solves the analytical solution of the intersection for one knot panel
 * with a normal direction */
static int solve_local_inversion_normal_order2
    ( NurbsFloat *pu  /* (out) First parameter solution */
    , NurbsFloat *pv  /* (out) Second parameter solution */
    , const NurbsSurface *surface /* NURBS surface data pointer */
    , const NurbsVector3 *normal  /* Normal direction */
    , const int cp_i              /* First knot panel */
    , const int cp_j              /* Second knot panel */
    , const NurbsVector3 *point   /* {x, y, z} coordinates */
    , const int shift             /* Indicate the shifting to maximaze precision */
    , const NurbsFloat rgamma     /* Relaxing factor */
    , const NurbsFloat threshold  /* Threshold if two knots are too close    */
    )
{
    register NurbsVector3 q;
    register NurbsVector3 n;
    NurbsFloat vj, vj1, ui, ui1;
    NurbsFloat a1, a2, a3;
    NurbsFloat b1, b2, b3;
    NurbsFloat c1, c2, c3;
    NurbsFloat d1, d2, d3;
    NurbsFloat e1, e2, e3;
    NurbsFloat u0, u1, v0, v1, r0, r1;
    NurbsFloat phi;
    NurbsFloat temp;
    NurbsVector4 cij, ci_1j, cij_1, ci_1j_1;
    int status;

    q.x = point->x;
    q.y = point->y;
    q.z = point->z;
    n.x = normal->x;
    n.y = normal->y;
    n.z = normal->z;

    ui = surface->knot_u[cp_i];
    ui1 = surface->knot_u[cp_i + 1];
    vj = surface->knot_v[cp_j];
    vj1 = surface->knot_v[cp_j + 1];

    /* special case. The knots are too close!
    * (I DO NOT KNOW HOW TO PROPERLY DEAL WITH THIS YET) */
    if (absf(ui1 - ui) < threshold || absf(vj1 - vj) < threshold){
        return 1;
    }

    phi = 1;

    cij = surface->cp[cp_i][cp_j];
    cij.x *= cij.w;
    cij.y *= cij.w;
    cij.z *= cij.w;

    ci_1j = surface->cp[cp_i - 1][cp_j];
    ci_1j.x *= ci_1j.w;
    ci_1j.y *= ci_1j.w;
    ci_1j.z *= ci_1j.w;

    cij_1 = surface->cp[cp_i][cp_j - 1];
    cij_1.x *= cij_1.w;
    cij_1.y *= cij_1.w;
    cij_1.z *= cij_1.w;

    ci_1j_1 = surface->cp[cp_i - 1][cp_j - 1];
    ci_1j_1.x *= ci_1j_1.w;
    ci_1j_1.y *= ci_1j_1.w;
    ci_1j_1.z *= ci_1j_1.w;

    a1 = (ci_1j_1.x + cij.x - ci_1j.x - cij_1.x) /((ui1-ui)*(vj1-vj)*phi);
    a2 = (ci_1j_1.y + cij.y - ci_1j.y - cij_1.y) /((ui1-ui)*(vj1-vj)*phi);
    a3 = (ci_1j_1.z + cij.z - ci_1j.z - cij_1.z) /((ui1-ui)*(vj1-vj)*phi);
    b1 = (ci_1j.x * vj + cij_1.x * vj1 - cij.x * vj - ci_1j_1.x * vj1) 
        / ((ui1-ui)*(vj1-vj)*phi);
    b2 = (ci_1j.y * vj + cij_1.y * vj1 - cij.y * vj - ci_1j_1.y * vj1) 
        / ((ui1-ui)*(vj1-vj)*phi);
    b3 = (ci_1j.z * vj + cij_1.z * vj1 - cij.z * vj - ci_1j_1.z * vj1) 
        / ((ui1-ui)*(vj1-vj)*phi);
    c1 = (ci_1j.x * ui1 + cij_1.x * ui - cij.x * ui - ci_1j_1.x * ui1) 
        / ((ui1-ui)*(vj1-vj)*phi);
    c2 = (ci_1j.y * ui1 + cij_1.y * ui - cij.y * ui - ci_1j_1.y * ui1) 
        / ((ui1-ui)*(vj1-vj)*phi);
    c3 = (ci_1j.z * ui1 + cij_1.z * ui - cij.z * ui - ci_1j_1.z * ui1) 
        / ((ui1-ui)*(vj1-vj)*phi);
    d1 = - n.x;
    d2 = - n.y;
    d3 = - n.z;
    e1 = (cij.x * vj * ui + ci_1j_1.x * ui1 * vj1 
        - cij_1.x * ui * vj1 - ci_1j.x * ui1 * vj) / ((ui1-ui)*(vj1-vj)*phi) 
        - q.x;
    e2 = (cij.y * vj * ui + ci_1j_1.y * ui1 * vj1 
        - cij_1.y * ui * vj1 - ci_1j.y * ui1 * vj) / ((ui1-ui)*(vj1-vj)*phi) 
        - q.y;
    e3 = (cij.z * vj * ui + ci_1j_1.z * ui1 * vj1 
        - cij_1.z * ui * vj1 - ci_1j.z * ui1 * vj) / ((ui1-ui)*(vj1-vj)*phi) 
        - q.z;

    /* {x, y, z} coordinates are shifted taking the greatest normal component */
    if (shift == 1){
        temp = a1; a1 = a2; a2 = a3; a3 = temp;
        temp = b1; b1 = b2; b2 = b3; b3 = temp;
        temp = c1; c1 = c2; c2 = c3; c3 = temp;
        temp = d1; d1 = d2; d2 = d3; d3 = temp;
        temp = e1; e1 = e2; e2 = e3; e3 = temp;
    }
    else if (shift == 2){
        temp = a3; a3 = a2; a2 = a1; a1 = temp;
        temp = b3; b3 = b2; b2 = b1; b1 = temp;
        temp = c3; c3 = c2; c2 = c1; c1 = temp;
        temp = d3; d3 = d2; d2 = d1; d1 = temp;
        temp = e3; e3 = e2; e2 = e1; e1 = temp;
    }

    status = solveLineHiperboleIntersection
        ( &u0, &u1, &v0, &v1, &r0, &r1
        , a1, a2, a3, b1, b2, b3, c1, c2, c3, d1, d2, d3, e1, e2, e3);

    if (status != 0)
    return status;

    /* de-shifting */
    if (shift == 1){
        temp = a3; a3 = a2; a2 = a1; a1 = temp;
        temp = b3; b3 = b2; b2 = b1; b1 = temp;
        temp = c3; c3 = c2; c2 = c1; c1 = temp;
        temp = d3; d3 = d2; d2 = d1; d1 = temp;
        temp = e3; e3 = e2; e2 = e1; e1 = temp;
    }
    else if (shift == 2){
        temp = a1; a1 = a2; a2 = a3; a3 = temp;
        temp = b1; b1 = b2; b2 = b3; b3 = temp;
        temp = c1; c1 = c2; c2 = c3; c3 = temp;
        temp = d1; d1 = d2; d2 = d3; d3 = temp;
        temp = e1; e1 = e2; e2 = e3; e3 = temp;
    }

    if (u0 >= ui-(ui1-ui)*rgamma && u0 <= ui1+(ui1-ui)*rgamma
        && v0 >= vj-(vj1-vj)*rgamma && v0 <= vj1+(vj1-vj)*rgamma){

        *pu = u0;
        *pv = v0;
        return 0;
    }
    if (u1 >= ui-(ui1-ui)*rgamma && u1 <= ui1+(ui1-ui)*rgamma
        && v1 >= vj-(vj1-vj)*rgamma && v1 <= vj1+(vj1-vj)*rgamma ){

        *pu = u1;
        *pv = v1;
        return 0;
    }

    return 1;
}


/* Estimates the nurbs parameters (the hard way) 
 * by comparing distances with knots. It always found one solution */
void nurbs_surface_estimation_subgrid
    ( NurbsFloat *pu    /* Returned value */
    , NurbsFloat *pv    /* Returned value */
    , const NurbsSurface *surface /* Non reduced NURBS surface        */
    , const NurbsVector3 *point   /* Coordinates of the point {x, y, z} */
    , const int grid_density      /* Set the number of divisions used (min: 1) */
    )
{
    int i, j;
    NurbsFloat ri, rj;
    NurbsFloat u, v, us, vs;
    NurbsVector3 q;
    NurbsFloat dist;
    NurbsFloat best_dist = 1e10;	/* The initial value is for avoiding a warning */
    int first = 1;

    if (grid_density <= 1){
        /* Checks only the knots values */
        for (i = 0; i < surface->knot_length_u; i++){
            for (j = 0; j < surface->knot_length_v; j++){
                u = surface->knot_u[i];
                v = surface->knot_v[j];
                q = nurbs_surface_get_point(surface, u, v);
                dist = (point->x - q.x) * (point->x - q.x) 
                + (point->y - q.y) * (point->y - q.y) 
                + (point->z - q.z) * (point->z - q.z);
                if (first == 1 || dist < best_dist){
                    *pu = u;
                    *pv = v;
                    best_dist = dist;
                    first = 0;
                }
            }
        }
    }
    else{
        for (i = 0; i < surface->knot_length_u-1; i++){
            for (j = 0; j < surface->knot_length_v-1; j++){
                for (ri = 0; ri < 1; ri += (NurbsFloat)(1./grid_density)){
                    for (rj = 0; rj < 1; rj += (NurbsFloat)(1./grid_density)){
                        u = surface->knot_u[i]; 
                        u += (surface->knot_u[i+1]-surface->knot_u[i])*ri;
                        v = surface->knot_v[j];
                        v += (surface->knot_v[j+1]-surface->knot_v[j])*rj;
                        q = nurbs_surface_get_point(surface, u, v);
                        dist = (point->x - q.x) * (point->x - q.x) 
                            + (point->y - q.y) * (point->y - q.y) 
                            + (point->z - q.z) * (point->z - q.z);

                        if (first == 1 || dist < best_dist){
                            *pu = u;
                            *pv = v;
                            best_dist = dist;
                            first = 0;
                        }
                    }
                }
            }
        }

        /* We also need to check the last knots on v */
        v = surface->knot_v[surface->knot_length_v];
        for (i = 0; i < surface->knot_length_u; i++){
            u = surface->knot_u[i];
            q = nurbs_surface_get_point(surface, u, v);
            dist = (point->x - q.x) * (point->x - q.x) 
            + (point->y - q.y) * (point->y - q.y) 
            + (point->z - q.z) * (point->z - q.z);
            if (first == 1 || dist < best_dist){
                *pu = u;
                *pv = v;
                best_dist = dist;
                first = 0;
            }
        }

        /* We also need to check the last knots on u */
        u = surface->knot_u[surface->knot_length_v];
        for (j = 0; j < surface->knot_length_v; j++){
            v = surface->knot_v[j];
            q = nurbs_surface_get_point(surface, u, v);
            dist = (point->x - q.x) * (point->x - q.x) 
            + (point->y - q.y) * (point->y - q.y) 
            + (point->z - q.z) * (point->z - q.z);
            if (first == 1 || dist < best_dist){
                *pu = u;
                *pv = v;
                best_dist = dist;
                first = 0;
            }
        }
    }

    us = *pu;
    vs = *pv;
    u = us;
    v = vs;
}


/* Estimates the nurbs parameters (the hard way) 
 * by comparing distances with knots. It always found one solution. */
void nurbs_surface_inversion_subgrid
    ( NurbsFloat *pu    /* Returned value */
    , NurbsFloat *pv    /* Returned value */
    , const NurbsSurface *surface /* Non reduced NURBS surface        */
    , const NurbsVector3 *point   /* Coordinates of the point {x, y, z} */
    , const NurbsFloat epsilon    /* Stop condition for the iterative methods */
    , const int grid_density      /* Set the number of divisions used (min: 1) */
    )
{
    int i, j;
    NurbsFloat ri, rj;
    NurbsFloat u, v, us, vs;
    NurbsVector3 q;
    NurbsFloat dist;
    NurbsFloat best_dist = 1e10;	/* the initial value is for avoiding a warning */
    int first = 1;

    if (grid_density <= 1){
        /* Checks only the knots values */
        for (i = 0; i < surface->knot_length_u; i++){
            for (j = 0; j < surface->knot_length_v; j++){
                u = surface->knot_u[i];
                v = surface->knot_v[j];
                q = nurbs_surface_get_point(surface, u, v);
                dist = (point->x - q.x) * (point->x - q.x) 
                    + (point->y - q.y) * (point->y - q.y) 
                    + (point->z - q.z) * (point->z - q.z);
                if (first == 1 || dist < best_dist){
                    *pu = u;
                    *pv = v;
                    best_dist = dist;
                    first = 0;
                }
            }
        }
    }
    else{
        for (i = 0; i < surface->knot_length_u-1; i++){
            for (j = 0; j < surface->knot_length_v-1; j++){
                for (ri = 0; ri < 1; ri += (NurbsFloat)(1./grid_density)){
                    for (rj = 0; rj < 1; rj += (NurbsFloat)(1./grid_density)){
                        u = surface->knot_u[i]; 
                        u += (surface->knot_u[i+1]-surface->knot_u[i])*ri;
                        v = surface->knot_v[j];
                        v += (surface->knot_v[j+1]-surface->knot_v[j])*rj;
                        q = nurbs_surface_get_point(surface, u, v);
                        dist = (point->x - q.x) * (point->x - q.x) 
                            + (point->y - q.y) * (point->y - q.y) 
                            + (point->z - q.z) * (point->z - q.z);

                        if (first == 1 || dist < best_dist){
                            *pu = u;
                            *pv = v;
                            best_dist = dist;
                            first = 0;
                        }
                    }
                }
            }
        }

        /* We also need to check the last knots on v */
        v = surface->knot_v[surface->knot_length_v];
        for (i = 0; i < surface->knot_length_u; i++){
            u = surface->knot_u[i];
            q = nurbs_surface_get_point(surface, u, v);
            dist = (point->x - q.x) * (point->x - q.x) 
            + (point->y - q.y) * (point->y - q.y) 
            + (point->z - q.z) * (point->z - q.z);
            if (first == 1 || dist < best_dist){
                *pu = u;
                *pv = v;
                best_dist = dist;
                first = 0;
            }
        }

        /* We also need to check the last knots on u */
        u = surface->knot_u[surface->knot_length_v];
        for (j = 0; j < surface->knot_length_v; j++){
            v = surface->knot_v[j];
            q = nurbs_surface_get_point(surface, u, v);
            dist = (point->x - q.x) * (point->x - q.x) 
            + (point->y - q.y) * (point->y - q.y) 
            + (point->z - q.z) * (point->z - q.z);
            if (first == 1 || dist < best_dist){
                *pu = u;
                *pv = v;
                best_dist = dist;
                first = 0;
            }
        }
    }

    us = *pu;
    vs = *pv;
    u = us;
    v = vs;

    /* Perform the first iterative method */
    nurbs_surface_inversion_quad(&u, &v, point, surface, epsilon);
    q = nurbs_surface_get_point(surface, u, v);
    dist = (q.x - point->x) * (q.x - point->x) 
        + (q.y - point->y) * (q.y - point->y) 
        + (q.z - point->z) * (q.z - point->z);

    if (dist < best_dist){
        best_dist = dist;
        *pu = u;
        *pv = v;
    }

    u = us;
    v = vs;

    /* Perform the second iterative method */
    nurbs_surface_inversion_proj(&u, &v, point, surface, epsilon);
    q = nurbs_surface_get_point(surface, u, v);
    dist = (q.x - point->x) * (q.x - point->x) 
        + (q.y - point->y) * (q.y - point->y) 
        + (q.z - point->z) * (q.z - point->z);

    if (dist < best_dist){
        best_dist = dist;
        *pu = u;
        *pv = v;
    }
}

/* Estimates the inversion by the intersection of the normal
 * with an equivalent second order nurbs 
 * returns the number of estimations found. */
int nurbs_surface_estimation_normal
    ( NurbsFloat *pu    /* Returned solution */
    , NurbsFloat *pv    /* Returned solution */
    , const NurbsSurface *surface_orig   /* Non reduced NURBS surface  */
    , const NurbsSurface *surface_order2 /* Second order NURBS surface */
    , const NurbsVector3 *point  /* Coordinates of the point {x, y, z} */
    , const NurbsVector3 *normal /* Surface normal (should be normalized) */
    , const NurbsFloat rgamma    /* Relaxing factor used by the estimation */
    )
{
    int i, j;
    NurbsVector3 q;
    NurbsFloat dist;
    NurbsFloat best_dist = 1e10;	/* The initial value is for avoiding a warning */
    NurbsFloat u, v;
    int num_solutions = 0;
    int shift = 0, status;
    NurbsFloat mod;
    int fail_inversion = 1;

    /* Check the greatest component of the normal 
    * and shift the three equations to maximize precision */
    mod = absf(normal->x);
    if (absf(normal->y) > mod){
        shift = 1;
        mod = absf(normal->y);
    }
    if (absf(normal->z) > mod){
        shift = 2;
    }

    *pu = -1;
    *pv = -1;

    for (i = 1; i < surface_order2->cp_length_u; i++){
        for (j = 1; j < surface_order2->cp_length_v; j++){
            /* Calculate the local intersection */
            status = solve_local_inversion_normal_order2
                ( &u, &v, surface_order2, normal, i, j
                , point, shift, rgamma, NURBS_EPSILON);

            if (status == 0){
                q = nurbs_surface_get_point(surface_orig, u, v);
                dist = (q.x - point->x) * (q.x - point->x) 
                    + (q.y - point->y) * (q.y - point->y) 
                    + (q.z - point->z) * (q.z - point->z);

                if (num_solutions == 0){
                    fail_inversion = 0;
                    *pu = u;
                    *pv = v;
                    best_dist = dist;
                }
                else{
                    if (dist < best_dist){
                        *pu = u;
                        *pv = v;
                        best_dist = dist;
                    }
                }

                num_solutions++;
            }
        }
    }

    /* Check if the solution is on the edges */
    if (fail_inversion == 0){
        if (*pu < surface_orig->knot_u[0]){
            *pu = surface_orig->knot_u[0];
        }
        if (*pu > surface_orig->knot_u[surface_orig->knot_length_u-1]){
            *pu = surface_orig->knot_u[surface_orig->knot_length_u-1];
        }
        if (*pv < surface_orig->knot_v[0]){
            *pv = surface_orig->knot_v[0];
        }
        if (*pv > surface_orig->knot_v[surface_orig->knot_length_v-1]){
            *pv = surface_orig->knot_v[surface_orig->knot_length_v-1];
        }
    }

    return num_solutions;
}

/* Calculates the inversion by the intersection of the normal
 * with an equivalent second order nurbs, and then apply an iterative method 
 * returns the number of estimations found. */
int nurbs_surface_inversion_normal
    ( NurbsFloat *pu    /* Returned solution */
    , NurbsFloat *pv    /* Returned solution */
    , const NurbsSurface *surface_orig   /* Non reduced NURBS surface        */
    , const NurbsSurface *surface_order2 /* Second order NURBS surface        */
    , const NurbsVector3 *point  /* Coordinates of the point {x, y, z} */
    , const NurbsVector3 *normal /* Surface normal (should be normalized) */
    , const NurbsFloat epsilon   /* Stop condition for the iterative methods  */
    , const NurbsFloat rgamma    /* Relaxing factor used by the estimation */
    )
{
    int i, j;
    NurbsVector3 q;
    NurbsFloat dist;
    NurbsFloat best_dist = 1e10;	/* The initial value is for avoiding a warning */
    NurbsFloat u, v;
    int num_solutions = 0;
    int shift = 0, status;
    NurbsFloat mod;
    int fail_inversion = 1;

    /* Check the greatest component of the normal 
    * and shift the three equations to maximize precision */
    mod = absf(normal->x);
    if (absf(normal->y) > mod){
        shift = 1;
        mod = absf(normal->y);
    }
    if (absf(normal->z) > mod){
        shift = 2;
    }

    *pu = -1;
    *pv = -1;

    for (i = 1; i < surface_order2->cp_length_u; i++){
        for (j = 1; j < surface_order2->cp_length_v; j++){
            /* Calculate the local intersection */
            status = solve_local_inversion_normal_order2
                ( &u, &v, surface_order2, normal, i, j
                , point, shift, rgamma, epsilon);

            if (status == 0){
                /* Perform the iterative method */
                status = nurbs_surface_inversion_proj(&u, &v, point
                , surface_orig, epsilon);

                /* If (status != 0) failed to converge */

                q = nurbs_surface_get_point(surface_orig, u, v);
                dist = (q.x - point->x) * (q.x - point->x) 
                + (q.y - point->y) * (q.y - point->y) 
                + (q.z - point->z) * (q.z - point->z);

                if (num_solutions == 0){
                    fail_inversion = 0;
                    *pu = u;
                    *pv = v;
                    best_dist = dist;
                }
                else{
                    if (dist < best_dist){
                        *pu = u;
                        *pv = v;
                        best_dist = dist;
                    }
                }

                num_solutions++;
            }
        }
    }

    /* Check that the final value is not outside of the knot interval */
    if (fail_inversion == 0){
        if (*pu < surface_orig->knot_u[0]){
            *pu = surface_orig->knot_u[0];
        }
        if (*pu > surface_orig->knot_u[surface_orig->knot_length_u-1]){
            *pu = surface_orig->knot_u[surface_orig->knot_length_u-1];
        }
        if (*pv < surface_orig->knot_v[0]){
            *pv = surface_orig->knot_v[0];
        }
        if (*pv > surface_orig->knot_v[surface_orig->knot_length_v-1]){
            *pv = surface_orig->knot_v[surface_orig->knot_length_v-1];
        }
    }

    return num_solutions;
}

/* Calculates the inversion in a local interval 
 * by the surface minimun distance method */
static int solve_local_inversion_min_dist_order2
    ( NurbsFloat* pu                /* Returned solution */
    , NurbsFloat* pv                /* Returned solution */
    , const NurbsSurface* surface   /* Reduced second order nurbs */
    , const int i                   /* Interval index for the u direction */
    , const int j                   /* Interval index for the v direction */
    , const NurbsVector3* point     /* Vertex to be inverted */
    , const NurbsFloat rgamma       /* Relaxing factor  */
    )
{
    int it;

    NurbsFloat gi0j0_x = surface->cp[i][j].x 
                       / surface->cp[i][j].w;
    NurbsFloat gi0j0_y = surface->cp[i][j].y 
                       / surface->cp[i][j].w;
    NurbsFloat gi0j0_z = surface->cp[i][j].z 
                       / surface->cp[i][j].w;
    NurbsFloat gi0j1_x = surface->cp[i][j-1].x 
                       / surface->cp[i][j-1].w;
    NurbsFloat gi0j1_y = surface->cp[i][j-1].y 
                       / surface->cp[i][j-1].w;
    NurbsFloat gi0j1_z = surface->cp[i][j-1].z 
                       / surface->cp[i][j-1].w;
    NurbsFloat gi1j0_x = surface->cp[i-1][j].x 
                       / surface->cp[i-1][j].w;
    NurbsFloat gi1j0_y = surface->cp[i-1][j].y 
                       / surface->cp[i-1][j].w;
    NurbsFloat gi1j0_z = surface->cp[i-1][j].z 
                       / surface->cp[i-1][j].w;
    NurbsFloat gi1j1_x = surface->cp[i-1][j-1].x 
                       / surface->cp[i-1][j-1].w;
    NurbsFloat gi1j1_y = surface->cp[i-1][j-1].y 
                       / surface->cp[i-1][j-1].w;
    NurbsFloat gi1j1_z = surface->cp[i-1][j-1].z 
                       / surface->cp[i-1][j-1].w;

    NurbsFloat u0 = surface->knot_u[i];
    NurbsFloat u1 = surface->knot_u[i+1];
    NurbsFloat v0 = surface->knot_v[j];
    NurbsFloat v1 = surface->knot_v[j+1];

    NurbsFloat phi = u0*v0 + u1*v1 - u0*v1 - u1*v0;

    NurbsFloat ax = (gi0j0_x - gi0j1_x - gi1j0_x + gi1j1_x) / phi; 
    NurbsFloat ay = (gi0j0_y - gi0j1_y - gi1j0_y + gi1j1_y) / phi;  
    NurbsFloat az = (gi0j0_z - gi0j1_z - gi1j0_z + gi1j1_z) / phi;  

    NurbsFloat bx = (-gi0j0_x*v0 + gi0j1_x*v1 + gi1j0_x*v0 - gi1j1_x*v1) / phi;  
    NurbsFloat by = (-gi0j0_y*v0 + gi0j1_y*v1 + gi1j0_y*v0 - gi1j1_y*v1) / phi;  
    NurbsFloat bz = (-gi0j0_z*v0 + gi0j1_z*v1 + gi1j0_z*v0 - gi1j1_z*v1) / phi;  

    NurbsFloat cx = (-gi0j0_x*u0 + gi0j1_x*u0 + gi1j0_x*u1 - gi1j1_x*u1) / phi;  
    NurbsFloat cy = (-gi0j0_y*u0 + gi0j1_y*u0 + gi1j0_y*u1 - gi1j1_y*u1) / phi;  
    NurbsFloat cz = (-gi0j0_z*u0 + gi0j1_z*u0 + gi1j0_z*u1 - gi1j1_z*u1) / phi;  

    NurbsFloat dx = (gi0j0_x*u0*v0 - gi0j1_x*u0*v1 - gi1j0_x*u1*v0 
                  + gi1j1_x*u1*v1) / phi;  
    NurbsFloat dy = (gi0j0_y*u0*v0 - gi0j1_y*u0*v1 - gi1j0_y*u1*v0 
                  + gi1j1_y*u1*v1) / phi;  
    NurbsFloat dz = (gi0j0_z*u0*v0 - gi0j1_z*u0*v1 - gi1j0_z*u1*v0 
                  + gi1j1_z*u1*v1) / phi;  
    
    NurbsFloat Px = point->x;
    NurbsFloat Py = point->y;
    NurbsFloat Pz = point->z;

    /* Initial values */
    NurbsFloat u = (u0 + u1) / 2;
    NurbsFloat v = (v0 + v1) / 2;
 
    NurbsFloat rfx, rfy, rfz, ruvx, ruvy, ruvz;
    NurbsFloat fuv, fu, fv, f1, f2;
    NurbsFloat dd, du, dv;

   /* It always converges in less than 4 iterations */
    for (it = 0; it < 4; it++){
        rfx = (ax*u*v + bx*u + cx*v + dx - Px);
        rfy = (ay*u*v + by*u + cy*v + dy - Py);
        rfz = (az*u*v + bz*u + cz*v + dz - Pz);

        f1 = (ax*v + bx)*rfx + (ay*v + by)*rfy + (az*v + bz)*rfz;
        f2 = (ax*u + cx)*rfx + (ay*u + cy)*rfy + (az*u + cz)*rfz;

        ruvx = 2*ax*ax*u*v + 2*ax*cx*v + 2*ax*bx*u + ax*dx + bx*cx - ax*Px;
        ruvy = 2*ay*ay*u*v + 2*ay*cy*v + 2*ay*by*u + ay*dy + by*cy - ay*Py;
        ruvz = 2*az*az*u*v + 2*az*cz*v + 2*az*bz*u + az*dz + bz*cz - az*Pz;

        fuv = ruvx + ruvy + ruvz;

        fu = (ax*v + bx) * (ax*v + bx) 
           + (ay*v + by) * (ay*v + by) 
           + (az*v + bz) * (az*v + bz);

        fv = (ax*u + cx) * (ax*u + cx) 
           + (ay*u + cy) * (ay*u + cy) 
           + (az*u + cz) * (az*u + cz);

        dd = fu*fv - fuv*fuv;

        du = (f1 * fv - fuv * f2) / dd;
        dv = (f2 * fu - fuv * f1) / dd;

        u = u - du;
        v = v - dv;
    }

    if ( u >= u0 - (u1-u0)*rgamma && u <= u1 + (u1-u0)*rgamma
      && v >= v0 - (v1-v0)*rgamma && v <= v1 + (v1-v0)*rgamma ){
          *pu = u;
          *pv = v;
          return 0;
    }

    return 1;
}


/* Estimates the inversion by calculating the surface minimun distance 
 * using with an equivalent second order nurbs. */
int nurbs_surface_estimation_min_distance
    ( NurbsFloat *pu                     /* Returned value */
    , NurbsFloat *pv                     /* Returned value */
    , const NurbsSurface *surface_orig   /* Non reduced NURBS surface        */
    , const NurbsSurface *surface_order2 /* Second order NURBS surface        */
    , const NurbsVector3 *ppoint  /* Coordinates of the point {x, y, z} */
    , const NurbsFloat rgamma     /* Relaxing factor (eg 0, 0.25, 0.50) */
    )
{
    int i, j;
    NurbsVector3 q, point;
    NurbsFloat dist;
    NurbsFloat dist_best = 1e10;	/* The initial value for avoiding a warning */
    NurbsFloat u, v;

    int status, num_solutions = 0;

    point.x = ppoint->x;
    point.y = ppoint->y;
    point.z = ppoint->z;

    *pu = -1;
    *pv = -1;

    for (i = 2; i < surface_order2->cp_length_u - 1; i++){
        for (j = 2; j < surface_order2->cp_length_v - 1; j++){
            /* Calculate the local inversion */
            status = solve_local_inversion_min_dist_order2
            (&u, &v, surface_order2, i, j, &point, rgamma);

            if (status == 0){
                q = nurbs_surface_get_point(surface_orig, u, v);
                dist = (q.x - point.x) * (q.x - point.x) 
                    + (q.y - point.y) * (q.y - point.y) 
                    + (q.z - point.z) * (q.z - point.z);

                if (num_solutions == 0){
                    *pu = u;
                    *pv = v;
                    dist_best = dist;
                }
                else{
                    if (dist < dist_best){
                        *pu = u;
                        *pv = v;
                        dist_best = dist;
                    }
                }

                num_solutions++;
            }
        }
    }

    /* Borders are more relaxed */
    j = 1;
    for (i = 1; i < surface_order2->cp_length_u; i++){
        status = solve_local_inversion_min_dist_order2
        (&u, &v, surface_order2, i, j, &point, rgamma*10);

        if (status == 0){
            q = nurbs_surface_get_point(surface_orig, u, v);
            dist = (q.x - point.x) * (q.x - point.x) 
                + (q.y - point.y) * (q.y - point.y) 
                + (q.z - point.z) * (q.z - point.z);

            if (num_solutions == 0){
                *pu = u;
                *pv = v;
                dist_best = dist;
            }
            else{
                if (dist < dist_best){
                    *pu = u;
                    *pv = v;
                    dist_best = dist;
                }
            }

            num_solutions++;
        }
    }

    j = surface_order2->cp_length_v - 1;
    for (i = 1; i < surface_order2->cp_length_u; i++){
        status = solve_local_inversion_min_dist_order2
        (&u, &v, surface_order2, i, j, &point, rgamma*10);

        if (status == 0){
            q = nurbs_surface_get_point(surface_orig, u, v);
            dist = (q.x - point.x) * (q.x - point.x) 
                + (q.y - point.y) * (q.y - point.y) 
                + (q.z - point.z) * (q.z - point.z);

            if (num_solutions == 0){
                *pu = u;
                *pv = v;
                dist_best = dist;
            }
            else{
                if (dist < dist_best){
                    *pu = u;
                    *pv = v;
                    dist_best = dist;
                }
            }

            num_solutions++;
        }
    }


    i = 1;
    for (j = 1; j < surface_order2->cp_length_v; j++){
        status = solve_local_inversion_min_dist_order2
        (&u, &v, surface_order2, i, j, &point, rgamma*10);

        if (status == 0){
            q = nurbs_surface_get_point(surface_orig, u, v);
            dist = (q.x - point.x) * (q.x - point.x) 
                + (q.y - point.y) * (q.y - point.y) 
                + (q.z - point.z) * (q.z - point.z);

            if (num_solutions == 0){
                *pu = u;
                *pv = v;
                dist_best = dist;
            }
            else{
                if (dist < dist_best){
                    *pu = u;
                    *pv = v;
                    dist_best = dist;
                }
            }

            num_solutions++;
        }
    }


    i = surface_order2->cp_length_u - 1;
    for (j = 1; j < surface_order2->cp_length_v; j++){
        status = solve_local_inversion_min_dist_order2
        (&u, &v, surface_order2, i, j, &point, rgamma*10);

        if (status == 0){
            q = nurbs_surface_get_point(surface_orig, u, v);
            dist = (q.x - point.x) * (q.x - point.x) 
                + (q.y - point.y) * (q.y - point.y) 
                + (q.z - point.z) * (q.z - point.z);

            if (num_solutions == 0){
                *pu = u;
                *pv = v;
                dist_best = dist;
            }
            else{
                if (dist < dist_best){
                    *pu = u;
                    *pv = v;
                    dist_best = dist;
                }
            }

            num_solutions++;
        }
    }


    return num_solutions;
}

/* Calculates the inversion by calculating the surface minimun distance 
 * using with an equivalent second order nurbs; then apply an iterative method
 * returns the number of estimations found. */
int nurbs_surface_inversion_min_distance
    ( NurbsFloat *pu                     /* Returned value */
    , NurbsFloat *pv                     /* Returned value */
    , const NurbsSurface *surface_orig   /* Non reduced NURBS surface        */
    , const NurbsSurface *surface_order2 /* Second order NURBS surface        */
    , const NurbsVector3 *point  /* Coordinates of the point {x, y, z} */
    , const NurbsFloat epsilon   /* Stop condition iterative methods (eg 1e-6) */
    , const NurbsFloat rgamma    /* Relaxing factor (eg 0, 0.25, 0.50) */
    )
{
    int i, j;
    NurbsVector3 q;
    NurbsFloat dist;
    NurbsFloat best_dist = -1;
    NurbsFloat u, v;

    int status = 1, num_solutions = 0;

    *pu = -1;
    *pv = -1;

    for (i = 1; i < surface_order2->cp_length_u; i++){
        for (j = 1; j < surface_order2->cp_length_v; j++){
            /* Calculate the local estimation */
            status = solve_local_inversion_min_dist_order2
                (&u, &v, surface_order2, i, j, point, rgamma);

            if (status == 0){
                /* Perform the iterative method */
                num_solutions++;
                if (surface_orig->degree_u > 1 || surface_orig->degree_v > 1){
                    status = nurbs_surface_inversion_proj
                        (&u, &v, point, surface_orig, epsilon);
                }
                /* If (status != 0){ Failed to converge, but maybe has some improvement } */
                /* Check the error */
                q = nurbs_surface_get_point(surface_orig, u, v);
                dist = (q.x - point->x) * (q.x - point->x) 
                    + (q.y - point->y) * (q.y - point->y) 
                    + (q.z - point->z) * (q.z - point->z);

                if (dist < best_dist || best_dist < 0){
                    *pu = u;
                    *pv = v;
                    best_dist = dist;
                }

                if (sqrt(dist) < epsilon){
                    return 0;
                }
            }
        }
    }

    return (num_solutions > 0 ? 0 : 1);
}

