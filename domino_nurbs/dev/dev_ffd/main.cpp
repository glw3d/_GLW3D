
#include <memory.h>
#include <malloc.h>
#include <math.h>

#include "common/log.h"
#include "common/check_malloc.h"
#include "domino_nurbs/nurbs_curve.h"

static int fac( const int n ){
    if (n <= 0){
        return 1;
    }

    int i = 2;
    int f = 1;
    for (; i <= n; i++){
        f *= i;
    }

    return f;
}

NurbsFloat nurbs_freeform_basis( const NurbsFloat t, const int ncps, const int k )
{
    double t1 = (1 - t);
    double fn, fk, fnk;
    double tn, t1n, coeff, basis = 0;

    if (ncps <= 1){
        return 0;
    }

    fn = fac( ncps - 1 );
    fk = fac( k );
    fnk = fac( ncps - k - 1 );

    coeff = fn / (fk * fnk);
    tn = pow( t, k );
    t1n = pow( t1, (ncps - k - 1) );
    basis = coeff * tn * t1n;

    return basis;
}

int nurbs_freeform_basis_function( NurbsFloat basis[], const NurbsFloat t, const int ncps )
{
    int k;
    double t1 = (1 - t);
    double fn, fk, fnk;
    double tn, t1n, coeff;

    if (ncps <= 1){
        return 0;
    }

    if (t < 0){
        basis[0] = 1;
        for (k = 1; k < ncps; k++){
            basis[k] = 0;
        }
        return 0;
    }
    else if (t > 1){
        basis[ncps - 1] = 1;
        for (k = 0; k < ncps-1; k++){
            basis[k] = 0;
        }
        return ncps - 1;
    }

    fn = fac( ncps - 1 );
    for (k = 0; k < ncps; k++){
        fk = fac( k );
        fnk = fac( ncps - k - 1 );

        coeff = fn / (fk * fnk);
        tn = pow( t, k );
        t1n = pow( t1, (ncps - k - 1) );
        basis[k] = coeff * tn * t1n;
    }

    return int(t * ncps);
}

