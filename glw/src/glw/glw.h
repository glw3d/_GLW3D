/***
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#ifndef H_GW_INTERFACE_H
#define H_GW_INTERFACE_H

#include "glw/defines.h"
#include "gwcamera.h"
#include "gwcolors.h"
#include "shapes.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Initializes GLEW (a wrap of OpenGL). 
 * This method must be called before creating the context */
int gw_initialize_glew( const int wid );

/* Returns a new window context. This then has to be linked to an OpenGL render
* context that requires an external library, such as GLFW, QT, or Wx */
int gw_requestWindowContext();

/* Returns the window context from its id */
gwWindowContext* gw_getWindowContext( const int wid );

/* Gets the frame under the mouse coordinates */
gwFrame* gw_frame_from_mouse_coord( const int wid, const int mx, const int my );

/* Adjust the framerate between rederings */
void gw_adjust_framerate( const int frame_rate );

/* Deletes the window context and releases resources. */
void gwWindowContext_dispose( gwWindowContext* context );

/* Gets the OpenGL version. If the context is not yet created, returns 0 */
int gwWindowContext_glver( const int wid );

/* Regenerates the buffers. 
 * This is required to be called when the window changes its size */
void gwWindowContext_regenerate_framebuffers( gwWindowContext* context );

/* Clears the buffers and background. */
void gwWindowContext_clear( gwWindowContext* context );

/* Reads the picking buffer at one coordinates */
gwMesh* gwWindowContext_picking
( const int wid
, int* p_vertex_id, float* p_zdepth
, const int px, const int py );

/* Reads the picking depth buffer inside a box */
gwFloat* gwWindowContext_picking_box_depth
( const int wid, int x0, int y0, int x1, int y1, size_t* buffer_len );

/* Attach a camera to a window, so it can be controlled by mouse inputs. */
void gwWindowContext_attachCamera( const int wid, gwCamera* camera_handler );

/* Draws the scene using the render list */
void gwWindowContext_draw( const int context_id );

/* Blits the current context into the auxiliary buffer */
void gwWindowContext_blitFramebuffer( gwRenderTarget* target );

/* Get the list of cameras linked to the window */
void* gw_window_getCameraList( const int wid );

/* Load a shader */
int gwShader_load
( const int wid         /* OpenGL context */
, const char* shaders_path  /* Path of the shader. */
, const char* vertex_filename   /* (Mandatory) vertex shader */
, const char* pixel_filename    /* (Mandatory) pixel/fragment shader */
, const char* geometry_filename /* Only available form OpenGL 3 */
, const int polygon     /* The shader is especialized in solids, lines or dots */
);

/* Assigns a function in OpenGL 1x as a shader */
int gwShader_loadGL1x( const int wid,   /* OpenGL context */
    void( *func )               /* Function pointer */
    ( const gwMesh* mesh
    , const gwShader* shader
    , const gwMatrix4f* model_matrix
    , const gwMatrix4f* camera_matrix
    )
);

/* Adds a pass for multipass techniques */
gwShader* gwShader_addpass
    ( const int wid                 /* OpenGL context */
    , const gwShader* gwshader      /* parent shader */
    , const char* shader_path       /* Path of the shader. */
    , const char* pixel_program     /* pixel/fragment shader */
    );  

/* Returns the shader from its mask */
gwShader* gw_getShader( const int wid, const int shader_mask );

/* Loads the default shaders */
void gw_load_default_shaders( const int wid, const char* working_path );

/* Creates a frame */
gwFrame* gwFrame_create( const int wid );

/* Indicates that the rendering is in the area defined by the frame */
void gwFrame_use( const gwFrame* frame );

/* Adds a camera to the render list */
extern "C"
void gwFrame_addCamera( gwFrame* frame_handler, gwCamera* camera_handler );

/* Creates a camera. At least one camera is required to render. */
gwCamera* gwCamera_create();

/* Calculates the transformation view matrix */
gwMatrix4f gwCamera_calculate_view_matrix( const gwCamera* camera );

