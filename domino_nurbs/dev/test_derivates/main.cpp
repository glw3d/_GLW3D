#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common/log.h"

#include "domino_nurbs/domino_nurbs.h"
#include "domino_nurbs/nurbs_basis.h"

void check_performance()
{
    int num_nurbs;
    NurbsSurface* nurbs_array = nurbs_surface_import_ascii("../test/f6.NURBS", &num_nurbs);
    NurbsSurface* nurbs = &nurbs_array[28];

    NurbsVector3 p, dpu, dpv;
    NurbsFloat u0, u1, v0, v1, u, v;

    clock_t t0, t1;
    
    nurbs_basis_get_parameter_interval(&u0, &u1, nurbs->knot_u, nurbs->knot_length_u, nurbs->degree_u);
    nurbs_basis_get_parameter_interval(&v0, &v1, nurbs->knot_v, nurbs->knot_length_v, nurbs->degree_v);

    t0 = clock();
    for (u = u0; u < u1; u += (u1-u0)/256){
        for (v = v0; v < v1; v += (v1-v0)/256){
            nurbs_surface_get_derivatives(&dpu, &dpv, &p, nurbs,u, v);
        }
    }
    t1 = clock();

    _debug_("%i\n", t1-t0);

    nurbs_surface_free(nurbs_array, num_nurbs);
}

void check_basis_derivates_bernstein()
{
    NurbsCurve* nurbs = nurbs_curve_alloc(nullptr, 4, 3);
    NurbsFloat bernstein[5];
    NurbsFloat d_bernstein[5];

    nurbs->knot[0] = 0;
    nurbs->knot[1] = 0;
    nurbs->knot[2] = 0;
    nurbs->knot[3] = 0;
    nurbs->knot[4] = 1;
    nurbs->knot[5] = 1;
    nurbs->knot[6] = 1;
    nurbs->knot[7] = 1;

    for (double t = -0.05; t <= 1.05; t += 1./50){
        NurbsVector3 point;
        NurbsVector3 deriv;

        nurbs_curve_get_derivatives(&point, &deriv, nurbs, t);
        
        _debug_("\n\n[%.4f]", t);

        bernstein[0] = 0;
        bernstein[1] = 0;
        bernstein[2] = 0;
        bernstein[3] = 0;
        bernstein[4] = 0;

        d_bernstein[0] = 0;
        d_bernstein[1] = 0;
        d_bernstein[2] = 0;
        d_bernstein[3] = 0;
        d_bernstein[4] = 0;

        if (t < 0){
          bernstein[0] = 1;
        }
        else if (t > 1){
          bernstein[4] = 1;
        }
        else {
            bernstein[0] = (1-t)*(1-t)*(1-t);
            bernstein[1] = 3*t*(1-t)*(1-t);
            bernstein[2] = 3*t*t*(1-t);
            bernstein[3] = t*t*t;

            d_bernstein[0] = -3 + 6*t - 3*t*t;
            d_bernstein[1] = 3 - 12*t + 9*t*t;
            d_bernstein[2] = 6*t - 9*t*t;
            d_bernstein[3] = 3*t*t;
        }

        _debug_( "\n{%.4f %.4f} {%.4f %.4f}", nurbs->basis[0], bernstein[0], nurbs->d_basis[0], d_bernstein[0] );
        _debug_( "\n{%.4f %.4f} {%.4f %.4f}", nurbs->basis[1], bernstein[1], nurbs->d_basis[1], d_bernstein[1] );
        _debug_( "\n{%.4f %.4f} {%.4f %.4f}", nurbs->basis[2], bernstein[2], nurbs->d_basis[2], d_bernstein[2] );
        _debug_( "\n{%.4f %.4f} {%.4f %.4f}", nurbs->basis[3], bernstein[3], nurbs->d_basis[3], d_bernstein[3] );
    }

    nurbs_curve_free(nurbs, 1);
}


