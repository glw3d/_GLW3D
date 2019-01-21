/***
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/
#include <time.h>

#include "common/string_ext.h"
#include "dataset/img/import_bmp.h"
#include "dataset/img/import_tga.h"
#include "font/font.h"

#include "gwdata.h"
#include "gwthreads.h"
#include "gwcamera.h"
#include "glw.h"

#include "ShaderGeneric.hpp"
#include "VertexStream.hpp"
#include "IndexStream.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Frame.hpp"
#include "RenderTarget.hpp"
#include "WindowContext.hpp"

using namespace gw;

namespace gw
{
    static WindowContext* global_list_context[_GW_MAX_WINDOWS] = { nullptr };
    static int global_default_font_regular = 0;
    static int global_default_font_bold = 0;
    static int global_default_font_italic = 0;
    static Camera* global_list_camera[_GW_MAX_CAMERAS] = { nullptr };
    static Mesh* global_list_mesh[_GW_MAX_MESHES] = { nullptr };
    static int global_num_meshes = 1;
    static WindowContext* global_current_context = nullptr;
    static gwViewPort global_current_viewport = { 0 };
    static gwViewPort global_current_framebuffer = { 0 };
};

static inline int get_shader_index( const int shader_mask )
{
    int s = shader_mask;
    if (s == 0) return 0;
    int nbits = sizeof( int ) * 8;
    for (int i = 0; i < nbits; i++){
        if ((s & 1) == 1){
            return i;
        }
        s = (s >> 1);
    }

    return 0;
}

static int check_object( const WindowContext* obj )
{
    if (obj == nullptr){
        return 1;
    }
    if (obj->signature != _GW_SIGNATURE_CONTEXT){
        _handle_error_( "Bad signature! Object is not correctly generated" );
        return 1;
    }
    return 0;
}

static int check_object( const Frame* obj )
{
    if (obj == nullptr){
        return 1;
    }
    if (obj->signature != _GW_SIGNATURE_FRAME){
        _handle_error_( "Bad signature! Object is not correctly generated" );
        return 1;
    }
    return 0;
}

static int check_object( const Mesh* obj )
{
    if (obj == nullptr){
        return 1;
    }
    if (obj->signature != _GW_SIGNATURE_MESH){
        _handle_error_( "Bad signature! Object is not correctly generated" );
        return 1;
    }
    return 0;
}

static int check_object( const VertexStream* obj )
{
    if (obj == nullptr){
        return 1;
    }
    if (obj->signature != _GW_SIGNATURE_VERTEX){
        _handle_error_( "Bad signature! Object is not correctly generated" );
        return 1;
    }
    return 0;
}

static int check_object( const IndexStream* obj )
{
    if (obj == nullptr){
        return 1;
    }
    if (obj->signature != _GW_SIGNATURE_INDEX){
        _handle_error_( "Bad signature! Object is not correctly generated" );
        return 1;
    }
    return 0;
}

static int check_object( const ShaderGeneric* obj )
{
    if (obj == nullptr){
        return 1;
    }
    if (obj->signature != _GW_SIGNATURE_SHADER){
        _handle_error_( "Bad signature! Object is not correctly generated" );
        return 1;
    }
    return 0;
}

static int check_object( const Texture* obj )
{
    if (obj == nullptr){
        return 1;
    }
    if (obj->signature != _GW_SIGNATURE_TEXTURE){
        _handle_error_( "Bad signature! Object is not correctly generated" );
        return 1;
    }
    return 0;
}

static int check_object( const Camera* obj )
{
    if (obj == nullptr){
        return 1;
    }
    if (obj->signature != _GW_SIGNATURE_CAMERA){
        _handle_error_( "Bad signature! Object is not correctly generated" );
        return 1;
    }
    return 0;
}

static int check_object( const RenderTarget* obj )
{
    if (obj == nullptr){
        return 1;
    }
    if (obj->signature != _GW_SIGNATURE_RENDER_TARGET){
        _handle_error_( "Bad signature! Object is not correctly generated" );
        return 1;
    }
    return 0;
}

/******************************************************************************/

/* Initializes GLEW */
extern "C"
int gw_initialize_glew( const int wid )
{
    if (sizeof( GLfloat ) != sizeof( gwFloat )){
        _handle_error_( "GLfloat and gwFloat sizes do not match!" );
        return 0;
    }
    if (sizeof( GLint ) != sizeof( gwInt )){
        _handle_error_( "GLint and gwInt sizes do not match!" );
        return 0;
    }

    _check_gl_error_
        glewExperimental = false;
    GLenum err = glewInit();
    if (err != GLEW_OK){
        _handle_error_( "Unable to initialize GLEW: %s"
            , glewGetErrorString( err ) );
        return 0;
    }
    _check_gl_error_

    const unsigned char* gl_version = glGetString( GL_VERSION );
    const unsigned char* gl_shading_language
        = glGetString( GL_SHADING_LANGUAGE_VERSION );
    const unsigned char* gl_renderer = glGetString( GL_RENDERER );
    int mayor_version = 0, minor_version = 0;
    if (gl_version != nullptr){
        sscanf( (char*)gl_version, "%i.%i", &mayor_version, &minor_version );
        _log_( "OpenGL version: %s\n", gl_version );
        _log_( "Shading language: %s\n", gl_shading_language );
        _log_( "Renderer: %s\n", gl_renderer );
    }
    else{
        _handle_error_( "No OpenGL context available!" );
    }
    int glver = 10 * mayor_version + minor_version;

    WindowContext* context = global_list_context[wid];
    if (context != nullptr){
        context->gl_version = glver;
    }
    return glver;
}

extern "C"
int gw_requestWindowContext()
{
    /* Find an available slot. The 0 is reserved. */
    for (int i = 1; i < _GW_MAX_WINDOWS; i++){
        if (global_list_context[i] == nullptr){
            global_list_context[i] = new WindowContext( i );

            return i;
        }
    }

    _handle_error_( "Too much windows!" );
    return 0;
}

