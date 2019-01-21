
%module domino_nurbs
%include "typemaps.i"

%{
#define SWIG_FILE_WITH_INIT
#include "domino_nurbs/nurbs_definitions.h"
#include "domino_nurbs/domino_nurbs.h"
#include "domino_nurbs/nurbs_py_tools.h"
#include <exception>
%}

%include "../domino_nurbs/nurbs_definitions.h"
%include "../domino_nurbs/nurbs_curve_data.h"
%include "../domino_nurbs/nurbs_surface_data.h"
%include "../domino_nurbs/nurbs_controlbox_data.h"
%include "../domino_nurbs/nurbs_curve.h"
%include "../domino_nurbs/nurbs_surface.h"
%include "../domino_nurbs/nurbs_controlbox.h"
%include "../domino_nurbs/nurbs_io.h"
%include "../domino_nurbs/nurbs_py_tools.h"


/* Calculates the range where the nurbs parameters are defined */
void nurbs_basis_get_parameter_interval
    ( NurbsFloat* OUTPUT   /* (out) u0 */
    , NurbsFloat* OUTPUT   /* (out) u1 */
    , const NurbsFloat* knot_vector
    , const int knot_length
    , const int degree
    );

%extend _NurbsVector3 
{
    inline _NurbsVector3* __getitem__(const size_t i){
        return NurbsVector3_getItem($self, i);
    }
    inline void __setitem__(const size_t i, const _NurbsVector3& v){
        self[i] = v;
    }
}

%extend _NurbsVector4
{
    inline _NurbsVector4* __getitem__(const size_t i){
        return NurbsVector4_getItem($self, i);
    }
    inline void __setitem__(const size_t i, const _NurbsVector4& v){
        self[i] = v;
    }
}

%extend NurbsCurve_
{
    inline NurbsCurve* __getitem__(size_t i) {
        return NurbsCurve_getItem($self, i);
    }
}

%extend NurbsSurface_
{
    inline NurbsSurface* __getitem__(size_t i) {
        return NurbsSurface_getItem($self, i);
    }
}

%extend NurbsControlBox_
{
    inline NurbsControlBox* __getitem__(size_t i) {
        return NurbsControlBox_getItem($self, i);
    }
}

