 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    Algorithms to calculate the intersection between NURBS surface.

*******************************************************************************/ 


#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include "nurbs_internal.h"

#include "nurbs_basis.h"
#include "nurbs_surface.h"


/* Takes out the sign of a number  */
#define absf(a) ((a > 0) ? a : -a)

/* Solves an triperhiperbolic equation in the form 
 * a*x*y + b*x + c*y + d*z + e = 0 
 * This is the same as is found in nurbs_surface_inversion.c */
static int solve_hiperbolic_three(NurbsFloat* u, NurbsFloat* v
    , const NurbsFloat A1, const NurbsFloat A2, const NurbsFloat A3
    , const NurbsFloat B1, const NurbsFloat B2, const NurbsFloat B3
    , const NurbsFloat C1, const NurbsFloat C2, const NurbsFloat C3
    , const NurbsFloat D1, const NurbsFloat D2, const NurbsFloat D3
    , const NurbsFloat E1, const NurbsFloat E2, const NurbsFloat E3)
{
    NurbsFloat a1, a2, a3;
    NurbsFloat b1, b2, b3;
    NurbsFloat c1, c2, c3;
    NurbsFloat d1, d2, d3;
    NurbsFloat det, cu, cv;

    a1 = A2*B1-A1*B2;
    b1 = A2*C1-A1*C2;
    c1 = A2*D1-A1*D2;
    d1 = A2*E1-A1*E2;

    a2 = A3*B2-A2*B3;
    b2 = A3*C2-A2*C3;
    c2 = A3*D2-A2*D3;
    d2 = A3*E2-A2*E3;

    a3 = A1*B3-A3*B1;
    b3 = A1*C3-A3*C1;
    c3 = A1*D3-A3*D1;
    d3 = A1*E3-A3*E1;

    det = a1*b2*c3 + a2*b3*c1 + a3*b1*c2 - a1*b3*c2 - a2*b1*c3 - a3*b2*c1;
    if (absf(det) < FLT_EPSILON){
        a1 = B1;
        a2 = B2;
        a3 = B3;
        b1 = C1;
        b2 = C2;
        b3 = C3;
        c1 = D1;
        c2 = D2;
        c3 = D3;
        d1 = E1;
        d2 = E2;
        d3 = E3;

        det = a1*b2*c3 + a2*b3*c1 + a3*b1*c2 - a1*b3*c2 - a2*b1*c3 - a3*b2*c1;
    }

    if (absf(det) < FLT_EPSILON){
        // there is no solution
        return 1;
    }

    cu = d1*b2*c3 + d2*b3*c1 + d3*b1*c2 - d1*b3*c2 - d2*b1*c3 - d3*b2*c1;
    cv = a1*d2*c3 + a2*d3*c1 + a3*d1*c2 - a1*d3*c2 - a2*d1*c3 - a3*d2*c1;

    *u = -cu / det;
    *v = -cv / det;

    return 0;
}