void check_basis_second_derivates()
{
    NurbsSurface* surface_array;
    NurbsSurface* surface;
    int num_nurbs;

    surface_array = nurbs_surface_import_ascii("../test/data/Surface7.NURBS", &num_nurbs);
    surface = &(surface_array[0]);
    
    NurbsFloat u0, u1, v0, v1;
    nurbs_basis_get_parameter_interval
        (&u0, &u1, surface->knot_u, surface->knot_length_u, surface->degree_u);

    nurbs_basis_get_parameter_interval
        (&v0, &v1, surface->knot_v, surface->knot_length_v, surface->degree_v);
    
    //for (float u = u0; u <= u1; u += (u1-u0)/100){
    //    nurbs_basis_function(surface->basis_u, u, surface->degree_u
    //        , surface->knot_u, surface->knot_length_u, surface->basis_k1u);
    //    trace("\n");
    //    for (int i = 0; i < surface->knot_length_u; i++){
    //        trace(" %#f", surface->basis_u[i]);
    //    }
    //}
    NurbsFloat* u_left = (NurbsFloat*)malloc(sizeof(NurbsFloat) * surface->knot_length_u);
    NurbsFloat* u_right = (NurbsFloat*)malloc(sizeof(NurbsFloat) * surface->knot_length_u);
    NurbsFloat* u_center = (NurbsFloat*)malloc(sizeof(NurbsFloat) * surface->knot_length_u);
    NurbsFloat* d2_basis_u = (NurbsFloat*)malloc(sizeof(NurbsFloat) * surface->knot_length_u);
    NurbsFloat* d2_basis_k1_u = (NurbsFloat*)malloc(sizeof(NurbsFloat) * surface->knot_length_u);

    for (NurbsFloat u = u0; u <= u1; u += (u1-u0)/100){
        nurbs_basis_second_derivate_function
            ( d2_basis_u, surface->d_basis_u, surface->basis_u
            , u, surface->degree_u
            , surface->knot_u, surface->knot_length_u);

        _debug_("\n\n");
        for (int i = 0; i < surface->knot_length_u; i++){
            _debug_( " %#f", d2_basis_u[i] );
        }

        _debug_( "\n" );

        nurbs_basis_function(u_center, u, surface->degree_u
            , surface->knot_u, surface->knot_length_u);

        nurbs_basis_function(u_left, u-((u1-u0)/1000), surface->degree_u
            , surface->knot_u, surface->knot_length_u);

        nurbs_basis_function(u_right, u+((u1-u0)/1000), surface->degree_u
            , surface->knot_u, surface->knot_length_u);

        for (int i = 0; i < surface->knot_length_u; i++){
            _debug_( " %#f", (u_right[i] + u_left[i] - 2 * u_center[i]) * 1000 * 1000 );
        }
    }

    nurbs_surface_free(surface_array, num_nurbs);

}

void check_surface_derivates()
{
    NurbsSurface* surface_array;
    NurbsSurface* surface;
    int num_nurbs;

    surface_array = nurbs_surface_import_ascii("../test/data/Surface7.NURBS", &num_nurbs);
    surface = &(surface_array[0]);
    
    NurbsFloat u0, u1, v0, v1;
    nurbs_basis_get_parameter_interval
        (&u0, &u1, surface->knot_u, surface->knot_length_u, surface->degree_u);

    nurbs_basis_get_parameter_interval
        (&v0, &v1, surface->knot_v, surface->knot_length_v, surface->degree_v);
    
    NurbsVector3 left_u;
    NurbsVector3 right_u;
    NurbsVector3 left_v;
    NurbsVector3 right_v;
    NurbsVector3 center;
    NurbsVector3 d1_u;
    NurbsVector3 d1_v;
    NurbsVector3 point;

    for (NurbsFloat u = u0; u <= u1; u += (u1-u0)/10){
        for (NurbsFloat v = v0; v <= v1; v += (v1-v0)/10){
            left_u = nurbs_surface_get_point(surface, u - (u1-u0)/1000, v);
            left_v = nurbs_surface_get_point(surface, u, v - (v1-v0)/1000);
            right_u = nurbs_surface_get_point(surface, u + (u1-u0)/1000, v);
            right_v = nurbs_surface_get_point(surface, u, v + (v1-v0)/1000);
            center = nurbs_surface_get_point(surface, u, v);
            nurbs_surface_get_derivatives(&d1_u, &d1_v, &point, surface, u, v);
            _debug_( "\n" );
            _debug_( " {%#f, %#f} {%#f, %#f} {%#f, %#f} {%#f, %#f} {%#f, %#f} {%#f, %#f}"
                , ((right_u.x - left_u.x)*1000)/2, d1_u.x
                , ((right_u.y - left_u.y)*1000)/2, d1_u.y
                , ((right_u.z - left_u.z)*1000)/2, d1_u.z
                , ((right_v.x - left_v.x)*1000)/2, d1_v.x
                , ((right_v.y - left_v.y)*1000)/2, d1_v.y
                , ((right_v.z - left_v.z)*1000)/2, d1_v.z
                );
        }
    }

    nurbs_surface_free(surface_array, num_nurbs);
}

