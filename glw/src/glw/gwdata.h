/***
    Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#ifndef _HGW_VERTEX_DATA_H
#define _HGW_VERTEX_DATA_H

#include <stdint.h>

#define GW_LABEL_LENGTH  1024

/* Single precission floating point, employed by the graphic card.
* Equivalent to GLflota, as defined in gl.h */
typedef float gwFloat;

/* Equivalent to GLint, as defined in gl.h */ 
typedef int32_t gwInt;

/* Double precission floating point, employed by the CPU. */
typedef double gwDouble;

/* Basic type employed by the graphic card fot the index stream.
* Strictly, it should be GLuint, GLushort, or GLubyte as defined in gl.h */
typedef unsigned int gwIndex;

/* This is known as the primitive restart index */
#define gw_primitive_restart_index = -1;

/* Three dimensional vertex coordinates.
* Used for the vertex stream. */
typedef struct
{
    gwFloat x, y, z;
}gwVector3f;

/* 4-dimensional vertex coordinates.
* Used for the vertex stream. */
typedef struct
{
    gwFloat x, y, z, w;
}gwVector4f;

/* A rectangle. */
typedef struct
{
    int x0, y0, width, height, fbo;
}gwViewPort;

/* Two dimensional texture coordinates for the vertex */
typedef struct
{
    gwFloat u, v;
}gwTexCoord;

/* Type to hold colors. Most graphic cards are optimized to BGRA
* instead of RGBA, which is considered a legacy */
typedef struct
{
    uint8_t b;  /* Blue */
    uint8_t g;  /* Green */
    uint8_t r;  /* Red */
    uint8_t a;  /* Alpha channel */
}gwColor;

/* Default vertex format of the render vertices with position, normal, and texture.
* The total size is 8 bytes, so no padding is needed */
typedef struct
{
    gwTexCoord texCoord;    /* Texture coordinates  */
    gwVector3f normal;      /* Surface normal */
    gwVector3f position;    /* Position */
}gwVertex;

/* A 4x4 transformation matrix */
typedef struct
{
    gwFloat data[16];
}gwMatrix4f;

/* Main vertex stream; used to send vertex data to the GPU,
 * with fields for position, normal, and texture coordinates. */
typedef struct
{
    /* Stream data */
    gwVertex* stream;

    /* Length of the data */
    size_t length;

    /* Indicates the render to update the stream, with new data */
    int update;
}gwVertexStream;

/* Vertex stream for a single value; used to send vertex data to the GPU */
typedef struct
{
    /* Stream data */
    gwFloat* stream;

    /* Length of the data */
    size_t length;

    /* Indicates the render to update the stream, with new data */
    int update;
}gwScalarStream;

/* Vertex stream for vector values; used to send vertex data to the GPU */
typedef struct
{
    /* Stream data */
    gwVector4f* stream;

    /* Length of the data */
    size_t length;

    /* Indicates the render to update the stream, with new data */
    int update;
}gwVectorStream;

/* Vertex stream; used to send the vertex data to the GPU */
typedef struct
{
    /* Stream data indicating the connectivities of the elements */
    gwIndex* stream;

    /* Length of the data */
    size_t length;

    /* Indicates the render to update the stream, with new data */
    int update;

    /* Indicates the primitive:
    GW_INDEX_POINTS, GW_INDEX_LINES, GW_INDEX_LINE_LOOP, 
    GW_INDEX_LINE_STRIP, GW_INDEX_TRIANGLES, GW_INDEX_TRIANGLE_STRIP, 
    Values are the same as its corresponding OpenGL GL_POINTS, ...*/
    int primitive;
}gwIndexStream;

/* Structure to hold images. */
typedef struct
{
    /* All images are 32 bits and BGRA format */
    gwColor* buffer;

    /* Width of the image in pixels */
    int width;

    /* Height of the image in pixels */
    int height;

    /* Indicates to update or regenerate the texture in the GPU */
    int update;

    /* Indicates the type of interpolation. 1: (default) linear; 0: nearest */
    int interpolation;
}gwTexture;

typedef struct
{
    /* The render is made to this texture */
    gwTexture* color_texture;

    /* Width of the target in pixels. 
     * For performance, it is convenient that dimensions are multiple of 2; 
     * e.g. 128, 256, 512, etc..
     * Assign 0 to use the same screen dimensions */
    int width;

    /* Height of the target in pixels.
     * For performance, it is convenient that dimensions are multiple of 2;
     * e.g. 128, 256, 512, etc..
     * Assign 0 to use the same screen dimensions */
    int height;

    /* Clear color */
    gwColor background_color;
}gwRenderTarget;