extern "C"
gwWindowContext* gw_getWindowContext( const int wid )
{
    if (wid >= 0 && wid < _GW_MAX_WINDOWS){
        return global_list_context[wid];
    }

    return nullptr;
}

extern "C"
void* gw_window_getCameraList( const int wid )
{
    WindowContext* context = global_list_context[wid];
    DynamicStack< Camera >* token = &context->list_cameras;
    return token;
}

extern "C" 
gwFrame* gw_frame_from_mouse_coord( const int wid, const int mx, const int my )
{
    Frame* highlited_frame = nullptr;
    WindowContext* context = global_list_context[wid];
    DynamicStack< Frame >* token = &context->list_frames;
    while (token != nullptr){
        Frame* frame = token->obj;
        token = token->next;

        int x0, y0, x1, y1;
        if (frame != nullptr){
            if (frame->size_in_pixels1_relative0 == 0){
                x0 = (int)(frame->x0 * context->screen_width);
                x1 = (int)(frame->x1 * context->screen_width);
                y0 = (int)((1 - frame->y0) * context->screen_height);
                y1 = (int)((1 - frame->y1) * context->screen_height);
            }
            else{
                x0 = (int)(frame->x0);
                x1 = (int)(frame->x1);
                y0 = (int)(context->screen_height - frame->y0);
                y1 = (int)(context->screen_height - frame->y1);
            }
        }
        else{
            x0 = 0;
            x1 = 0;
            y0 = context->screen_height;
            y1 = context->screen_height;
        }

        if (x0 > x1){
            int temp = x0;
            x0 = x1;
            x1 = temp;
        }
        if (y0 > y1){
            int temp = y0;
            y0 = y1;
            y1 = temp;
        }

        if (mx >= x0 && my <= x1 && my >= y0 && my <= y1){
            highlited_frame = frame;
        }
    }

    return highlited_frame;
}

extern "C"
void gw_adjust_framerate( const int frame_rate )
{
    static clock_t start_time = 0;
    clock_t end_time = clock();

    /* Calculates the minimun miliseconds to adjust the framerate */
    const int miliseconds = 1000 / frame_rate;
    if (start_time == 0){
        gw_sleep( miliseconds );
        start_time = clock();
        return;
    }

    int time_rendering = int( ((end_time - start_time) * 1000) / CLOCKS_PER_SEC );
    if (time_rendering < miliseconds){
        gw_sleep( miliseconds - time_rendering );
    }
    start_time = clock();
}

/*****/

/* Loads a shader */
void load_shader
( ShaderGeneric* shader
, const char* shaders_path      /* Path of the shader. Usually is in "shader" */
, const char* vertex_filename   /* (Mandatory) vertex shader */
, const char* pixel_filename    /* (Mandatory) pixel/fragment shader */
, const char* geometry_filename /* Only available from OpenGL 3 */
, const int gl_version          /* Indicates if the context is available */
)
{
    /* Copy the files and the path */
    if (pixel_filename != nullptr){
        strcpy( shader->label, pixel_filename );
    }
    if (shaders_path != nullptr){
        strcpy( shader->path, shaders_path );
    }
    if (vertex_filename != nullptr){
        strcpy( shader->vertex_filename, vertex_filename );
    }
    if (pixel_filename != nullptr){
        strcpy( shader->fragment_filename, pixel_filename );
    }
    if (geometry_filename != nullptr){
        strcpy( shader->geometric_filename, geometry_filename );
    }

    if (gl_version > 0){
        int status = shader->load
            ( shaders_path
            , shader->vertex_filename
            , shader->fragment_filename
            , nullptr
            );
        shader->update = 0;

        if (status != gw::SHADER_OK){
            _handle_error_( "Failed to load shader: %s %s %s"
                , (shader->vertex_filename == nullptr) ? "" : shader->vertex_filename
                , (shader->fragment_filename == nullptr) ? "" : shader->fragment_filename
                );

            shader->shader.unload();
        }
    }
    else{
        /* The context is not created and the shader cannot be loaded.
        * Set update to true, so it will be loaded the next render cycle */
        shader->update = 1;
    }
}

ShaderGeneric* context_load_shader
( WindowContext* context
, const char* path
, const char* vert
, const char* frag
, const char* geo
, const int mask
)
{
    int slot = get_shader_index( mask );
    ShaderGeneric* shader = context->shader_pipeline[slot];
    if (shader == nullptr){
        shader = new ShaderGeneric;
    }
    load_shader( shader, path, vert, frag, geo, context->gl_version );
    context->shader_pipeline[slot] = shader;
    shader->mask = mask;

    /* Load multipass shaders */
    ShaderGeneric* next = (ShaderGeneric*)shader->multipass;
    while (next != nullptr){
        load_shader( next, next->path
            , next->vertex_filename
            , next->fragment_filename
            , next->geometric_filename
            , context->gl_version );

        next = (ShaderGeneric*)next->multipass;
    }
    return shader;
}