/* Add a mesh to the render list */
void gwCamera_addMesh( gwCamera* camera_handler, gwMesh* mesh_handler );

/* Draw the scene linked to this camera */
void gwCamera_render( gwCamera* camera_handler );

/* Creates an empty mesh */
gwMesh* gwMesh_create();

/* Calculates the normals */
void gwMesh_calculateNormals( gwMesh* gwmesh );

/* Sets the texture coordinates to a single slab, when different textures
 * come from the same image in a form os mosaic */
void gwMesh_textureSlab( gwMesh* mesh
    , const float slab_column, const float slab_row
    , const float mosaic_width, const float mosaic_height );

/* Draws a mesh using a camera and a shader */
void gwMesh_draw
( const gwMesh* mesh_handler
, const gwShader* shader_handler
, const gwMatrix4f model_matrix
, const gwMatrix4f view_matrix
);

/* Calculates the model matrix, based on position, rotation, size, etc... */
gwMatrix4f gwMesh_calculate_local_matrix( const gwMesh* mesh_handler );

/* Creates a vertex stream */
gwVertexStream* gwVertexStream_create( const int len );

/* Allocates memory in the vertex stream */
void gwVertexStream_alloc( gwVertexStream* vs, const int len );

/* Assign the value of a single vertex */
void gwVertexStream_set
( gwVertexStream* vertex_stream
, const size_t i
, const gwFloat pos_x, const gwFloat pos_y, const gwFloat pos_z
, const gwFloat normal_x, const gwFloat normal_y, const gwFloat normal_z
, const gwFloat texcoord_u, const gwFloat texcoord_v
);

/* Creates an index stream */
gwIndexStream* gwIndexStream_create( const int len, const int primitive );

/* Allocates memory in the index stream */
void gwIndexStream_alloc( gwIndexStream* index, const int len );

/* Creates a scalar stream.Pprovides a single value float value. 
 * If the stream is nullpotr it will allocate memory;
 * use the first parameter to use a external stream for the data */
gwScalarStream* gwScalarStream_create( gwFloat* stream, const int len );

/* Creates an empty texture with the especified dimensions */
gwTexture* gwTexture_create( const int width, const int height );

/* Imports a texture from a .bmp file */
gwTexture* gwTexture_loadBMP( const char* filename );

/* Imports a texture from a .tga file */
gwTexture* gwTexture_loadTGA( const char* filename );

/* Imports a texture from a file. Takes the format from the file extension */
gwTexture* gwTexture_load( const char* filename );

/* Saves the image in a .bmp file */
int gwTexture_saveBMP( const gwTexture* texture, const char* filename );

/* Set the alpha channel of one color to 0. Tolerance is indicated as 0-255 */
void gwTexture_transparencyMask( const gwTexture* tex
    , const gwColor color, const int tolerance );

/* Deletes a texture */
void gwTexture_delete( gwTexture* tex );

/* The render target is an offscreen texture.
* For performance it is recommended that the dimensions are power of 2;
* e.g. 256x256. Assign width and height 0 to have the same size as the window */
gwRenderTarget* gwRenderTarget_create( const int wid, const int width, const int height );

/* Indicates that the rendering is to an off-screen buffer */
void gwRenderTarget_use( const gwRenderTarget* frame );

/* Adds a frame to the render list */
void gwRenderTarget_addFrame( gwRenderTarget* frame_handler, gwFrame* camera_handler );

/* Calculates a Cubic Hermite Spline interpolation */
gwVector3f gwspline
( const gwVector4f* cps /* Array of control points */
, const int ncp         /* Number of control points */
, const float s         /* Indp. parameter */
);

/* Creates a texture with the text */
gwTexture* gwText( const int ftid, const char* text, const int size, gwColor color );

/* Creates a label to visualize a text */
gwMesh* gwLabel( const int ftid, const char* text, const int size );

/* Modifies an existing label */
gwMesh* gwLabel_update( gwMesh* mesh, const int ftid, const char* text, const int size );

/******************************************************************************/

#if defined(__cplusplus)
}
#endif

#endif /* H_GW_INTERFACE_H */