void check_surface_second_derivates()
{
    NurbsSurface* surface_array;
    NurbsSurface* surface;
    int num_nurbs;

    surface_array = nurbs_surface_import_ascii("../test/data/Surface7.NURBS", &num_nurbs);
    surface = &(surface_array[0]);
    
    NurbsFloat u0, u1, v0, v1;
    nurbs_basis_get_parameter_interval
        (&u0, &u1, surface->knot_u, surface->knot_length_u, surface->degree_u);

    nurbs_basis_get_parameter_interval
        (&v0, &v1, surface->knot_v, surface->knot_length_v, surface->degree_v);
    
    NurbsVector3 left_u;
    NurbsVector3 right_u;
    NurbsVector3 left_v;
    NurbsVector3 right_v;
    NurbsVector3 center;
    NurbsVector3 du;
    NurbsVector3 dv;
    NurbsVector3 duu;
    NurbsVector3 duv;
    NurbsVector3 dvv;
    NurbsVector3 point;
    NurbsVector3 fu1v1;
    NurbsVector3 fu0v0;
    NurbsVector3 fu1v0;
    NurbsVector3 fu0v1;

    for (NurbsFloat u = u0; u <= u1; u += (u1-u0)/10){
        for (NurbsFloat v = v0; v <= v1; v += (v1-v0)/10){
            left_u = nurbs_surface_get_point(surface, u - (u1-u0)/1000, v);
            left_v = nurbs_surface_get_point(surface, u, v - (v1-v0)/1000);
            right_u = nurbs_surface_get_point(surface, u + (u1-u0)/1000, v);
            right_v = nurbs_surface_get_point(surface, u, v + (v1-v0)/1000);
            center = nurbs_surface_get_point(surface, u, v);
            fu1v1 = nurbs_surface_get_point(surface, u + (u1-u0)/1000, v + (v1-v0)/1000);
            fu0v0 = nurbs_surface_get_point(surface, u - (u1-u0)/1000, v - (v1-v0)/1000);
            fu1v0 = nurbs_surface_get_point(surface, u + (u1-u0)/1000, v - (v1-v0)/1000);
            fu0v1 = nurbs_surface_get_point(surface, u - (u1-u0)/1000, v + (v1-v0)/1000);
            nurbs_surface_get_second_derivatives(&duu, &duv, &dvv, &du, &dv, &point, surface, u, v);
            _debug_( "\n" );
            _debug_( " {%.3f, %.3f} {%.3f, %.3f} {%.3f, %.3f} {%.3f, %.3f} {%.3f, %.3f} {%.3f, %.3f} {%.3f, %.3f} {%.3f, %.3f} {%.3f, %.3f}"
                , ((right_u.x + left_u.x - 2*center.x) * 1000 * 1000), duu.x
                , ((right_u.y + left_u.y - 2*center.y) * 1000 * 1000), duu.y
                , ((right_u.z + left_u.z - 2*center.z) * 1000 * 1000), duu.z
                , ((fu1v1.x + fu0v0.x - fu1v0.x - fu0v1.x) * 250 * 1000), duv.x
                , ((fu1v1.y + fu0v0.y - fu1v0.y - fu0v1.y) * 250 * 1000), duv.y
                , ((fu1v1.z + fu0v0.z - fu1v0.z - fu0v1.z) * 250 * 1000), duv.z
                , ((right_v.x + left_v.x - 2*center.x) * 1000 * 1000), dvv.x
                , ((right_v.y + left_v.y - 2*center.y) * 1000 * 1000), dvv.y
                , ((right_v.z + left_v.z - 2*center.z) * 1000 * 1000), dvv.z
                );
        }
    }

    nurbs_surface_free(surface_array, num_nurbs);
}