static
void load_default_shaders( WindowContext* context )
{
    /* Load the default shaders */
    char shaders_path[1024] = { '\0' };
    size_t len = strlen( context->default_shaders_path );
    if (len >= 1024){
        len = 1023;
    }
    if (len > 0){
        memcpy( shaders_path, context->default_shaders_path
            , sizeof( char ) * (len + 1) );
    }
    if (shaders_path[strlen( shaders_path ) - 1] != '/'
        && shaders_path[strlen( shaders_path ) - 1] != '\\')
    {
        strcat( shaders_path, "/" );
    }

    ShaderGeneric* shader = nullptr;
    if (context->gl_version > 30){
        /* Default shader for surfaces */
        shader = context_load_shader
            ( context, shaders_path
            , "lambertian_lighting.vsh"
            , "lambertian_lighting.fsh"
            , nullptr
            , GW_SHADER_SOLID
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 1;
            shader->polygon = GW_POLYGON_FILL;
        }

        /* Default shader for lines */
        shader = context_load_shader
            ( context, shaders_path
            , "wireframe.vsh"
            , "wireframe.fsh"
            , nullptr
            , GW_SHADER_WIREFRAME
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 1;
            shader->polygon = GW_POLYGON_LINE;
        }

        /* Default shader for points */
        shader = context_load_shader
            ( context, shaders_path
            , "dotcloud.vsh"
            , "dotcloud.fsh"
            , nullptr
            , GW_SHADER_DOTCLOUD
            );
        if (shader != nullptr){
            shader->polygon = GW_POLYGON_POINT;
            shader->alpha_test = 0;
            shader->depth_test = 1;
        }

        /* Default shader for sprites and blitting */
        shader = context_load_shader
            ( context, shaders_path
            , "blit.vsh"
            , "blit.fsh"
            , nullptr
            , GW_SHADER_BLIT
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 1;
            shader->polygon = GW_POLYGON_FILL;
        }

        /* Default shader for drawing objects with textures */
        shader = context_load_shader
            ( context, shaders_path
            , "texture.vsh"
            , "texture.fsh"
            , nullptr
            , GW_SHADER_TEXTURE
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 1;
            shader->polygon = GW_POLYGON_FILL;
        }

        /* Default shader for drawing text */
        shader = context_load_shader
            ( context, shaders_path
            , "font.vsh"
            , "font.fsh"
            , nullptr
            , GW_SHADER_FONT
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 0;
            shader->polygon = GW_POLYGON_FILL;
        }

        /* Default shader for picking */
        shader = context_load_shader
            ( context, shaders_path
            , "picking.vsh"
            , "picking.fsh"
            , nullptr
            , GW_SHADER_PICKING
            );
        if (shader != nullptr){
            shader->alpha_test = 0;
            shader->depth_test = 1;
            shader->target = GW_TARGET_PICKING;
            shader->polygon = GW_POLYGON_NONE;
        }

        /* Default shader for scalars */
        shader = context_load_shader
            ( context, shaders_path
            , "scalar.vsh"
            , "scalar0.fsh"
            , nullptr
            , GW_SHADER_ISOLINES
            );
        if (shader != nullptr){
            shader->alpha_test = 0;
            shader->depth_test = 0;
            shader->polygon = GW_POLYGON_FILL;
            gwShader_addpass( context->id, shader, shaders_path, "scalar1.fsh" );
        }
    }
    else if (context->gl_version > 20){
        /* Default shader for surfaces */
        shader = context_load_shader
            ( context, shaders_path
            , "lambertian_lighting_21.vsh"
            , "lambertian_lighting_21.fsh"
            , nullptr
            , GW_SHADER_SOLID
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 1;
            shader->polygon = GW_POLYGON_FILL;
        }

        /* Default shader for lines */
        shader = context_load_shader
            ( context, shaders_path
            , "wireframe_21.vsh"
            , "wireframe_21.fsh"
            , nullptr
            , GW_SHADER_WIREFRAME
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 1;
            shader->polygon = GW_POLYGON_LINE;
        }

        /* Default shader for points */
        shader = context_load_shader
            ( context, shaders_path
            , "dotcloud_21.vsh"
            , "dotcloud_21.fsh"
            , nullptr
            , GW_SHADER_DOTCLOUD
            );
        if (shader != nullptr){
            shader->alpha_test = 0;
            shader->depth_test = 1;
            shader->polygon = GW_POLYGON_POINT;
        }

        /* Default shader for blitting */
        shader = context_load_shader
            ( context, shaders_path
            , "blit_21.vsh"
            , "blit_21.fsh"
            , nullptr
            , GW_SHADER_BLIT
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 1;
            shader->polygon = GW_POLYGON_FILL;
        }

        /* Default shader for drawing objects with textures */
        shader = context_load_shader
            ( context, shaders_path
            , "texture_21.vsh"
            , "texture_21.fsh"
            , nullptr
            , GW_SHADER_TEXTURE
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 1;
            shader->polygon = GW_POLYGON_FILL;
        }

        /* Default shader for drawing text */
        shader = context_load_shader
            ( context, shaders_path
            , "font_21.vsh"
            , "font_21.fsh"
            , nullptr
            , GW_SHADER_FONT
            );
        if (shader != nullptr){
            shader->alpha_test = 1;
            shader->depth_test = 0;
            shader->polygon = GW_POLYGON_FILL;
        }

        /* Default shader for picking */
        shader = context_load_shader
            ( context, shaders_path
            , "picking_21.vsh"
            , "picking_21.fsh"
            , nullptr
            , GW_SHADER_PICKING
            );
        if (shader != nullptr){
            shader->alpha_test = 0;
            shader->depth_test = 1;
            shader->target = GW_TARGET_PICKING;
            shader->polygon = GW_POLYGON_NONE;
        }
    }
    context->request_load_default_shaders = 0;
}

/*****/

extern "C"
void gwWindowContext_dispose( gwWindowContext* context )
{
    if (context != nullptr){

        // CLEAN UP ALL RENDER OBJECTS!!!
    }
}

int gwWindowContext_glver( const int wid )
{
    if (wid > 0 && wid < _GW_MAX_WINDOWS){
        WindowContext* context = global_list_context[wid];
        if (context != nullptr){
            return context->gl_version;
        }
    }

    return 0;
}

/* Forces to update the buffer when the window has changed its size */
void gwWindowContext_regenerate_framebuffers( gwWindowContext* gwcontext )
{
    if (gwcontext != nullptr){
        WindowContext* context = (WindowContext*)gwcontext;
        context->regenerate_framebuffers();
    }
}

