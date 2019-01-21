
#include "domino_nurbs/domino_nurbs.h"

int main(int argc, char *argv[])
{
    int num_nurbs;
    NurbsSurface* nurbs_f6_array = nurbs_surface_import_ascii( "../test/f6.NURBS", &num_nurbs );
    nurbs_surface_export_ascii( "f6_v21.NURBS", nurbs_f6_array, num_nurbs );

    NurbsSurface* nurbs_naca_array = nurbs_surface_import_ascii( "../test/naca0012_cp9_2nurbs.NURBS", &num_nurbs );
    nurbs_surface_export_ascii( "naca0012_cp9_2nurbs_v21.NURBS", nurbs_naca_array, num_nurbs );

    NurbsControlBox* cb = nurbs_controlbox_import_ascii( "../test/controlbox_1.nurbs", &num_nurbs );
    nurbs_controlbox_export_ascii( "cb_v21.NURBS", cb, num_nurbs );

    return 0;
}