void check_basis_second_derivatives_bernstein()
{
    NurbsCurve* nurbs = nurbs_curve_alloc(nullptr, 4, 3);
    NurbsFloat bernstein[5];
    NurbsFloat d_bernstein[5];
    NurbsFloat d2_bernstein[5];
    NurbsFloat d2_basis[8];
    NurbsFloat* basis = nurbs->basis;
    NurbsFloat* d_basis = nurbs->d_basis;

    nurbs->knot[0] = 0;
    nurbs->knot[1] = 0;
    nurbs->knot[2] = 0;
    nurbs->knot[3] = 0;
    nurbs->knot[4] = 1;
    nurbs->knot[5] = 1;
    nurbs->knot[6] = 1;
    nurbs->knot[7] = 1;

    for (double t = -0.05; t <= 1.05; t += 1./50){

        nurbs_basis_second_derivate_function
            (d2_basis, nurbs->d_basis, nurbs->basis, t
            , nurbs->degree, nurbs->knot, nurbs->knot_length);

        _debug_( "\n\n[%.4f]", t );

        bernstein[0] = 0;
        bernstein[1] = 0;
        bernstein[2] = 0;
        bernstein[3] = 0;
        bernstein[4] = 0;

        d_bernstein[0] = 0;
        d_bernstein[1] = 0;
        d_bernstein[2] = 0;
        d_bernstein[3] = 0;
        d_bernstein[4] = 0;

        d2_bernstein[0] = 0;
        d2_bernstein[1] = 0;
        d2_bernstein[2] = 0;
        d2_bernstein[3] = 0;
        d2_bernstein[4] = 0;

        if (t < 0){
          bernstein[0] = 1;
        }
        else if (t > 1){
          bernstein[4] = 1;
        }
        else {
            bernstein[0] = (1-t)*(1-t)*(1-t);
            bernstein[1] = 3*t*(1-t)*(1-t);
            bernstein[2] = 3*t*t*(1-t);
            bernstein[3] = t*t*t;

            d_bernstein[0] = -3 + 6*t - 3*t*t;
            d_bernstein[1] = 3 - 12*t + 9*t*t;
            d_bernstein[2] = 6*t - 9*t*t;
            d_bernstein[3] = 3*t*t;

            d2_bernstein[0] = 6 - 6*t;
            d2_bernstein[1] = -12 + 18*t;
            d2_bernstein[2] = 6 - 18*t;
            d2_bernstein[3] = 6*t;
        }
        _debug_( "\nbasis:" );
        _debug_( "\n{%.4f %.4f}", basis[0], bernstein[0] );
        _debug_( "\n{%.4f %.4f}", basis[1], bernstein[1] );
        _debug_( "\n{%.4f %.4f}", basis[2], bernstein[2] );
        _debug_( "\n{%.4f %.4f}", basis[3], bernstein[3] );

        _debug_( "\nd_basis:" );
        _debug_( "\n{%.4f %.4f}", d_basis[0], d_bernstein[0] );
        _debug_( "\n{%.4f %.4f}", d_basis[1], d_bernstein[1] );
        _debug_( "\n{%.4f %.4f}", d_basis[2], d_bernstein[2] );
        _debug_( "\n{%.4f %.4f}", d_basis[3], d_bernstein[3] );

        _debug_( "\nd2_basis:" );
        _debug_( "\n{%.4f %.4f}", d2_basis[0], d2_bernstein[0] );
        _debug_( "\n{%.4f %.4f}", d2_basis[1], d2_bernstein[1] );
        _debug_( "\n{%.4f %.4f}", d2_basis[2], d2_bernstein[2] );
        _debug_( "\n{%.4f %.4f}", d2_basis[3], d2_bernstein[3] );
    }

    nurbs_curve_dispose( nurbs );
}

void calculate_curvature()
{
    int num_surfaces;
    NurbsSurface* surface = nurbs_surface_import_ascii("../test/data/Surface6.NURBS", &num_surfaces);
    
    NurbsFloat u0, u1, v0, v1;
    nurbs_basis_get_parameter_interval
        (&u0, &u1, surface->knot_u, surface->knot_length_u, surface->degree_u);
    nurbs_basis_get_parameter_interval
        (&v0, &v1, surface->knot_v, surface->knot_length_v, surface->degree_v);

    NurbsVector3 du, dv, duu, dvv, duv, point;

    //for (NurbsFloat i = v0; i < v1; i += (v1-v0)/100){
    //    nurbs_surface_get_derivatives(du, dv, point, surface, u0, i);
    //    trace("\nu:%f v:%f \n du:{%f, %f, %f} dv:{%f, %f, %f} \n p:{%f, %f, %f}", u0, i
    //        , du[0], du[1], du[2], dv[0], dv[1], dv[2], point[0], point[1], point[2]);

    //    nurbs_surface_get_point(point, surface, u0, i);
    //    trace(" p:{%f, %f, %f}\n", point[0], point[1], point[2]);
    //}

    for (NurbsFloat i = v0; i < v1; i += (v1-v0)/100){
        nurbs_surface_get_second_derivatives(&duu, &duv, &dvv, &du, &dv, &point, surface, u0, i);
        _debug_( "\nu:%f v:%f \n duu:{%f, %f, %f} dvv:{%f, %f, %f} duv:{%f, %f, %f}", u0, i
            , duu.x, duu.y, duu.z, dvv.x, dvv.y, dvv.z, duv.x, duv.y, duv.z);
    }

    nurbs_surface_free(surface, num_surfaces);
}

int main(int argc, char *argv[])
{
    check_surface_derivates();
    //check_basis_derivates_bernstein();
    //check_basis_second_derivates();
    //check_surface_second_derivates();
    //calculate_curvature();
    //check_performance();
    //check_basis_second_derivatives_bernstein();
    //check_surface_second_derivates();

    getchar();
#ifdef _INC_CRTDBG  
    /* Microsoft's memory leak check */
    _CrtDumpMemoryLeaks();
#endif

	return 0;
}