/* Clears the buffers, background and load default shaders */
void gwWindowContext_clear( gwWindowContext* gwcontext )
{
    WindowContext* context = (WindowContext*)gwcontext;

    /* Set the current context this one */
    global_current_context = context;

    if (check_object( context )) return;

    context->clear();
    global_current_viewport.x0 = 0;
    global_current_viewport.y0 = 0;
    global_current_viewport.width = context->screen_width;
    global_current_viewport.height = context->screen_height;
    global_current_viewport.fbo = 0; /* default frame buffer */

    global_current_framebuffer = global_current_viewport;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    /* Load the default shaders if requested */
    if (context->request_load_default_shaders != 0){
        load_default_shaders( context );
    }
}

/* Reads the picking depth buffer inside a box */
extern "C"
gwMesh* gwWindowContext_picking
( const int wid
, int* p_vertex_id, float* p_zdepth
, const int px, const int py )
{
    int mesh_id = 0;
    *p_vertex_id = -1;
    *p_zdepth = 0;

    WindowContext* context = nullptr;
    if (wid >= 0 && wid < _GW_MAX_WINDOWS){
        context = global_list_context[wid];
    }
    if (context != nullptr){
        context->get_picking( &mesh_id, p_vertex_id, p_zdepth, px, py );
    }

    Mesh* picked_mesh = nullptr;
    if (mesh_id > 0 && mesh_id < _GW_MAX_MESHES){
        /* Picked mesh */
        return global_list_mesh[mesh_id];
    }
    else{
        return nullptr;
    }
}

/* Reads the picking buffer inside a square box */
extern "C"
GLfloat* gwWindowContext_picking_box_depth
( const int wid, int x0, int y0, int x1, int y1, size_t* buffer_len )
{
    GLfloat* zdepth_buffer = nullptr;
    if (wid >= 0 && wid < _GW_MAX_WINDOWS){
        WindowContext* context = global_list_context[wid];
        if (context != nullptr){
            zdepth_buffer = context->get_picking_box( x0, y0, x1, y1, buffer_len );
        }
    }

    return zdepth_buffer;
}


/* Attachs a camera to a window, so it can be controlled by mouse inputs */
extern "C"
void gwWindowContext_attachCamera( const int wid, gwCamera* camera_handler )
{
    WindowContext* context = global_list_context[wid];
    Camera* camera = (Camera*)camera_handler;

    if (check_object( context )) return;
    if (check_object( camera )) return;

    context->list_cameras.add( camera );
}

extern "C"
void gwWindowContext_draw( const int wid )
{
    WindowContext* context = global_list_context[wid];

    if (context != nullptr){
        /* Arrange the frames acording to its preference number */
        context->arrange_frames();

        /* Load the default shaders if requested */
        if (context->request_load_default_shaders != 0){
            load_default_shaders( context );
        }

        if (context->f_on_render != nullptr){
            context->f_on_render( context );
        }

        /* Render the scene */
        context->render();
    }
}


extern "C"
void gwWindowContext_blitFramebuffer( gwRenderTarget* target )
{
    if (global_current_context == nullptr || target == nullptr) return;

    RenderTarget* render_target = (RenderTarget*)target;
    if (check_object( render_target )) return;


    int x0 = global_current_viewport.x0;
    int y0 = global_current_viewport.y0;
    int width = global_current_viewport.width;
    int height = global_current_viewport.height;
    int current_fbo = global_current_viewport.fbo;

    int buf_width = render_target->render_buffer.buffer_width;
    int buf_height = render_target->render_buffer.buffer_height;

    /* Copy everything, including the depth buffer, 
     * although it will not be accesible in the shader */ 
    glBindFramebuffer( GL_READ_FRAMEBUFFER, current_fbo );
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, render_target->render_buffer.fbo );

    glBlitFramebuffer
        ( x0, y0, x0 + width, y0 + height
        , 0, 0, buf_width, buf_height
        , GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
        );

    _check_gl_error_
}

/*****/

/* Loads a shader */
extern "C"
int gwShader_load
( const int wid         /* Context the shader is linked with */
, const char* shaders_path      /* Path of the shader. Usually is in "shader" */
, const char* vertex_filename   /* (Mandatory) vertex shader */
, const char* pixel_filename    /* (Mandatory) pixel/fragment shader */
, const char* geometry_filename /* Only available form OpenGL 3 */
, const int geometry    /* Geometry, the shader is especialized */
)
{
    WindowContext* context = nullptr;
    if (wid > 0 && wid < _GW_MAX_WINDOWS){
        context = global_list_context[wid];
    }
    if (context == nullptr){
        _handle_error_( "No active context!" );
        return 0;
    }
    
    /* Assign the shader in a slot */
    int slot = context->requestShaderSlot();
    if (slot < 0){
        _handle_error_( "Too much shaders!" );
        return 0;
    }
    ShaderGeneric* shader = new ShaderGeneric;
    shader->mask = (1 << slot);
    context->shader_pipeline[slot] = shader;
    load_shader( shader, shaders_path
        , vertex_filename, pixel_filename, geometry_filename
        , context->gl_version );
    shader->polygon = geometry;

    return shader->mask;
}

/* Assign a function in OpenGL 1.x as shader */
extern "C"
int gwShader_loadGL1x( const int wid, void( *func )
(const gwMesh* mesh, const gwShader* shader
, const gwMatrix4f* model_matrix, const gwMatrix4f* camera_matrix) )
{
    WindowContext* context = nullptr;
    if (wid > 0 && wid < _GW_MAX_WINDOWS){
        context = global_list_context[wid];
    }
    if (context == nullptr){
        _handle_error_( "Not an active context!" );
        return 0;
    }

    /* Assign the shader in a slot */
    int slot = context->requestShaderSlot();
    if (slot < 0){
        _handle_error_( "Too much shaders!" );
        return 0;
    }
    ShaderGeneric* shader = new ShaderGeneric;
    shader->mask = (1 << slot);
    context->shader_pipeline[slot] = shader;
    shader->f_gl1x = func;

    return (1 << slot);
}

