#include <stdlib.h>

#include "domino_nurbs/domino_nurbs_data.h"

extern "C"
_NurbsVector4* NurbsVector4_free( _NurbsVector4* stream )
{
    free( stream );
    return nullptr;
}

extern "C"
_NurbsVector3* NurbsVector3_free( _NurbsVector3* stream )
{
    free( stream );
    return nullptr;
}

extern "C"
_NurbsVector3* NurbsVector3_getItem( _NurbsVector3* stream, const size_t index )
{
    return &(stream[index]);
}

extern "C"
_NurbsVector4* NurbsVector4_getItem( _NurbsVector4* stream, const size_t index )
{
    return &(stream[index]);
}

extern "C"
NurbsCurve* NurbsCurve_getItem( NurbsCurve* stream, const size_t index )
{
    return &(stream[index]);
}

extern "C"
NurbsSurface* NurbsSurface_getItem( NurbsSurface* stream, const size_t index )
{
    return &(stream[index]);
}

extern "C"
NurbsControlBox* NurbsControlBox_getItem( NurbsControlBox* stream, const size_t index )
{
    return &(stream[index]);
}

extern "C"
_NurbsVector4* nurbs_surface_getControlPoint
( NurbsSurface* nurbs, const size_t iu, const size_t iv )
{
    return &(nurbs->cp[iu][iv]);
}

extern "C"
_NurbsVector3* nurbs_control_box_getControlPoint
( NurbsControlBox* cb, const size_t iu, const size_t iv, const size_t iw )
{
    return &(cb->cp[iu][iv][iw]);
}


