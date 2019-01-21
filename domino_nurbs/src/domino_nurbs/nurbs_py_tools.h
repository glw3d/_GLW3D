
#if !defined(__cplusplus) && !defined(SWIG)
#error C++ compiler is required for the nurbs_tools!
#endif

#ifndef _DOMINONURBS_PY_TOOLS_H
#define _DOMINONURBS_PY_TOOLS_H

#include "domino_nurbs/domino_nurbs.h"

#ifdef  __cplusplus
extern "C" {
#endif

    _NurbsVector4* NurbsVector4_free( _NurbsVector4* stream );
    _NurbsVector3* NurbsVector3_free( _NurbsVector3* stream );

    _NurbsVector3* NurbsVector3_getItem( _NurbsVector3* stream, const size_t index );
    _NurbsVector4* NurbsVector4_getItem( _NurbsVector4* stream, const size_t index );
    NurbsCurve* NurbsCurve_getItem( NurbsCurve* stream, const size_t index );
    NurbsSurface* NurbsSurface_getItem( NurbsSurface* stream, const size_t index );
    NurbsControlBox* NurbsControlBox_getItem( NurbsControlBox* stream, const size_t index );

    _NurbsVector4* nurbs_surface_getControlPoint
        ( NurbsSurface* nurbs, const size_t iu, const size_t iv );
    _NurbsVector3* nurbs_control_box_getControlPoint
        ( NurbsControlBox* cb, const size_t iu, const size_t iv, const size_t iw );

#ifdef  __cplusplus
}
#endif

#endif /* _DOMINONURBS_PY_TOOLS_H */