/* Gets the shader from its mask */
extern "C" 
gwShader* gw_getShader( const int wid, const int shader_mask )
{
    WindowContext* context = global_list_context[wid];
    int shader_id = get_shader_index( shader_mask );
    if (shader_id >= 0 && shader_id < _GW_SHADERS_PIPELINE_LENGTH){
        return context->shader_pipeline[shader_id];
    }
    return nullptr;
}

/* Adds a multipass */
gwShader* gwShader_addpass
( const int wid, const gwShader* gwshader, const char* shader_path, const char* pixel_program )
{
    WindowContext* context = nullptr;
    if (wid > 0 && wid < _GW_MAX_WINDOWS){
        context = global_list_context[wid];
    }
    if (context == nullptr){
        _handle_error_( "No active context!" );
        return 0;
    }

    ShaderGeneric* shader = (ShaderGeneric*)gwshader;
    if (check_object( shader )) return nullptr;

    ShaderGeneric* next_pass = new ShaderGeneric();
    shader->multipass = next_pass;

    load_shader( next_pass
        , shader_path, "postprocessing.vsh", pixel_program
        , nullptr, context->gl_version );

    next_pass->polygon = GW_POLYGON_FILL;
    next_pass->depth_test = 0;

    return next_pass;
}

/* Loads all default shaders. This is called when the window is created */
extern "C"
void gw_load_default_shaders( const int wid, const char* working_path )
{
    WindowContext* context = nullptr;

    if (wid > 0 && wid < _GW_MAX_WINDOWS){
        context = global_list_context[wid];
    }
    if (context == nullptr){
        _handle_error_( "%i is not an active context!", wid );
        return;
    }

    if (working_path != nullptr){
        memcpy( context->default_shaders_path, working_path, strlen( working_path ) + 1 );
    }
    if (context->gl_version == 0){
        context->request_load_default_shaders = 1;
    }
    else{
        load_default_shaders( context );
    }
}

/*****/

extern "C"
gwFrame* gwFrame_create( const int wid )
{
    WindowContext* context = nullptr;
    if (wid > 0 && wid < _GW_MAX_WINDOWS){
        context = global_list_context[wid];
    }
    if (context == nullptr){
        _handle_error_( "No an active context!" );
        return 0;
    }

    /* Find an available slot */
    Frame* frame = new Frame();
    context->list_frames.add( frame );

    return frame;
}

/* Indicates that the rendering is in the area defined by the frame */
void gwFrame_use( const gwFrame* frame_handler )
{
    Frame* frame = (Frame*)frame_handler;
    if (check_object( frame )) return;

    global_current_viewport = frame->clear( global_current_framebuffer );
}

/* Adds a camera to the render list */
extern "C"
void gwFrame_addCamera( gwFrame* frame_handler, gwCamera* camera_handler )
{
    Frame* frame = (Frame*)frame_handler;
    Camera* camera = (Camera*)camera_handler;
    if (check_object( frame )) return;
    if (check_object( camera )) return;

    frame->render_list.add( camera );
}

/*****/

/* The render target is an offscreen texture.
* For performance dimensions are recommended to be power of 2; e.g. 256x256 
* Assign dimensions 0, 0 to make the buffer the same dimensions as the screen */
gwRenderTarget* gwRenderTarget_create( const int wid, const int width, const int height )
{
    WindowContext* context = nullptr;

    if (wid > 0 && wid < _GW_MAX_WINDOWS){
        context = global_list_context[wid];
    }
    if (context == nullptr){
        _handle_error_( "%i is not an active context!", wid );
        return nullptr;
    }

    RenderTarget* fb = new RenderTarget;
    if (fb == nullptr){
        return nullptr;
    }

    fb->width = width;
    fb->height = height;

    context->list_buffers.add( fb );

    return fb;
}

/* Indicates that the rendering is in the area defined by the frame */
void gwRenderTarget_use( const gwRenderTarget* target )
{
    RenderTarget* render_target = (RenderTarget*)target;

    /* If the argument is nullptr, returns to the default frame buffer */
    if (render_target == nullptr && global_current_context != nullptr){
        WindowContext* context = global_current_context;
        glBindFramebuffer( GL_FRAMEBUFFER, 0 ); /* default framebuffer */
        glViewport( 0, 0, context->screen_width, context->screen_height );
        glScissor( 0, 0, context->screen_width, context->screen_height );

        global_current_viewport.x0 = 0;
        global_current_viewport.y0 = 0;
        global_current_viewport.width = context->screen_width;
        global_current_viewport.height = context->screen_height;
        global_current_viewport.fbo = 0; 

        global_current_framebuffer = global_current_viewport;

        return;
    }

    if (check_object( render_target )) return; 

    /* Check the render target dimensions */
    int screen_width = global_current_context->screen_width; 
    int screen_height = global_current_context->screen_height; 
    render_target->update( screen_width, screen_height ); 

    global_current_viewport = render_target->clear(); 
    global_current_framebuffer = global_current_viewport; 
}

/*****/

/* Creates a camera */
extern "C"
gwCamera* gwCamera_create()
{
    Camera* camera = new Camera;
    return camera;
}

/* Calculates the transformation matrix */
gwMatrix4f gwCamera_calculate_view_matrix( const gwCamera* camera )
{
    int width = global_current_viewport.width;
    int height = global_current_viewport.height;

    return gwCamera_calculate_matrix( camera, width, height );
}

/* Add a mesh to the render list */
void gwCamera_addMesh( gwCamera* camera_handler, gwMesh* mesh_handler )
{
    Camera* camera = (Camera*)camera_handler;
    Mesh* mesh = (Mesh*)mesh_handler;

    if (check_object( camera )) return;
    if (check_object( mesh )) return;

    camera->render_list.add( mesh );
}