/* A mesh is a drawing entity that combines the vertex and index stream, 
 * a well other properties, such as the primary color, shaders, ... */
typedef struct _gwMesh
{
    /* A description or name to identify the shader */
    char label[GW_LABEL_LENGTH];

    /* Activates or deactivates the rendering.
     * If show = -1, all childs are hidden as well */
    int show;

    /* Vertex stream with the points */
    gwVertexStream* vertex;

    /* Index stream with the connectivities */
    gwIndexStream* index;

    /* Vertex stream with scalar values */
    gwScalarStream* scalar;

    /* Vertex stream with vector values */
    gwVectorStream* vector;

    /* Indicates to update the vertex and index streams in he GPU */
    int update;

    /* Position of the object in the world or relative to its parent */
    gwVector3f position;

    /* Euler angles of the object */
    gwVector3f rotation;

    /* Scale to the 3 axis */
    gwVector3f scale;

    /* Size of object */
    gwFloat size;

    /* Relative position in screen [-1,1] coordinates */
    gwVector3f blit_coords;

    /* Primary color. The exact use, depends on the shader */
    gwColor primary_color;

    /* Secondary color. The exact use, depends on the shader */
    gwColor secondary_color;

    /* This is usually added to the ambient light  */
    gwFloat self_ilumination;

    /* 0 there is no culling, 1 hide back surfaces, -1 hide front surfaces */
    int culling;
   
    /* Size of the points */
    float point_size;

    /* Width of the line. (IT MIGHT NOT WORK WITH OPENGL 3!) */
    float line_width;

    /* Used to explicitly enable shaders */
    int shaders;

    /* Image used as texture. (Use the appropiate shader to render it) */
    gwTexture* texture0;

    /* The strength the primary color blends with the texture. 
     * A value of 0 indicates that the primary color is ignored */
    float texture_blend;

    /* Used to normalize the scalar valçues */
    gwFloat scalar_range_min;
    gwFloat scalar_range_max;

    /* Parent in the hierarchy. It is only used for calculating the transformation matrix */
    _gwMesh* parent;

    /* A blank pointer; used to instanciate data associated with the mesh */
    void* attachment;

    /* Function to implement behaviours when the mouse clicks on the mesh */
    void( *f_on_mouse_click )(_gwMesh* mesh, const int vertex_id, const int button);

    /* Function to implement behaviours when the mouse clicks on the mesh */
    void( *f_on_mouse_release )(_gwMesh* mesh, const int vertex_id, const int button);

    /* Function to implement behaviours when the mouse clicks on the mesh */
    void( *f_on_mouse_enter )(_gwMesh* mesh, const int vertex_id, const int button);

    /* Function to implement behaviours when the mouse clicks on the mesh */
    void( *f_on_mouse_leave )(_gwMesh* mesh, const int vertex_id, const int button);

    /* Function called when the destructor is called */
    void( *f_on_dispose )(_gwMesh* mesh);
}gwMesh;

/* The window context is generated when a window is created. */
typedef struct _gwWindowContext
{
    /* The OpenGL version the context has been created */
    int gl_version_hint;

    /* If it is minimized, the window stops rendering.
     * This is dynamically updated. */
    int is_rendering;  

    /* Window width and height in pixels. Updated when the window resizes */
    int screen_width, screen_height;

    /* Background color when the render area is cleared */
    gwColor background_color;

    /* The rendering is only activated on events, like mouse
     * This is used to force to redraw the scene. */
    int update;

    /* Indicates an action to GW_WINDOW_OPEN or GW_WINDOW_CLOSE */
    int action;

    /* auxiliary buffer */
    gwRenderTarget* offscreen_buffer;

    /* Quad used for multipass and postprocessing */
    gwMesh* quad;

    /* Pointer to an operation performed each reander loop, before rendering. */
    void( *f_on_render )(_gwWindowContext* context);

    /* Function pointer that triggers on keyboard inputs. */
    void( *f_on_keycharpress )(_gwWindowContext* context, unsigned int codepoint);

}gwWindowContext;