/* Gets the first nurbs derivative using finite differences */
static void get_nurbs_derivates_byfd_u
    ( NurbsVector3 *du      /* (out) Derivative */
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
    ( NurbsVector3 *dv     /* (out) Derivative */
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

/******************************************************************************/

/* Local data structure that store the polinomials coefficients */
typedef struct 
{
    NurbsFloat u0, u1, v0, v1;

    NurbsFloat uvx, uvy, uvz;
    NurbsFloat uux, uuy, uuz;
    NurbsFloat vvx, vvy, vvz;

    NurbsFloat Ix, Iy, Iz;
}Nurbs2Coef;

/* polinomial coefficients of the reduced second order NURBS */
static Nurbs2Coef nurbs_polinomial_coef
    ( const NurbsSurface* s, const int i, const int j )
{
    Nurbs2Coef coef;

    NurbsFloat wa00 = s->cp[i][j].w;
    NurbsFloat wa11 = s->cp[i-1][j-1].w;
    NurbsFloat wa01 = s->cp[i][j-1].w;
    NurbsFloat wa10 = s->cp[i-1][j].w;

    NurbsFloat g00x = s->cp[i][j].x * wa00;
    NurbsFloat g00y = s->cp[i][j].y * wa00;
    NurbsFloat g00z = s->cp[i][j].z * wa00;
    NurbsFloat g01x = s->cp[i][j-1].x * wa01;
    NurbsFloat g01y = s->cp[i][j-1].y * wa01;
    NurbsFloat g01z = s->cp[i][j-1].z * wa01;
    NurbsFloat g10x = s->cp[i-1][j].x * wa10;
    NurbsFloat g10y = s->cp[i-1][j].y * wa10;
    NurbsFloat g10z = s->cp[i-1][j].z * wa10;
    NurbsFloat g11x = s->cp[i-1][j-1].x * wa11;
    NurbsFloat g11y = s->cp[i-1][j-1].y * wa11;
    NurbsFloat g11z = s->cp[i-1][j-1].z * wa11;

    NurbsFloat phi_uv;

    coef.u0 = s->knot_u[i];
    coef.u1 = s->knot_u[i+1];
    coef.v0 = s->knot_v[j];
    coef.v1 = s->knot_v[j+1];

    phi_uv = coef.u0*coef.v0 + coef.u1*coef.v1 - coef.u0*coef.v1 - coef.u1*coef.v0;

    coef.uvx = (g11x + g00x - g01x - g10x) / phi_uv;
    coef.uvy = (g11y + g00y - g01y - g10y) / phi_uv;
    coef.uvz = (g11z + g00z - g01z - g10z) / phi_uv;

    coef.uux = (coef.v0*g10x + coef.v1*g01x - coef.v0*g00x - coef.v1*g11x) / phi_uv;
    coef.uuy = (coef.v0*g10y + coef.v1*g01y - coef.v0*g00y - coef.v1*g11y) / phi_uv;
    coef.uuz = (coef.v0*g10z + coef.v1*g01z - coef.v0*g00z - coef.v1*g11z) / phi_uv;

    coef.vvx = (coef.u1*g10x + coef.u0*g01x - coef.u0*g00x - coef.u1*g11x) / phi_uv;
    coef.vvy = (coef.u1*g10y + coef.u0*g01y - coef.u0*g00y - coef.u1*g11y) / phi_uv;
    coef.vvz = (coef.u1*g10z + coef.u0*g01z - coef.u0*g00z - coef.u1*g11z) / phi_uv;

    coef.Ix = (coef.u1*coef.v1*g11x + coef.u0*coef.v0*g00x - coef.u1*coef.v0*g10x - coef.u0*coef.v1*g01x) / phi_uv;
    coef.Iy = (coef.u1*coef.v1*g11y + coef.u0*coef.v0*g00y - coef.u1*coef.v0*g10y - coef.u0*coef.v1*g01y) / phi_uv;
    coef.Iz = (coef.u1*coef.v1*g11z + coef.u0*coef.v0*g00z - coef.u1*coef.v0*g10z - coef.u0*coef.v1*g01z) / phi_uv;

    return coef;
}

/* Analytical solution of the intersection between two second order nurbs */ 
static int solve_intersec_m_cte
    ( NurbsFloat* pua, NurbsFloat* pva, NurbsFloat* pna
    , const Nurbs2Coef* a, const Nurbs2Coef* b 
    , const NurbsFloat m
    )
{
    NurbsFloat A1 = a->uvx;
    NurbsFloat B1 = a->uux;
    NurbsFloat C1 = a->vvx;
    NurbsFloat D1 = -b->vvx - b->uvx * m;
    NurbsFloat E1 = a->Ix - b->uux * m - b->Ix;

    NurbsFloat A2 = a->uvy;
    NurbsFloat B2 = a->uuy;
    NurbsFloat C2 = a->vvy;
    NurbsFloat D2 = -b->vvy - b->uvy * m;
    NurbsFloat E2 = a->Iy - b->uuy * m - b->Iy;

    NurbsFloat A3 = a->uvz;
    NurbsFloat B3 = a->uuz;
    NurbsFloat C3 = a->vvz;
    NurbsFloat D3 = -b->vvz - b->uvz * m;
    NurbsFloat E3 = a->Iz - b->uuz * m - b->Iz;

    int status;
    NurbsFloat ua, va;

   status = solve_hiperbolic_three(pua, pva
        , A2, A1, A3, B2, B1, B3, C2, C1, C3, D2, D1, D3, E2, E1, E3);
   if (status == 0){
        ua = *pua;
        va = *pva;
        *pna = (-A2*ua*va-B2*ua-C2*va-E2)/D2;
   }

    return status;
}


/* Analytical solution of the intersection between two second order nurbs */ 
static int solve_intersec_n_cte
    ( NurbsFloat* pua,NurbsFloat* pva, NurbsFloat* pma
    , const Nurbs2Coef* a, const Nurbs2Coef* b
    , const NurbsFloat n
    )
{
    NurbsFloat A1 = a->uvx;
    NurbsFloat B1 = a->uux;
    NurbsFloat C1 = a->vvx;
    NurbsFloat D1 = -b->uux - b->uvx * n;
    NurbsFloat E1 = a->Ix - b->vvx * n - b->Ix;

    NurbsFloat A2 = a->uvy;
    NurbsFloat B2 = a->uuy;
    NurbsFloat C2 = a->vvy;
    NurbsFloat D2 = -b->uuy - b->uvy * n;
    NurbsFloat E2 = a->Iy - b->vvy * n - b->Iy;

    NurbsFloat A3 = a->uvz;
    NurbsFloat B3 = a->uuz;
    NurbsFloat C3 = a->vvz;
    NurbsFloat D3 = -b->uuz - b->uvz * n;
    NurbsFloat E3 = a->Iz - b->vvz*n - b->Iz;

    int status;
    NurbsFloat ua, va;

    status = solve_hiperbolic_three(pua, pva
        , A3, A2, A1, B3, B2, B1, C3, C2, C1, D3, D2, D1, E3, E2, E1);
    if (status == 0){
        ua = *pua;
        va = *pva;
        *pma = (-A3*ua*va-B3*ua-C3*va-E1)/D3;
    }

    return status;
}


/* Local intersection between two nurbs surface intervales with u = cte 
 * It only checks the middle point */
static void nurbs_surface_intersec_fast_u
    ( const Nurbs2Coef* a, const Nurbs2Coef* b
    , NurbsIntersection* sol, size_t* num_sol, const size_t sol_max_length
    , const NurbsFloat relax
    )
{
    NurbsFloat va, ma, na, u;
    int status;

    /* check u */

    u = (a->u0 + a->u1) / 2;
    status = solve_intersec_m_cte( &ma, &na, &va, b, a, u );
    
    if (status == 0 && *num_sol < sol_max_length-1){
        /* It is very unlikely that there are two different simultaneous solutions.
         * Most probably it is the same solution. */
        if (va >= a->v0-(a->v1-a->v0)*relax && va <= a->v1+(a->v1-a->v0)*relax 
            && ma >= b->u0-(b->u1-b->u0)*relax && ma <= b->u1+(b->u1-b->u0)*relax
            && na >= b->v0-(b->v1-b->v0)*relax && na <= b->v1+(b->v1-b->v0)*relax
        ){
            sol[*num_sol].u = u;
            sol[*num_sol].v = va;
            sol[*num_sol].m = ma;
            sol[*num_sol].n = na;
            (*num_sol)++;
         }
    }
}

/* Local intersection between two nurbs surface intervales with v = cte 
 * It only checks the middle point */
static void nurbs_surface_intersec_fast_v
    ( const Nurbs2Coef* a, const Nurbs2Coef* b
    , NurbsIntersection* sol, size_t* num_sol, const size_t sol_max_length
    , const NurbsFloat relax
    )
{
    NurbsFloat ua, ma, na, v;
    int status;

    /* check v */
    v = (a->v0 + a->v1) / 2;
    status = solve_intersec_n_cte( &ma, &na, &ua, b, a, v );
    
    if (status == 0 && *num_sol < sol_max_length-1){
        if (ua >= a->u0-(a->u1-a->u0)*relax && ua <= a->u1+(a->u1-a->u0)*relax
            && ma >= b->u0-(b->u1-b->u0)*relax && ma <= b->u1+(b->u1-b->u0)*relax
            && na >= b->v0-(b->v1-b->v0)*relax && na <= b->v1+(b->v1-b->v0)*relax
        ){
            sol[*num_sol].u = ua;
            sol[*num_sol].v = v;
            sol[*num_sol].m = ma;
            sol[*num_sol].n = na;
            (*num_sol)++;
         }
    }
}

/* Local intersection between two nurbs surface intervales with m = cte 
 * It only checks the middle point */
static void nurbs_surface_intersec_fast_m
    ( const Nurbs2Coef* a, const Nurbs2Coef* b
    , NurbsIntersection* sol, size_t* num_sol, const size_t sol_max_length
    , const NurbsFloat relax
    )
{
    NurbsFloat ua, va, na, m;
    int status;

    /* check m */

    m = (b->u0 + b->u1) / 2;
    status = solve_intersec_m_cte( &ua, &va, &na, a, b, m);
    
    if (status == 0 && *num_sol < sol_max_length-1){
        if (ua >= a->u0-(a->u1-a->u0)*relax && ua <= a->u1+(a->u1-a->u0)*relax
            && va >= a->v0-(a->v1-a->v0)*relax && va <= a->v1+(a->v1-a->v0)*relax 
            && na >= b->v0-(b->v1-b->v0)*relax && na <= b->v1+(b->v1-b->v0)*relax
        ){
            sol[*num_sol].u = ua;
            sol[*num_sol].v = va;
            sol[*num_sol].m = m;
            sol[*num_sol].n = na;
            (*num_sol)++;
         }
    }
}

/* Local intersection between two nurbs surface intervales with m = cte 
 * It only checks the middle point */
static void nurbs_surface_intersec_fast_n
    ( const Nurbs2Coef* a, const Nurbs2Coef* b
    , NurbsIntersection* sol, size_t* num_sol, const size_t sol_max_length
    , const NurbsFloat relax
    )
{
    NurbsFloat ua, va, ma, n;
    int status;

    /* check n */
    n = (b->v0 + b->v1) / 2;
    status = solve_intersec_n_cte(&ua, &va, &ma, a, b, n );
    
    if (status == 0 && *num_sol < sol_max_length-1){
        /* It is very unlikely that there are two different simultaneous solutions.
         * Most probably it is the same solution. */
        if (ua >= a->u0-(a->u1-a->u0)*relax && ua <= a->u1+(a->u1-a->u0)*relax
            && va >= a->v0-(a->v1-a->v0)*relax && va <= a->v1+(a->v1-a->v0)*relax 
            && ma >= b->u0-(b->u1-b->u0)*relax && ma <= b->u1+(b->u1-b->u0)*relax
        ){
            sol[*num_sol].u = ua;
            sol[*num_sol].v = va;
            sol[*num_sol].m = ma;
            sol[*num_sol].n = n;
            (*num_sol)++;
         }
    }
}

static void check_knot_limits(const NurbsSurface* nurbs, NurbsFloat* u, NurbsFloat* v)
{
    NurbsFloat u0 = nurbs->knot_u[nurbs->degree_u];
    NurbsFloat u1 = nurbs->knot_u[nurbs->knot_length_u - nurbs->degree_u - 1];
    NurbsFloat v0 = nurbs->knot_v[nurbs->degree_v];
    NurbsFloat v1 = nurbs->knot_v[nurbs->knot_length_v - nurbs->degree_v - 1];

    if (*u < u0){
        *u = u0;
    }
    else if (*u > u1){
        *u = u1;
    }

    if (*v < v0){
        *v = v0;
    }
    else if (*v > v1){
        *v = v1;
    }
}


/**  Refines the intersection with an iterative method */
void nurbs_surface_intersection_iterative
    ( NurbsFloat* u, NurbsFloat* v, NurbsFloat* m, NurbsFloat* n
    , const NurbsSurface* nurbs_a, const NurbsSurface* nurbs_b
    , const NurbsFloat epsilon)
{
    NurbsVector3 point_a;
    NurbsVector3 point_b;
    NurbsVector3 point1;
    NurbsVector3 point2;
    NurbsVector3 vec;
    NurbsVector3 du;
    NurbsVector3 dv;
    NurbsFloat dist, dist1, dist2, dist3;
    NurbsFloat qpu, qpv, u2, v2, uv, d, u0, v0, m0, n0;
    NurbsFloat delta_u, delta_v;

    point_a = nurbs_surface_get_point(nurbs_a, *u, *v);
    point_b = nurbs_surface_get_point(nurbs_b, *m, *n);

    vec.x = point_a.x - point_b.x;
    vec.y = point_a.y - point_b.y;
    vec.z = point_a.z - point_b.z;

    /* Initial error */
    dist = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    dist = (NurbsFloat)sqrt(dist);

    while (dist > epsilon){

        nurbs_surface_get_derivatives(&du, &dv, &point_a, nurbs_a, *u, *v);
        /* If the derivative is zero, recalculate it using finite diferences */
        u2 = du.x * du.x + du.y * du.y + du.z * du.z;
        if (u2 < FLT_EPSILON){
            get_nurbs_derivates_byfd_u(&du, *u, *v,  nurbs_a);
            u2 = du.x * du.x + du.y * du.y + du.z * du.z;
        }

        v2 = dv.x * dv.x + dv.y * dv.y + dv.z * dv.z;
        if (v2 < FLT_EPSILON){
            get_nurbs_derivates_byfd_v(&dv, *u, *v, nurbs_a);
            v2 = dv.x * dv.x + dv.y * dv.y + dv.z * dv.z;
        }

        /* Calculate the proyection onto the derivatives */
        qpu = vec.x * du.x + vec.y * du.y + vec.z * du.z;
        qpv = vec.x * dv.x + vec.y * dv.y + vec.z * dv.z;

        uv = du.x * dv.x + du.y * dv.y + du.z * dv.z;

        d = u2 * v2 - uv * uv;

        delta_u = (qpu * v2 - qpv * uv) / d;
        delta_v = (qpv * u2 - qpu * uv) / d;

        u0 = *u - delta_u/2;
        v0 = *v - delta_v/2;

        /*********/

        nurbs_surface_get_derivatives(&du, &dv, &point_b, nurbs_b, *m, *n);
        /* If the derivative is zero, recalculate it using finite diferences */
        u2 = du.x * du.x + du.y * du.y + du.z * du.z;
        if (u2 < FLT_EPSILON){
            get_nurbs_derivates_byfd_u(&du, *m, *n,  nurbs_b);
            u2 = du.x * du.x + du.y * du.y + du.z * du.z;
       }

        v2 = dv.x * dv.x + dv.y * dv.y + dv.z * dv.z;
        if (v2 < FLT_EPSILON){
            get_nurbs_derivates_byfd_v(&dv, *m, *n, nurbs_b);
            v2 = dv.x * dv.x + dv.y * dv.y + dv.z * dv.z;
        }

        /* Calculate the proyection onto the derivatives */
        qpu = vec.x * du.x + vec.y * du.y + vec.z * du.z;
        qpv = vec.x * dv.x + vec.y * dv.y + vec.z * dv.z;

        uv = du.x * dv.x + du.y * dv.y + du.z * dv.z;

        d = u2 * v2 - uv * uv;

        delta_u = (qpu * v2 - qpv * uv) / d;
        delta_v = (qpv * u2 - qpu * uv) / d;

        m0 = *m + delta_u/2;
        n0 = *n + delta_v/2;

        /*********/

        /* move through uv */
        point1 = nurbs_surface_get_point(nurbs_a, u0, v0);
        vec.x = point1.x - point_b.x;
        vec.y = point1.y - point_b.y;
        vec.z = point1.z - point_b.z;

        dist1 = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
        dist1 = (NurbsFloat)sqrt(dist1);

        /* move through mn */
        point2 = nurbs_surface_get_point(nurbs_b, m0, n0);
        vec.x = point_a.x - point2.x;
        vec.y = point_a.y - point2.y;
        vec.z = point_a.z - point2.z;

        dist2 = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
        dist2 = (NurbsFloat)sqrt(dist2);

        /* move through both */
        vec.x = point1.x - point2.x;
        vec.y = point1.y - point2.y;
        vec.z = point1.z - point2.z;

        dist3 = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
        dist3 = (NurbsFloat)sqrt(dist3);

        /*********/

        /* check the best */
        if (dist1 <= dist2 && dist1 <= dist3 && dist1 < dist){
            dist = dist1;
            *u = u0;
            *v = v0;
        }
        else if (dist2 <= dist1 && dist2 <= dist3 && dist2 < dist){
            dist = dist2;
            *m = m0;
            *n = n0;
        }
        else if (dist3 <= dist1 && dist3 <= dist2 && dist3 < dist){
            dist = dist3;
            *u = u0;
            *v = v0;
            *m = m0;
            *n = n0;
        }
        else{
            /* The solution does not converge */
            break;
        }
    }

    check_knot_limits(nurbs_a, u, v);
    check_knot_limits(nurbs_b, m, n);
}


/** Calculates an estimation of intersection between nurbs. 
  * This is a fast (and potentialy low accurate) algorithm. 
  * It only checks the middle point of each nurbs segment.
  * It performs a raster through all 4 possibilities.
  * Usually, the raster that provides more solutions is the best one.
  * Return 1 if these two nurbs may intersec or 0 if not */
int nurbs_surface_intersection_fast
    ( NurbsIntersection* buffer
    , size_t* num_sol
    , const size_t max_sol
    , const NurbsSurface* nurbs_a
    , const NurbsSurface* nurbs_b
    , NurbsSurface* nurbs_2order_a
    , NurbsSurface* nurbs_2order_b
    , const NurbsFloat relaxation
    )
{
    NurbsSurface* sa = nullptr;
    NurbsSurface* sb = nullptr;
    Nurbs2Coef a, b;
    int i, j, k, s;

    /* reduce both nurbs to second order */
    if (nurbs_2order_a == nullptr){
        sa = nurbs_surface_reduce_order2(nullptr, nurbs_a);
    }
    else{
        sa = nurbs_2order_a;
    }
    if (nurbs_2order_b == nullptr){
        sb = nurbs_surface_reduce_order2(nullptr, nurbs_b);
    }
    else{
        sb = nurbs_2order_b;
    }

    *num_sol = 0;

    /* Huge loop through all segments to see which may intersec */
    for (i = sa->degree_u; i < sa->knot_length_u - sa->degree_u - 1; i++){
        for (j = sa->degree_v; j < sa->knot_length_v - sa->degree_v - 1; j++){
            a = nurbs_polinomial_coef(sa, i, j);

            for (k = sb->degree_u; k < sb->knot_length_u - sb->degree_u - 1; k++){
                for (s = sb->degree_v; s < sb->knot_length_v - sb->degree_v - 1; s++){

                    b = nurbs_polinomial_coef(sb, k, s);
                
                    nurbs_surface_intersec_fast_u(&a, &b, buffer, num_sol, max_sol, relaxation);
                    nurbs_surface_intersec_fast_v(&a, &b, buffer, num_sol, max_sol, relaxation);
                    nurbs_surface_intersec_fast_m(&a, &b, buffer, num_sol, max_sol, relaxation);
                    nurbs_surface_intersec_fast_n(&a, &b, buffer, num_sol, max_sol, relaxation);
                }
            }
        }
    }

    if (*num_sol > 0){
        return 1;
    }
    else{
        return 0;
    }
}