/* Draw the scene linked to this camera */
void gwCamera_render( gwCamera* camera_handler )
{
    WindowContext* context = global_current_context;
    if (context == nullptr){
        return;
    }
    int viewport_width = global_current_viewport.width;
    int viewport_height = global_current_viewport.height;

    Camera* camera = (Camera*)camera_handler;
    if (check_object( camera )) return;
    if (camera->active == 0){
        return;
    }

    camera->view_matrix = gwCamera_calculate_matrix
        ( camera, viewport_width, viewport_height );

    camera->update_transformations( viewport_width, viewport_height );

    DynamicStack<Camera> list;
    list.add( camera );
    int num_entities = 0;
    /* Loop through the shaders pipeline */
    for (int ish = 0; ish < _GW_SHADERS_PIPELINE_LENGTH; ish++){
        ShaderGeneric* shader = context->shader_pipeline[ish];
        if (shader == nullptr) continue;

        context->render_shader( &list, shader, global_current_viewport );
    }

    list.clear();
}

void gwCamera_autofit( gwCamera* gwcamera )
{
	Camera* camera= ( Camera* )gwcamera;
	camera->autofit();
}

/*****/

/* Creates an empty mesh */
extern "C"
gwMesh* gwMesh_create()
{
    if (global_num_meshes >= _GW_MAX_MESHES){
        _handle_error_( "Too much meshes!" );
        return nullptr;
    }

    Mesh* mesh = new Mesh;
    global_list_mesh[global_num_meshes] = mesh;
    mesh->id = global_num_meshes;
    global_num_meshes++;
    return mesh;
}

extern "C"
void gwMesh_draw
( const gwMesh* mesh_handler
, const gwShader* shader_handler
, const gwMatrix4f model_matrix
, const gwMatrix4f view_matrix)
{
    Mesh* mesh = (Mesh*)mesh_handler;
    ShaderGeneric* shader = (ShaderGeneric*)shader_handler;

    if (check_object( mesh )) return;
    if (check_object( shader )) return;
    if (global_current_context == nullptr ){
        return;
    }
    int screen_width = global_current_viewport.width;
    int screen_height = global_current_viewport.height;

    /* Load or reload the shader if on request */
    if (shader->update != 0 || shader->shader.fragment_shader == 0){

        shader->load( shader->path
            , shader->vertex_filename
            , shader->fragment_filename
            , shader->geometric_filename );


        /* Load multipass shaders */
        ShaderGeneric* next = (ShaderGeneric*)shader->multipass;
        while (next != nullptr){
            next->load( next->path
                , next->vertex_filename
                , next->fragment_filename
                , next->geometric_filename );

            next = (ShaderGeneric*)next->multipass;
        }

        shader->update = 0;
    }

    _check_gl_error_

    /* Bind the correct framebuffer */
    if (shader->target == GW_TARGET_PICKING){
        /* Target the picking buffer */
        const GLenum att[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glBindFramebuffer( GL_FRAMEBUFFER, global_current_context->picking_buffer.fbo );
        glDrawBuffers( 2, att );
    }
    else{
        const GLenum att[] = { GL_COLOR_ATTACHMENT0 };
        glBindFramebuffer( GL_FRAMEBUFFER, global_current_viewport.fbo );
    }

    if (shader->shader.program == 0){
        _handle_error_( "The shader %s is not loaded", shader->fragment_filename );
        return;
    }

    /* Activate the shader */
    shader->useProgram();

    /* In multipass, activate the stencil */
    if (shader->multipass != nullptr){
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xFF );
        glDepthMask( GL_TRUE );
        glStencilFunc( GL_ALWAYS, 1, 0xFF );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
        glClearStencil( 0 );
        glClear( GL_STENCIL_BUFFER_BIT );
    }

    /* Set the uniforms */
    mesh->set_uniforms( shader, mesh, model_matrix, view_matrix
        , screen_width, screen_height );

    /* Draw the object */
    mesh->draw( shader, global_current_context->gl_version
        , &model_matrix, &view_matrix );

    if (shader->multipass != nullptr){
        Mesh* quad = (Mesh*)(global_current_context->quad);
        RenderTarget* buffer = (RenderTarget*)(&global_current_context->auxiliary_buffer);
        ShaderGeneric* next_pass = (ShaderGeneric*)shader->multipass;
        while (next_pass != nullptr && next_pass->signature == _GW_SIGNATURE_SHADER){
            /* Copy the current buffer to the auxiliary buffer */
            gwWindowContext_blitFramebuffer( &global_current_context->auxiliary_buffer );
            glBindFramebuffer( GL_FRAMEBUFFER, global_current_viewport.fbo );

            glDisable( GL_DEPTH_TEST );
            glStencilFunc( GL_EQUAL, 1, 0xFF );

            next_pass->useProgram();

            gwMatrix4f ident = gwMatrix4f_identity();
            quad->texture0 = buffer->color_texture;
            quad->set_uniforms( next_pass, quad, ident, ident
                , screen_width, screen_height );
            quad->draw( next_pass, global_current_context->gl_version
                , &ident, &ident );

            next_pass = (ShaderGeneric*)next_pass->multipass;
        }

        glDisable( GL_STENCIL_TEST );
    }
}

extern "C"
void gwMesh_calculateNormals( gwMesh* gwmesh )
{
    Mesh* mesh = (Mesh*)gwmesh;
    if (check_object( mesh )) return;

    mesh->calculateNormals();
}

extern "C"
void gwMesh_textureSlab( gwMesh* mesh
, const float slab_row, const float slab_column
, const float mosaic_height, const float mosaic_width )
{
    const gwFloat u0 = (gwFloat)slab_column / mosaic_width;
    const gwFloat v0 = (gwFloat)slab_row / mosaic_height;
    const gwFloat du = (gwFloat)1.0 / mosaic_width;
    const gwFloat dv = (gwFloat)1.0 / mosaic_height;

    for (size_t i = 0; i < mesh->vertex->length; i++){
        gwFloat tu = mesh->vertex->stream[i].texCoord.u;
        gwFloat tv = mesh->vertex->stream[i].texCoord.v;
        mesh->vertex->stream[i].texCoord.u = u0 + tu * du;
        mesh->vertex->stream[i].texCoord.v = v0 + tv * dv;
    }
}