void check_basis_01()
{
    NurbsFloat berns_2[2] = { 0 };
    NurbsFloat basis_2[2] = { 0 };
    NurbsFloat ffd_2[2] = { 0 };

    for (NurbsFloat t = -0.1f; t < 1.11f; t += 0.05f){
        berns_2[0] = (1 - t);
        berns_2[1] = t;

        basis_2[0] = nurbs_freeform_basis( t, 2, 0 );
        basis_2[1] = nurbs_freeform_basis( t, 2, 1 );

        nurbs_freeform_basis_function( ffd_2, t, 2 );

        _debug_( "\n%f  {%f %f %f}  {%f %f %f}   %f"
            , t
            , berns_2[0], basis_2[0], ffd_2[0]
            , berns_2[1], basis_2[1], ffd_2[1]
            , basis_2[0] + basis_2[1]
            );
    }

    /**************************************************************************/
    _debug_( "\n\n" );
    NurbsFloat berns_3[3] = { 0 };
    NurbsFloat basis_3[3] = { 0 };
    NurbsFloat ffd_3[3] = { 0 };

    for (NurbsFloat t = -0.1f; t < 1.11f; t += 0.05f){
        berns_3[0] = (1 - t)*(1 - t);
        berns_3[1] = 2 * t*(1 - t);
        berns_3[2] = t*t;

        basis_3[0] = nurbs_freeform_basis( t, 3, 0 );
        basis_3[1] = nurbs_freeform_basis( t, 3, 1 );
        basis_3[2] = nurbs_freeform_basis( t, 3, 2 );

        nurbs_freeform_basis_function( ffd_3, t, 3 );
        
        _debug_( "\n%f  {%f %f %f}  {%f %f %f}  {%f %f %f}    %f"
            , t
            , berns_3[0], basis_3[0], ffd_3[0]
            , berns_3[1], basis_3[1], ffd_3[1]
            , berns_3[2], basis_3[2], ffd_3[2]
            , basis_3[0] + basis_3[1] + basis_3[2]
        );
    }

    /**************************************************************************/
    _debug_( "\n\n" );
    NurbsFloat berns_4[4] = {0};
    NurbsFloat basis_4[4] = {0};
    NurbsFloat ffd_4[4] = { 0 };

    for (NurbsFloat t = 0; t < 1; t += 0.05f){
        berns_4[3] = t*t*t;
        berns_4[2] = 3*t*t*(1-t);
        berns_4[1] = 3*t*(1-t)*(1-t);
        berns_4[0] = (1-t)*(1-t)*(1-t);

        basis_4[0] = nurbs_freeform_basis( t, 4, 0 );
        basis_4[1] = nurbs_freeform_basis( t, 4, 1 );
        basis_4[2] = nurbs_freeform_basis( t, 4, 2 );
        basis_4[3] = nurbs_freeform_basis( t, 4, 3 );

        nurbs_freeform_basis_function( ffd_4, t, 4 );

        _debug_( "\n%f ", t );
        _debug_( "  {%f %f %f}", berns_4[0], basis_4[0], ffd_4[0] );
        _debug_( "  {%f %f %f}", berns_4[1], basis_4[1], ffd_4[1] );
        _debug_( "  {%f %f %f}", berns_4[2], basis_4[2], ffd_4[2] );
        _debug_( "  {%f %f %f}", berns_4[3], basis_4[3], ffd_4[3] );
        _debug_( "  %f", berns_4[0] + basis_4[1] + basis_4[2] + basis_4[3] );
    }

    /**************************************************************************/
    _debug_( "\n\n" );
    NurbsFloat berns_5[5] = {0};
    NurbsFloat basis_5[5] = { 0 };
    NurbsFloat ffd_5[5] = { 0 };

    for (NurbsFloat t = 0; t < 1; t += 0.05f){
        berns_5[4] = t*t*t*t;
        berns_5[3] = 4 * t*t*t*(1 - t);
        berns_5[2] = 6 * t*t*(1 - t)*(1 - t);
        berns_5[1] = 4 * t*(1 - t)*(1 - t)*(1 - t);
        berns_5[0] = (1 - t)*(1 - t)*(1 - t)*(1 - t);

        basis_5[0] = nurbs_freeform_basis( t, 5, 0 );
        basis_5[1] = nurbs_freeform_basis( t, 5, 1 );
        basis_5[2] = nurbs_freeform_basis( t, 5, 2 );
        basis_5[3] = nurbs_freeform_basis( t, 5, 3 );
        basis_5[4] = nurbs_freeform_basis( t, 5, 4 );

        nurbs_freeform_basis_function( ffd_5, t, 5 );

        _debug_( "\n%f ", t );
        _debug_( "  {%f %f %f}", berns_5[0], basis_5[0], ffd_5[0] );
        _debug_( "  {%f %f %f}", berns_5[1], basis_5[1], ffd_5[1] );
        _debug_( "  {%f %f %f}", berns_5[2], basis_5[2], ffd_5[2] );
        _debug_( "  {%f %f %f}", berns_5[3], basis_5[3], ffd_5[3] );
        _debug_( "  {%f %f %f}", berns_5[4], basis_5[4], ffd_5[4] );
        _debug_( "  %f", berns_5[0] + berns_5[1] + berns_5[2] + berns_5[3] + berns_5[4] );
    }
}


void compare_nurbs_ffd_basis()
{
    NurbsCurve* curve = nurbs_curve_alloc(nullptr, 4, 3);

    NurbsFloat ffd[4];

    curve->knot[0] = 0;
    curve->knot[1] = 0;
    curve->knot[2] = 0;
    curve->knot[3] = 0;
    curve->knot[4] = 1;
    curve->knot[5] = 1;
    curve->knot[6] = 1;
    curve->knot[7] = 1;

    NurbsVector3 p;
    for (NurbsFloat t = -0.1; t < 1.1; t += 0.05f){
        p = nurbs_curve_get_point(curve, t);
        nurbs_freeform_basis_function(ffd, t, 4);

        _debug_("\n%f", t);
        _debug_( "\n   %f %f", curve->basis[0], ffd[0] );
        _debug_( "\n   %f %f", curve->basis[1], ffd[1] );
        _debug_( "\n   %f %f", curve->basis[2], ffd[2] );
        _debug_( "\n   %f %f", curve->basis[3], ffd[3] );
    }
}

