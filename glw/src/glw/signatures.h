/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Signatures are used in some classes to check the upcasting.
For example to upcasting; e.g. from gwMesh to Mesh, the signature indicates if 
the object has been created as Mesh.
*******************************************************************************/

#ifndef _GWH_SIGNATURE_H
#define _GWH_SIGNATURE_H

#define _GW_SIGNATURE_CONTEXT       0xabcdef01
#define _GW_SIGNATURE_VERTEX        0xabcdef02
#define _GW_SIGNATURE_INDEX         0xabcdef03
#define _GW_SIGNATURE_MESH          0xabcdef04
#define _GW_SIGNATURE_CAMERA        0xabcdef05
#define _GW_SIGNATURE_FRAME         0xabcdef06
#define _GW_SIGNATURE_TEXTURE       0xabcdef07
#define _GW_SIGNATURE_RENDER_TARGET 0xabcdef08  // Buffer for render to texture
#define _GW_SIGNATURE_RENDER_BUFFER 0xabcdef09
#define _GW_SIGNATURE_SHADER        0xabcdef10


#endif /* _GWH_SIGNATURE_H */