extern "C"
gwMatrix4f gwMesh_calculate_local_matrix( const gwMesh* mesh )
{
    if (mesh == nullptr){
        return gwMatrix4f_identity();
    }

    int screen_width = global_current_viewport.width;
    int screen_height = global_current_viewport.height;

    gwMatrix4f model_matrix = gwMatrix4f_transform
        ( mesh->position.x, mesh->position.y, mesh->position.z
        , mesh->rotation.x, mesh->rotation.y, mesh->rotation.z
        , mesh->scale.x * mesh->size
        , mesh->scale.y * mesh->size
        , mesh->scale.z * mesh->size
        );

    model_matrix.data[12] += (2 * mesh->blit_coords.x) / screen_width;
    model_matrix.data[13] += (2 * mesh->blit_coords.y) / screen_width;
    model_matrix.data[14] += mesh->blit_coords.z;

    return model_matrix;
}

/*****/

/* Creates a vertex stream */
extern "C"
gwVertexStream* gwVertexStream_create( const int len )
{
    VertexStream* vs = new VertexStream;

    if (len > 0){
        vs->alloc( len );
    }
    return vs;
}

/* Allocates the memory in the vertex stream */
extern "C"
void gwVertexStream_alloc( gwVertexStream* vs, const int len )
{
    if (vs == nullptr){
        _warning_( "NULL pointer!" );
        return;
    }

    if (vs->stream != nullptr && len == vs->length){
        return;
    }

    free( vs->stream );
    vs->length = 0;
    _check_( vs->stream = (gwVertex*)_malloc_( sizeof( gwVertex )*len ) );
    if (vs->stream != nullptr){
        vs->length = len;
    }
}

/* Assign the value of a single vertex */
extern "C"
void gwVertexStream_set
( gwVertexStream* vertex_stream
, const size_t i
, const gwFloat pos_x
, const gwFloat pos_y
, const gwFloat pos_z
, const gwFloat normal_x
, const gwFloat normal_y
, const gwFloat normal_z
, const gwFloat texcoord_u
, const gwFloat texcoord_v
)
{
    if (vertex_stream != nullptr && vertex_stream->stream
        && i >= 0 && i < vertex_stream->length)
    {
        gwVertex* vertex = &(vertex_stream->stream[i]);

        vertex->position.x = pos_x;
        vertex->position.y = pos_y;
        vertex->position.z = pos_z;

        vertex->normal.x = normal_x;
        vertex->normal.y = normal_y;
        vertex->normal.z = normal_z;

        vertex->texCoord.u = texcoord_u;
        vertex->texCoord.v = texcoord_v;
    }
}

/*****/

/* Creates an index stream */
extern "C"
gwIndexStream* gwIndexStream_create( const int len, const int index_primitive )
{
    IndexStream* index = new IndexStream;

    if (len > 0){
        index->alloc( len );
    }
    index->primitive = index_primitive;

    return index;
}

/* Allocates or re-allocates the memory in the vertex stream */
extern "C"
void gwIndexStream_alloc( gwIndexStream* index, const int len )
{
    if (index == nullptr){
        _warning_( "null pointer!" );
        return;
    }
    if (index->stream != nullptr && len == index->length){
        return;
    }
    free( index->stream );
    index->length = 0;
    index->stream = (gwIndex*)malloc( sizeof( gwIndex )*len );
    if (index->stream != nullptr){
        index->length = len;
    }
}

/*****/

/* Creates a vertex stream */
extern "C"
gwScalarStream* gwScalarStream_create( gwFloat* stream, const int len )
{
    ScalarStream* vs = new ScalarStream;

    if (len > 0){
        if (stream == nullptr){
            vs->alloc( len );
        }
        else{
            vs->length = len;
            vs->stream = stream;
        }
    }

    return vs;
}

/*****/

/* Creates an empty image with the indicated size */
extern "C"
gwTexture* gwTexture_create( const int width, const int height )
{
    Texture* texture = new Texture;
    gwColor* buffer = nullptr;
    _check_( buffer = (gwColor*)_calloc_( width * height, sizeof( gwColor ) ) );
    if (buffer != nullptr){
        texture->internalFormat = GL_COMPRESSED_RGBA;
        texture->format = GL_BGRA;
        texture->bpp = 4;
        texture->image.buffer = buffer;
        texture->image.width = width;
        texture->image.height = height;
    }

    return &(texture->image);
}

/* Imports a texture from a .bmp file */
extern "C"
gwTexture* gwTexture_loadBMP( const char* filename )
{
    Texture* texture = new Texture;
    gwBmpImage bmp = gw_bmp_import( filename, true );
    if (bmp.data == nullptr || bmp.header.width <= 0 || bmp.header.height <= 0){
        /* failed to load the image */
        return &(texture->image);
    }

    if (bmp.bpp != 4){
        /* This should never not happens, but just in case... */
        _handle_error_( "Pixel format not supported!" );
        return &(texture->image);
    }

    texture->internalFormat = GL_COMPRESSED_RGBA;
    texture->format = GL_BGRA;
    texture->bpp = 4;
    texture->image.buffer = (gwColor*)bmp.data;
    texture->image.width = bmp.header.width;
    texture->image.height = bmp.header.height;

    return &(texture->image);
}

/* Imports a texture from a .tga file */
extern "C"
gwTexture* gwTexture_loadTGA( const char* filename )
{
    Texture* texture = new Texture;
    gwTgaImage tga = gw_tga_import( filename, true );
    if (tga.data == nullptr || tga.header.width <= 0 || tga.header.height <= 0){
        /* failed to load the image */
        return &(texture->image);
    }

    if (tga.bpp != 4){
        /* this should not never happens, but just in case... */
        _handle_error_( "Pixel format not supported!" );
        return &(texture->image);
    }

    texture->internalFormat = GL_COMPRESSED_RGBA;
    texture->format = GL_BGRA;
    texture->bpp = 4;
    texture->image.buffer = (gwColor*)tga.data;
    texture->image.width = tga.header.width;
    texture->image.height = tga.header.height;

    return &(texture->image);
}