//void check_derivatives()
//{
//    NurbsFloat berns[3];
//    NurbsFloat d_berns[3];
//    NurbsFloat basis[3];
//    NurbsFloat d_basis[3];
//    NurbsFloat nurbs[7];
//    NurbsFloat d_nurbs[7];
//
//    NurbsCurve* curve = nurbs_curve_init(nullptr, 3, 2);
//    curve->knot[0] = 0;
//    curve->knot[2] = 0;
//    curve->knot[3] = 0;
//    curve->knot[4] = 1;
//    curve->knot[5] = 1;
//    curve->knot[6] = 1;
//
//    for (NurbsFloat t = -0.1f; t < 1.11f; t += 0.05f){
//        berns[2] = t*t;
//        berns[1] = 2*t*(1-t);
//        berns[0] = (1-t)*(1-t);
//
//        d_berns[2] = 2*t;
//        d_berns[1] = 2-4*t;
//        d_berns[0] = -2+2*t;
//
//        freeform_derivative_basis_function(d_basis, basis, t, 3, 2);
//        nurbs_basis_derivate_function(d_nurbs, nurbs, t, 2, curve->knot, 7);
//
//        trace("\n\n[%f]", t);
//        trace("\n%f %f %f", berns[0], nurbs[1], basis[0]);
//        trace("\n%f %f %f", berns[1], nurbs[2], basis[1]);
//        trace("\n%f %f %f", berns[2], nurbs[3], basis[2]);
//        trace("\n%f %f %f", d_berns[0], d_nurbs[1], d_basis[0]);
//        trace("\n%f %f %f", d_berns[1], d_nurbs[2], d_basis[1]);
//        trace("\n%f %f %f", d_berns[2], d_nurbs[3], d_basis[2]);
//    }
//}
//
//
//
//void check_get_point()
//{
//    FreeForm* ffd = freeform_init(nullptr, 5, 2, 1, 0, 1, 0);
//
//    ffd->cp[0][0][0].x = 0;
//    ffd->cp[0][0][0].y = 0;
//    ffd->cp[0][0][0].z = 0;
//
//    ffd->cp[1][0][0].x = 0.25;
//    ffd->cp[1][0][0].y = 0;
//    ffd->cp[1][0][0].z = 0;
//
//    ffd->cp[2][0][0].x = 0.5;
//    ffd->cp[2][0][0].y = 0;
//    ffd->cp[2][0][0].z = 0;
//
//    ffd->cp[3][0][0].x = 0.75;
//    ffd->cp[3][0][0].y = 0;
//    ffd->cp[3][0][0].z = 0;
//
//    ffd->cp[4][0][0].x = 1;
//    ffd->cp[4][0][0].y = 0;
//    ffd->cp[4][0][0].z = 0;
//
//    NurbsCurve* nurbs = nurbs_curve_init(nullptr, 5, 2);
//
//    nurbs->cp[0].x = 0;
//    nurbs->cp[0].y = 0;
//    nurbs->cp[0].z = 0;
//
//    nurbs->cp[1].x = 0.25;
//    nurbs->cp[1].y = 0;
//    nurbs->cp[1].z = 0;
//
//    nurbs->cp[2].x = 0.5;
//    nurbs->cp[2].y = 0;
//    nurbs->cp[2].z = 0;
//
//    nurbs->cp[3].x = 0.75;
//    nurbs->cp[3].y = 0;
//    nurbs->cp[3].z = 0;
//
//    nurbs->cp[4].x = 1;
//    nurbs->cp[4].y = 0;
//    nurbs->cp[4].z = 0;
//
//    nurbs->knot[0] = 0;
//    nurbs->knot[1] = 0;
//    nurbs->knot[2] = 0;
//    nurbs->knot[3] = 1;
//    nurbs->knot[4] = 2;
//    nurbs->knot[5] = 3;
//    nurbs->knot[6] = 3;
//    nurbs->knot[7] = 3;
//
//
//    for (NurbsFloat t = -0.11f; t < 1.11f; t += 0.05f){
//        NurbsVector3 p0 = freeform_get_point(ffd, t, 0, 0);
//        NurbsVector3 p1 = nurbs_curve_get_point(nurbs, t*3);
//  
//        trace("\n%f {%f %f} {%f %f} {%f %f}", t
//            , p0.x, p1.x, p0.y, p1.y, p0.z, p1.z); 
//    }
//}
//
//void check_get_inversion()
//{
//    FreeForm ffd;
//    NurbsCurve curve;
//    int failed;
//
//    freeform_init(&ffd, 5, 2, 1, 0, 1, 0);
//
//    ffd.cp[0][0][0].x = 0;
//    ffd.cp[0][0][0].y = 0;
//    ffd.cp[0][0][0].z = 0;
//
//    ffd.cp[1][0][0].x = 0.25;
//    ffd.cp[1][0][0].y = 0;
//    ffd.cp[1][0][0].z = 0;
//
//    ffd.cp[2][0][0].x = 0.5;
//    ffd.cp[2][0][0].y = 0;
//    ffd.cp[2][0][0].z = 0;
//
//    ffd.cp[3][0][0].x = 0.75;
//    ffd.cp[3][0][0].y = 0;
//    ffd.cp[3][0][0].z = 0;
//
//    ffd.cp[4][0][0].x = 1;
//    ffd.cp[4][0][0].y = 0;
//    ffd.cp[4][0][0].z = 0;
//
//    nurbs_curve_init(&curve, 5, 2);
//    
//    curve.knot[0] = 0;
//    curve.knot[1] = 0;
//    curve.knot[2] = 0;
//    curve.knot[3] = 1.0f/3;
//    curve.knot[4] = 2.0f/3;
//    curve.knot[5] = 1;
//    curve.knot[6] = 1;
//    curve.knot[7] = 1;
//
//    curve.cp[0].x = 0;
//    curve.cp[0].y = 0;
//    curve.cp[0].z = 0;
//
//    curve.cp[1].x = 0.25f;
//    curve.cp[1].y = 0;
//    curve.cp[1].z = 0;
//
//    curve.cp[2].x = 0.5f;
//    curve.cp[2].y = 0;
//    curve.cp[2].z = 0;
//
//    curve.cp[3].x = 0.75f;
//    curve.cp[3].y = 0;
//    curve.cp[3].z = 0;
//
//    curve.cp[4].x = 1;
//    curve.cp[4].y = 0;
//    curve.cp[4].z = 0;
//
//    for (int i = 0; i < 101; i++){
//        NurbsFloat t = (NurbsFloat)i / 100;
//        NurbsVector3 p = freeform_get_point(&ffd, t, 0, 0);
//        NurbsVector3 pn = nurbs_curve_get_point(&curve, t);
//
//        NurbsVector3 tuvw = {t, 0, 0};
//        NurbsVector3 t1 = freeform_inversion(&ffd, tuvw, 1e-8f, &failed);
//        NurbsVector3 p1 = freeform_get_point(&ffd, t1.x, 0, 0);
//
//        trace("\n %f %f %f %f", t, pn.x, p.x, p1.x);
//    }
//}
//
//
//void check_get_inversion2()
//{
//    NurbsVector3 shape[128];
//    NurbsVector3 t[128];
//    FreeForm ffd;
//    const double pi = atan(1.)*4;
//    int failed;
//
//    for (int i = 0; i < 128; i++){
//        double alpha = 2*pi * ((double)i/127);
//        shape[i].x = (NurbsFloat)(sin(alpha) / 2 + 0.5);
//        shape[i].y = (NurbsFloat)(cos(alpha) / 2 + 0.5);
//        shape[i].z = 0;
//    }
//
//    freeform_init(&ffd, 5, 2, 5, 2, 1, 0);
//
//    /***/
//    ffd.cp[0][0][0].x = 0;
//    ffd.cp[0][0][0].y = 0;
//    ffd.cp[0][0][0].z = 0;
//
//    ffd.cp[1][0][0].x = 0.25;
//    ffd.cp[1][0][0].y = 0;
//    ffd.cp[1][0][0].z = 0;
//
//    ffd.cp[2][0][0].x = 0.5;
//    ffd.cp[2][0][0].y = 0;
//    ffd.cp[2][0][0].z = 0;
//
//    ffd.cp[3][0][0].x = 0.75;
//    ffd.cp[3][0][0].y = 0;
//    ffd.cp[3][0][0].z = 0;
//
//    ffd.cp[4][0][0].x = 1.0;
//    ffd.cp[4][0][0].y = 0;
//    ffd.cp[4][0][0].z = 0;
//
//    /***/
//    ffd.cp[0][1][0].x = 0;
//    ffd.cp[0][1][0].y = 0.25;
//    ffd.cp[0][1][0].z = 0;
//
//    ffd.cp[1][1][0].x = 0.25;
//    ffd.cp[1][1][0].y = 0.25;
//    ffd.cp[1][1][0].z = 0;
//
//    ffd.cp[2][1][0].x = 0.5;
//    ffd.cp[2][1][0].y = 0.25;
//    ffd.cp[2][1][0].z = 0;
//
//    ffd.cp[3][1][0].x = 0.75;
//    ffd.cp[3][1][0].y = 0.25;
//    ffd.cp[3][1][0].z = 0;
//
//    ffd.cp[4][1][0].x = 1.0;
//    ffd.cp[4][1][0].y = 0.25;
//    ffd.cp[4][1][0].z = 0;
//
//    /***/
//    ffd.cp[0][2][0].x = 0;
//    ffd.cp[0][2][0].y = 0.5;
//    ffd.cp[0][2][0].z = 0;
//
//    ffd.cp[1][2][0].x = 0.25;
//    ffd.cp[1][2][0].y = 0.5;
//    ffd.cp[1][2][0].z = 0;
//
//    ffd.cp[2][2][0].x = 0.5;
//    ffd.cp[2][2][0].y = 0.5;
//    ffd.cp[2][2][0].z = 0;
//
//    ffd.cp[3][2][0].x = 0.75;
//    ffd.cp[3][2][0].y = 0.5;
//    ffd.cp[3][2][0].z = 0;
//
//    ffd.cp[4][2][0].x = 1.0;
//    ffd.cp[4][2][0].y = 0.5;
//    ffd.cp[4][2][0].z = 0;
//
//    /***/
//    ffd.cp[0][3][0].x = 0;
//    ffd.cp[0][3][0].y = 0.75;
//    ffd.cp[0][3][0].z = 0;
//
//    ffd.cp[1][3][0].x = 0.25;
//    ffd.cp[1][3][0].y = 0.75;
//    ffd.cp[1][3][0].z = 0;
//
//    ffd.cp[2][3][0].x = 0.5;
//    ffd.cp[2][3][0].y = 0.75;
//    ffd.cp[2][3][0].z = 0;
//
//    ffd.cp[3][3][0].x = 0.75;
//    ffd.cp[3][3][0].y = 0.75;
//    ffd.cp[3][3][0].z = 0;
//
//    ffd.cp[4][3][0].x = 1.0;
//    ffd.cp[4][3][0].y = 0.75;
//    ffd.cp[4][3][0].z = 0;
//
//    /***/
//    ffd.cp[0][4][0].x = 0;
//    ffd.cp[0][4][0].y = 1.0;
//    ffd.cp[0][4][0].z = 0;
//
//    ffd.cp[1][4][0].x = 0.25;
//    ffd.cp[1][4][0].y = 1.0;
//    ffd.cp[1][4][0].z = 0;
//
//    ffd.cp[2][4][0].x = 0.5;
//    ffd.cp[2][4][0].y = 1.0;
//    ffd.cp[2][4][0].z = 0;
//
//    ffd.cp[3][4][0].x = 0.75;
//    ffd.cp[3][4][0].y = 1.0;
//    ffd.cp[3][4][0].z = 0;
//
//    ffd.cp[4][4][0].x = 1.0;
//    ffd.cp[4][4][0].y = 1.0;
//    ffd.cp[4][4][0].z = 0;
//
//    for (int i = 0; i < 128; i++){
//        NurbsFloat u = shape[i].x;
//        NurbsFloat v = shape[i].y;
//        NurbsFloat w = 0;
//
//        t[i] = freeform_inversion(&ffd, shape[i], 1e-8f, &failed);
//        NurbsVector3 p = freeform_get_point(&ffd, t[i].x, t[i].y, t[i].z);
//
//        trace("\n {%f %f} {%f %f} {%f %f}", u, p.x, v, p.y, w, p.z);
//    }
//
//    /* transformation */
//    trace("\n");
//    double alpha = 2*pi / 6;
//
//    for (int i = 0; i < 128; i++){
//        shape[i].x = (NurbsFloat)(shape[i].x * sin(alpha) + shape[i].y * cos(alpha) + 1.0);
//        shape[i].y = (NurbsFloat)(shape[i].x * cos(alpha) - shape[i].y * sin(alpha) + 2.0);
//        shape[i].z += 1.0f;
//
//        shape[i].x *= 2;
//        shape[i].y *= 3;
//    }
//
//    for (int i = 0; i < ffd.cp_length_u; i++){
//        for (int j = 0; j < ffd.cp_length_v; j++){
//            for (int k = 0; k < ffd.cp_length_w; k++){
//                ffd.cp[i][j][k].x = (NurbsFloat)(ffd.cp[i][j][k].x * sin(alpha) + ffd.cp[i][j][k].y * cos(alpha) + 1.0);
//                ffd.cp[i][j][k].y = (NurbsFloat)(ffd.cp[i][j][k].x * cos(alpha) - ffd.cp[i][j][k].y * sin(alpha) + 2.0);
//                ffd.cp[i][j][k].z += 1.0f;
//
//                ffd.cp[i][j][k].x *= 2;
//                ffd.cp[i][j][k].y *= 3;
//            }
//        }
//    }
//
//    for (int i = 0; i < 128; i++){
//        NurbsVector3 p = freeform_get_point(&ffd, t[i].x, t[i].y, t[i].z);
//
//        trace("\n {%f %f} {%f %f} {%f %f}"
//            , shape[i].x, p.x
//            , shape[i].y, p.y
//            , shape[i].z, p.z
//            );
//    }
//
//}


int main()
{
    //check_basis_01();
    compare_nurbs_ffd_basis();

    //check_get_point();
    //check_derivatives();
    //check_get_inversion();
    //check_get_inversion2();
    //check_io();

    getchar();
    return 0;
}

