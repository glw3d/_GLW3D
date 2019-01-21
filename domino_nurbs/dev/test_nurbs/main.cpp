/**
Author: Mario J. Martin <dominonurbs$gmail.com>

How the Bernstein basis coefficients are calculated

*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "common/log.h"

#include "domino_nurbs/domino_nurbs.h"



void check_nurbs_cilinder()
{
    const double pi = 4*atan( 1.0 );
    int n;
    NurbsSurface* cilinder = nurbs_surface_import_ascii( "../test/Cilinder.NURBS", &n );
    NurbsFloat u0 = cilinder->knot_u[0];
    NurbsFloat u1 = cilinder->knot_u[cilinder->knot_length_u - 1];
    NurbsFloat v0 = cilinder->knot_v[0];
    NurbsFloat v1 = cilinder->knot_v[cilinder->knot_length_v - 1];

    for (NurbsFloat t = v0; t <= v1; t += (v1 - v0) / 100){
        NurbsVector3 p = nurbs_surface_get_point( cilinder, t, 0 );
        double r2 = (p.x*p.x + p.y*p.y + p.z*p.z);
        // Must be 1.0
        printf( "\nr:%g", r2 );   

        NurbsVector3 du, dv;
        nurbs_surface_get_derivatives( &du, &dv, &p, cilinder, t, 0 );
        printf( " \td:{%g %g}", du.y / du.x, tan( pi / 2 + ((t - v0) / v1)*pi * 2 ) );
    }
}

void check_nurbs_sphere()
{
    const double pi = 4 * atan( 1.0 );
    int n;
    NurbsSurface* sphere = nurbs_surface_import_ascii( "../test/data/Sphere.NURBS", &n );
    NurbsFloat u0 = sphere->knot_u[0];
    NurbsFloat u1 = sphere->knot_u[sphere->knot_length_u - 1];
    NurbsFloat v0 = sphere->knot_v[0];
    NurbsFloat v1 = sphere->knot_v[sphere->knot_length_v - 1];

    for (NurbsFloat u = u0; u <= u1; u += (u1 - u0) / 100){
        for (NurbsFloat v = v0; v <= v1; v += (v1 - v0) / 100){
            NurbsVector3 p = nurbs_surface_get_point( sphere, u, v );
            double r2 = (p.x*p.x + p.y*p.y + p.z*p.z);
            // Must be 1.0
            printf( "\nr:%g", r2 );
        }
    }
}


void check_basis_recursive()
{
    NurbsCurve* nurbs = nurbs_curve_alloc( nullptr, 7, 3 );
    NurbsFloat basis[7];
    int i;

    nurbs->knot[0] = 0;
    nurbs->knot[1] = 0;
    nurbs->knot[2] = 0;
    nurbs->knot[3] = 0;
    nurbs->knot[4] = 1;
    nurbs->knot[5] = 2;
    nurbs->knot[6] = 3;
    nurbs->knot[7] = 4;
    nurbs->knot[8] = 4;
    nurbs->knot[9] = 4;
    nurbs->knot[10] = 4;

    for (double t = -0.05; t <= 4.05; t += 1. / 50){
        NurbsVector3 point_nurbs;

        point_nurbs = nurbs_curve_get_point( nurbs, t );

        _debug_( "\n [%.4f]", t );

        basis[0] = 0;
        basis[1] = 0;
        basis[2] = 0;
        basis[3] = 0;
        basis[4] = 0;
        basis[5] = 0;
        basis[6] = 0;

        i = nurbs_basis_knot_index( t, nurbs->knot, nurbs->knot_length );
        if (i < 0){
            basis[0] = 1;
        }
        else if (i >= nurbs->knot_length - 1){
            basis[nurbs->knot_length - nurbs->degree - 2] = 1;
        }
        else{
            basis[i] = nurbs_basis_term( nurbs->knot, t, nurbs->degree, i, 0 );
            basis[i - 1] = nurbs_basis_term( nurbs->knot, t, nurbs->degree, i, 1 );
            basis[i - 2] = nurbs_basis_term( nurbs->knot, t, nurbs->degree, i, 2 );
            basis[i - 3] = nurbs_basis_term( nurbs->knot, t, nurbs->degree, i, 3 );
        }

        _debug_( " {%.4f %.4f}", nurbs->basis[0], basis[0] );
        _debug_( " {%.4f %.4f}", nurbs->basis[1], basis[1] );
        _debug_( " {%.4f %.4f}", nurbs->basis[2], basis[2] );
        _debug_( " {%.4f %.4f}", nurbs->basis[3], basis[3] );
        _debug_( " {%.4f %.4f}", nurbs->basis[4], basis[4] );
        _debug_( " {%.4f %.4f}", nurbs->basis[5], basis[5] );
        _debug_( " {%.4f %.4f}", nurbs->basis[6], basis[6] );
    }
}

void draw_basis()
{
    NurbsCurve* nurbs = nurbs_curve_alloc(nullptr, 7, 2);

    int index = 0;
    int knot = 0;
    for (int i = 0; i <= nurbs->degree; i++){
        nurbs->knot[index] = knot;
        index++;
    }
    knot++;
    for (int i = 0; i < nurbs->knot_length - 2*nurbs->degree - 2; i++){
        nurbs->knot[index] = knot; 
        knot++;
        index++;
    }
    for (int i = 0; i <= nurbs->degree; i++){
        nurbs->knot[index] = knot; 
        index++;
    }
    for (int i = 0; i < nurbs->knot_length; i++){
        printf( "\n%g", nurbs->knot[i] );
    }
    int c = 0;
    for (double t = 0; t <= nurbs->knot[nurbs->knot_length-1]; t += 0.1){
        NurbsVector3 p = nurbs_curve_get_point( nurbs, t );
        _debug_( "\n %f", t );
        for (int i = 0; i < nurbs->cp_length; i++){
            _debug_( " %f,", nurbs->basis[i] );
        }
    }
}


int main(int argc, char *argv[])
{
    //check_nurbs_cilinder();
    //check_nurbs_sphere();
    //check_basis_recursive();
    //draw_basis();

    getchar();

    return 0;
}