/* Imports a texture from a file. Takes the format from the extension */
extern "C"
gwTexture* gwTexture_load( const char* filename )
{
    /* Checks the extension of the filename */
    size_t i = 0;
    char dot[8] = { '\0' };

    while (filename[i] != '\0'){
        if (filename[i++] == '.'){
            strcpy_safe( dot, &(filename[i]), 7 );
        }
    }

    str_upcase( dot );
    if (strcmp( dot, "TGA" ) == 0){
        return gwTexture_loadTGA( filename );
    }
    else if (strcmp( dot, "BMP" ) == 0){
        return gwTexture_loadBMP( filename );
    }
    else if (strcmp( dot, "DIB" ) == 0){
        return gwTexture_loadBMP( filename );
    }
    else{
        _handle_error_( "Unsupported image file type .%s!", dot );
        Texture* tex = new Texture;
        return &(tex->image);
    }
}

/* Saves the image in a .bmp file */
extern "C"
int gwTexture_saveBMP( const gwTexture* texture, const char* filename )
{
    if (texture == nullptr){
        return 1;
    }

    /* format should be GL_BGRA */
    lock_threads();
    int r = gw_bmp_save( filename
        , (uint8_t*)texture->buffer
        , texture->width, texture->height, 32, 0 );
    unlock_threads();

    return 0;
}

/* Modifies one color by setting its alpha channel to 0.
 * Tolerance indicates the distance to the reference color */
extern "C"
void gwTexture_transparencyMask( const gwTexture* tex
, const gwColor color, const int tolerance )
{
    Texture* texture = (Texture*)tex;
    if (check_object( texture )) return;

    /* format should be GL_BGRA */
    texture->transparencyMask( color, tolerance );
}

/* Deletes a texture */
extern "C"
void gwTexture_delete( gwTexture* tex )
{
    Texture* texture = (Texture*)tex;
    if (check_object( texture )) return;

    /* format should be GL_BGRA */
    texture->dispose();
}

/* Creates a texture with a text */
extern "C"
gwTexture* gwText( const int ftid, const char* text, const int size, gwColor color )
{
    /* Create a bitmap with the text */
    BitmapTag bitmap = font_write_richtext( ftid, text, size );

    /* Creates a texture from the bitmap */
    gwTexture* image = gwTexture_create( bitmap.width, bitmap.height );
    if (image == nullptr){
        return nullptr;
    }

    /* Copy the bitmap into the texture. Textures are always in RGBA format.
    * The color of the text is defined with the primary_color, and
    * the background with the secondary color. */
    int ibit = bitmap.height * bitmap.width - 1;
    for (int i = 0; i < bitmap.height * bitmap.width; i++){
        int bb = bitmap.buffer[ibit];
        ibit--;
        image->buffer[i].r = (bb * color.r) / 255;
        image->buffer[i].g = (bb * color.g) / 255;
        image->buffer[i].b = (bb * color.b) / 255;
        image->buffer[i].a = (bb * color.a) / 255;
    }

    /* It is no longer needed */
    BitmapTag_dispose( &bitmap );

    return image;
}

/* Creates a label with a text */
extern "C"
gwMesh* gwLabel( const int ftid, const char* text, const int size )
{
    /* Create a bitmap with the text */
    BitmapTag bitmap = font_write_richtext( ftid, text, size );

    /* Creates a texture from the bitmap */
    gwTexture* image = gwTexture_create( bitmap.width, bitmap.height );

    if (image == nullptr){
        return nullptr;
    }

    /* Copy the bitmap into the texture. Textures are always in RGBA format.
    * The color of the text is defined with the primary_color, and
    * the background with the secondary color. */
    for (int i = 0; i < bitmap.width * bitmap.height; i++){
        int bb = bitmap.buffer[i];
        image->buffer[i].r = 255 - bb;
        image->buffer[i].g = 255 - bb;
        image->buffer[i].b = 255 - bb;
        image->buffer[i].a = bb;
    }

    /* Create a mesh quad */
    gwMesh* mesh = gwShape_sprite( bitmap.width, bitmap.height );

    /* It is no longer needed */
    BitmapTag_dispose( &bitmap );

    /* There is a shader especialized for writting text */
    mesh->shaders = GW_SHADER_FONT;
    mesh->primary_color = gwColorBlack;
    mesh->secondary_color = gwColorWhite;
    mesh->texture0 = image;

    /* We need to invert the texture in the vertical direction.
     * This is becuase OpenGL uses the convention that the [0,0] coordinate
     * is the lower left, while FreeType uses the upper left */
    for (size_t i = 0; i < mesh->vertex->length; i++){
        gwFloat v = mesh->vertex->stream[i].texCoord.v;
        mesh->vertex->stream[i].texCoord.v = 1 - v;
    }

    return mesh;
}

/* Modifies an existing label */
extern "C"
gwMesh* gwLabel_update( gwMesh* mesh, const int ftid, const char* text, const int size )
{
    gwTexture_delete( mesh->texture0 );
    mesh->texture0 = gwText( ftid, text, size, gwColorBlack );
    gwFloat w = (gwFloat)mesh->texture0->width;
    gwFloat h = (gwFloat)mesh->texture0->height;

    gwVertexStream_set( mesh->vertex, 0, 0, 0, 0, 0, 0, 1., 1, 0 );
    gwVertexStream_set( mesh->vertex, 1, w, 0, 0, 0, 0, 1, 0, 0 );
    gwVertexStream_set( mesh->vertex, 2, w, h, 0, 0, 0, 1, 0, 1 );
    gwVertexStream_set( mesh->vertex, 3, 0, h, 0, 0, 0, 1, 1, 1 );
    mesh->vertex->update = 1;

    return mesh;
}