typedef struct _gwShader
{
    /* A description or name to identify the shader */
    char label[GW_LABEL_LENGTH];

    /* Indicates the output: 
     * GW_TARGET_SCREEN, GW_TARGET_PICKING, GW_TARGET_OFFSCREEN_BUFFER */
    int target;

    /* Indicates to reload the shader */
    int update;

    /* Indicates if this shader is appropiated for transparencies.
    * Notice that antialiasing and smoothing requires alpha blend */
    int alpha_test;

    /* Indicates if this shader ignores the depth test.
     * This is usually used for drawing icons that overlap the scene. */
    int depth_test;

    /* Indicates if the shader is especialized to draw points, lines or surfaces
     * GW_POLYGON_NONE, GW_POLYGON_POINT, GW_POLYGON_LINE, GW_POLYGON_FILL */
    int polygon;

    /* Ambient light */
    gwColor ambient_light;

    /* Light direction */
    gwVector3f light_direction;

    /* Function to emulate the shader using OpenGL 1x */
    void( *f_gl1x )
        (const gwMesh* mesh
        , const _gwShader* shader
        , const gwMatrix4f* model_matrix
        , const gwMatrix4f* camera_matrix
        );

    _gwShader* multipass;

}gwShader;

typedef struct _gwFrame
{
    /* Indicates if the frame is active for rendering */
    int active;

    /* If is set to 1, the width and height are measured in pixels;
     * by default the size is % relative to the size of the screen */
    int size_in_pixels1_relative0;

    /* Lower left corner; in pixels or relative to the screen */
    float x0;

    /* Lower left corner; in pixels or relative to the screen */
    float y0;

    /* Upper right corner; in pixels or relative to the screen */
    float x1;

    /* Upper right corner; in pixels or relative to the screen */
    float y1;

    /* Background color */
    gwColor background_color;

    /* Used to change the order the frames */
    int preference;

}gwFrame;

/* The most important value of the camera is the transformation matrix.
* Other variables are used to store some position, rotation and scale
* to calculate the transformation matrix based on the camera type. */
typedef struct _gwCamera
{
    /* Type of camera: GW_CAMERA_SCREEN, GW_CAMERA_CAD, GW_CAMERA_AXIS, etc. */
    int type;

    /* Camera position; sometimes referred as eye position */
    gwVector3f eye;

    /* Camera orientation. Used for CAD view */
    gwVector3f rotation_angles;

    /* Scale to the 3 axis */
    gwVector3f scale;

    /* Zoom of the camera */
    gwFloat zoom;

    /* Depth clipping */
    gwFloat farClip;

    /* Near clipping */
    gwFloat nearClip;

    /* Used for the perspective view */
    gwFloat field_angle;

    /* Used only for the lookat view */
    gwVector3f target;

    /* Used only for the lookat view */
    gwVector3f lookup;

    /* Used to round the view angles (in degrees).
    * For example, if it is set to 10, the angle values are multiple of 10.
    * Set to 0 to deactivate rounding. */
    int angle_rounding;

    /* Set to zero to deactivate the camera in the render list */
    int active;

    /* Links the behaviour of the camera when the mouse is on this frame only */
    gwFrame* boundary_frame;

    /* Function triggered everytime the mouse clicks */
    void( *on_mouse_click )(_gwCamera* camera
        , const int context_id, const gwFrame* frame
        , const int button, const int dbl_click
        , const int mx, const int my, const float zdepth);

    /* Function triggered everytime the mouse moves */
    void( *on_mouse_move )(_gwCamera* camera
        , const int context_id, const gwFrame* frame, const int pressed_buttons
        , const int mx, const int my, const float zdepth
        , const int delta_x, const int delta_y);

    /* Function triggered everytime a mouse button is released */
    void( *on_mouse_release )(_gwCamera* camera
        , const int context_id, const gwFrame* frame
        , const int button, const int mx, const int my, const float zdepth);

    /* Function triggered everytime the mouse wheel moves */
    void( *on_mouse_scroll )(_gwCamera* camera
        , const int context_id, const gwFrame* frame
        , const double delta_x, const double delta_y);

    /* Function triggered everytime a key is pressed */
    void( *on_keyboard )(_gwCamera* camera
        , const int context_id, const gwFrame* frame
        , const int key, const int action
		, const int mouse_x, const int mouse_y
		, const int mouse_pressed_x, const int mouse_pressed_y );

}gwCamera;

#endif /*_HGW_VERTEX_DATA_H*/

/**/
