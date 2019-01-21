/**
Author: Mario J. Martin <dominonurbs$gmail.com>

This example draw geometries from scratch using three OpenGL versions.
It only covers until OpenGL 3x, but higher versions only add functionalities. 

Rendering is performed through a continuous loop until the window is closed.
In the simplest rendering application, drawing a scene object requires
to create the vertex and index buffers and render them in the GL context.
Since OpenGL 3x, it also requires a shader to work, although for OpenGL 2x, 
we also use shaders, instead of the default shader.

*/

#include <stdio.h>
#include <stdlib.h>

#include "common/definitions.h"
#include "common/log.h"

#include "glw/gwgl.h"
#include "GLFW/glfw3.h"

#include "glw/defines.h"
#include "glw/gwthreads.h"
#include "glw/shapes.h"
#include "glw/gwmath.h"

#include "glw/ShaderProgram.hpp"

#define BUFFER_OFFSET(bytes) ((GLubyte*)NULL + (bytes))

int rendering = 0;

GLFWwindow* window = nullptr;

gwMesh* surface = gwShape_cube();
gwMesh* wireframe = gwWireframe_cube();

GLuint vbo;     // Vertex Buffer Object
GLuint ibos;    // Index Buffer Object for surface
GLuint ibow;    // Index Buffer Object for wireframe

GLuint vaos;     // Vertex Array Object for drawing the surface
GLuint vaow;     // Vertex Array Object for drawing the wireframe
GLuint vaop;     // Vertex Array Object for drawing the points

gwCamera camera;
int mouse_left_pressed = 0;
int mouse_right_pressed = 0;
double mouse_pressed_coord_x = 0;
double mouse_pressed_coord_y = 0;

gw::ShaderProgram shader;
GLint unifShade = -1;
GLint unifModelViewProjMatrix = -1;
GLint attPosition = -1;
GLint attNormal = -1;
GLint attTexCoord = -1;
GLint attScalar = -1;
GLint attVector = -1;
GLint frgColor = -1;
GLint frgDepth = -1;

/* Check the error messages from OpenGL */
GLenum check_gl_error()
{
    GLenum er = glGetError();

    if (er == GL_OUT_OF_MEMORY){
        _handle_error_( "OpenGL - Out of memory!" );
    }
    else if (er != GL_NO_ERROR){
        _handle_error_( "OpenGL - error = %i", er );
    }
    return er;
}

/* GLFW error callback, when an error is reported */
static void error_callback( int error, const char* description )
{
    printf( description );
}

GLFWwindow* create_window( const int gl_version_hint )
{
    /* GLFW library must be initialized.
    * This function may only be called from the main thread.
    * Additional calls to this function will do nothing. */
    if (glfwInit() == GL_FALSE){
        _handle_error_( "Cannot initialize GLFW!\n" );
        return nullptr;
    }

    glfwMakeContextCurrent( NULL );
    glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_API );

    /* Create the render context */
    GLFWwindow* window = NULL;
    if (gl_version_hint >= 32){
        /* #if __APPLE__ Macs only support core versions of OpenGL */
        _log_( "Trying OpenGL 3.2 core..." );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        window = glfwCreateWindow( 800, 600, "OpenGL 3.1", NULL, NULL );
    }

    if (window == NULL && gl_version_hint >= 31){
        _log_( "Trying OpenGL 3.1 ..." );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );
        window = glfwCreateWindow( 800, 600, "OpenGL 3.1", NULL, NULL );
    }

    if (window == NULL && gl_version_hint >= 21){
        /* Trying to create a older version of OpenGL */
        _log_( "Trying OpenGL 2.1 ..." );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );
        window = glfwCreateWindow( 800, 600, "OpenGL 2.1", NULL, NULL );
    }

    if (window == NULL && gl_version_hint >= 11){
        /* Trying to create a legacy version of OpenGL */
        _log_( "Trying OpenGL 1.1 ..." );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 1 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );
        window = glfwCreateWindow( 800, 600, "OpenGL 1.1", NULL, NULL );
    }

    if (window == NULL){
        _handle_error_( "Cannot create GLFW render window\n" );
        return nullptr;
    }

    glfwMakeContextCurrent( window );

    /* Initialize GLEW */
    glewExperimental = false;
    GLenum err = glewInit();
    if (err != GLEW_OK){
        _handle_error_( "GLEW is unable to initialize: %s"
            , glewGetErrorString( err ) );
        return nullptr;
    }

    const unsigned char* gl_version = glGetString( GL_VERSION );
    const unsigned char* gl_shading_language = glGetString( GL_SHADING_LANGUAGE_VERSION );
    const unsigned char* gl_renderer = glGetString( GL_RENDERER );
    int mayor_version, minor_version;
    sscanf( (char*)gl_version, "%i.%i", &mayor_version, &minor_version );
    _log_( "OpenGL version: %s", gl_version );
    _log_( "Shading Language: %s", gl_shading_language );
    _log_( "Renderer: %s", gl_renderer );

    check_gl_error();

    return window;
}

/******************************************************************************/

gwVector3f calculate_camera_displacement
( const gwCamera* camera, const gwFloat dx, const gwFloat dy, const gwFloat dz )
{
    /* Performs the same rotations, but in inverse order */
    gwMatrix4f temp;
    gwFloat iz = (gwFloat)1 / camera->zoom;
    gwFloat ix = iz * (gwFloat)1 / camera->scale.x;
    gwFloat iy = iz * (gwFloat)1 / camera->scale.y;
    gwMatrix4f matrix = gwMatrix4f_identity();

    temp = gwMatrix4f_rotation( camera->rotation_angles.y, 0.0f, 1.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera->rotation_angles.x, 0.0f, 0.0f, -1.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera->rotation_angles.z, -1.0f, 0.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( 180.0f, 0.0f, 1.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_scale( ix, iy, iz * camera->farClip );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    gwVector3f vec = { dx, dy, dz };
    gwVector3f delta = gwVector3f_mul( &matrix, &vec );

    return delta;
}

gwMatrix4f gwCamera_calculate_view( const gwCamera camera )
{
    gwMatrix4f matrix, temp;

    matrix = gwMatrix4f_identity();
    temp = gwMatrix4f_translation( 0.0f, 0.0f, -camera.nearClip );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_scale
        ( camera.scale.x * camera.zoom
        , camera.scale.y * camera.zoom
        , (camera.scale.z * camera.zoom) / camera.farClip
        );

    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera.rotation_angles.z, 1.0f, 0.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera.rotation_angles.x, 0.0f, 0.0f, 1.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_rotation( camera.rotation_angles.y, 0.0f, -1.0f, 0.0f );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    temp = gwMatrix4f_translation
        ( camera.eye.x, camera.eye.y, camera.eye.z );
    matrix = gwMatrix4f_mul( &matrix, &temp );

    return matrix;
}

/******************************************************************************/

static void key_callback
( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose( window, GL_TRUE );
}

static void window_iconify_callback( GLFWwindow* window, int iconified )
{
    if (iconified){
        // The window was iconified
        rendering = false;
    }
    else{
        // The window was restored
        rendering = true;
    }
}

static void mouse_button_callback
( GLFWwindow* window, int button, int action, int mods )
{
    static double seconds = 0;
    double time_between_clicks = glfwGetTime() - seconds;
    seconds = glfwGetTime();
    if (time_between_clicks < 0.5){
        /* double click */
    }

    if (action == GLFW_PRESS){
        double mx = mouse_pressed_coord_x, my = mouse_pressed_coord_y;
        if (mouse_left_pressed == 0 && mouse_right_pressed == 0){
            glfwGetCursorPos( window, &mx, &my );
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT){
            mouse_left_pressed = 1;
            mouse_pressed_coord_x = mx;
            mouse_pressed_coord_y = my;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT){
            mouse_right_pressed = 1;
            mouse_pressed_coord_x = mx;
            mouse_pressed_coord_y = my;
        }
        if (button == GLFW_MOUSE_BUTTON_MIDDLE){
        }
    }
    else if (action == GLFW_RELEASE){
        if (button == GLFW_MOUSE_BUTTON_LEFT){
            mouse_left_pressed = 0;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT){
            mouse_right_pressed = 0;
        }
        if (button == GLFW_MOUSE_BUTTON_MIDDLE){
        }
    }
}

static void scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
    camera.zoom += gwFloat(yoffset * 0.05) * camera.zoom;
}

/* Called everytime the window resizes */
static void resize_callback( GLFWwindow* window, int width, int height )
{
}

/* Called everytime the pointer moves */
static void mouse_move_callback( GLFWwindow* window, double xpos, double ypos )
{
    double delta_x = 0, delta_y = 0;
    if (mouse_right_pressed != 0 || mouse_left_pressed != 0){
        delta_x = mouse_pressed_coord_x - xpos;
        delta_y = mouse_pressed_coord_y - ypos;
        mouse_pressed_coord_x = xpos;
        mouse_pressed_coord_y = ypos;
    }
    if (mouse_right_pressed != 0 && mouse_left_pressed == 0){
        /* Rotate camera */
        camera.rotation_angles.x += gwFloat( delta_x );
        camera.rotation_angles.z += gwFloat( delta_y );
    }
    else if (mouse_right_pressed == 0 && mouse_left_pressed != 0){
        /* Move the camera */
        int window_width = 1, window_height = 1;
        glfwGetWindowSize( window, &window_width, &window_height );
        gwFloat dx = (2 * (gwFloat)delta_x / window_height);
        gwFloat dy = (2 * (gwFloat)delta_y / window_height);

        gwVector3f displ = calculate_camera_displacement
            ( &camera, dx, dy, 0 );

        camera.eye.x += displ.x;
        camera.eye.y += displ.y;
        camera.eye.z += displ.z;
    }
}

/* This is how geometries were drawn in former OpenGL 1.x. */
void render_GL1x()
{
    int screen_width, screen_height;
    glfwGetWindowSize( window, &screen_width, &screen_height );

    /* Update the camera */
    glMatrixMode( GL_PROJECTION );
    gwMatrix4f m_scale 
        = gwMatrix4f_scale( (float)screen_height / screen_width, 1, 1 );
    gwMatrix4f matrix = gwCamera_calculate_view( camera );
    matrix = gwMatrix4f_mul( &m_scale, &matrix );
    glLoadMatrixf( matrix.data );

    glPointSize( 12.0f );
    glDepthRange( 0.0f, 1.0f - 0.02f );
    glBegin( GL_POINTS );
    for (size_t iv = 0; iv < surface->vertex->length; iv++){
        gwFloat r = surface->vertex->stream[iv].position.x + 0.5f;
        gwFloat g = surface->vertex->stream[iv].position.y + 0.5f;
        gwFloat b = surface->vertex->stream[iv].position.z + 0.5f;
        glColor3f( r * 0.8f, g * 0.8f, b * 0.8f );
        glVertex3f
            ( surface->vertex->stream[iv].position.x
            , surface->vertex->stream[iv].position.y
            , surface->vertex->stream[iv].position.z
            );
    }
    glEnd();

    glLineWidth( 1.0f );
    glDepthRange( 0.0f, 1.0f - 0.01f );
    glColor3f( 0.0f, 0.0f, 0.0f );

    if (wireframe->index != nullptr){
        glBegin( wireframe->index->primitive );
        if (wireframe->index->stream != nullptr){
            for (size_t i = 0; i < wireframe->index->length; i++){
                unsigned int iv = wireframe->index->stream[i];
                glVertex3f
                    ( wireframe->vertex->stream[iv].position.x
                    , wireframe->vertex->stream[iv].position.y
                    , wireframe->vertex->stream[iv].position.z
                    );
            }
        }
        else{
            for (size_t iv = 0; iv < wireframe->vertex->length; iv++){
                glVertex3f
                    ( wireframe->vertex->stream[iv].position.x
                    , wireframe->vertex->stream[iv].position.y
                    , wireframe->vertex->stream[iv].position.z
                    );
            }
        }
        glEnd();
    }

    glDepthRange( 0.0f, 1.0f );
    if (surface->index != nullptr){
        glBegin( surface->index->primitive );
        if (surface->index->stream != nullptr){
            for (size_t i = 0; i < surface->index->length; i++){
                unsigned int iv = surface->index->stream[i];
                gwFloat r = surface->vertex->stream[iv].position.x + 0.5f;
                gwFloat g = surface->vertex->stream[iv].position.y + 0.5f;
                gwFloat b = surface->vertex->stream[iv].position.z + 0.5f;
                glColor3f( r, g, b );
                glVertex3f
                    ( surface->vertex->stream[iv].position.x
                    , surface->vertex->stream[iv].position.y
                    , surface->vertex->stream[iv].position.z
                    );
            }
        }
        else{
            for (size_t iv = 0; iv < surface->vertex->length; iv++){
                glVertex3f
                    ( surface->vertex->stream[iv].position.x
                    , surface->vertex->stream[iv].position.y
                    , surface->vertex->stream[iv].position.z
                    );
            }
        }
        glEnd();
    }
}

/******************************************************************************/

/* Creates a vertex buffer object */
GLuint create_vbo( const gwVertexStream* vs )
{
    GLuint vbo;

    /* Request a new buffer */
    glGenBuffers( 1, &vbo );

    if (vbo == 0){
        _handle_error_( "Failed to generate the vertex buffer object!" );
        return 0;
    }

    /* Hook the buffer to the context */
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    /* Copy data to the GPU buffer */
    glBufferData( GL_ARRAY_BUFFER
        , sizeof( gwVertex ) * vs->length, vs->stream, GL_DYNAMIC_DRAW );

    return vbo;
}

/* Creates an index buffer object */
GLuint create_ibo( const gwIndexStream* ind )
{
    GLuint ibo;

    /* Request a new buffer */
    glGenBuffers( 1, &ibo );

    if (ibo == 0){
        _handle_error_( "Failed to create the index buffer object!" );
        return 0;
    }

    /* Hook the buffer to the context */
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );

    /* Copy data to the buffer */
    glBufferData( GL_ELEMENT_ARRAY_BUFFER
        , sizeof( gwIndex ) * ind->length, ind->stream, GL_STATIC_DRAW );

    return ibo;
}

void startGL2x()
{
    /* Load the shader program */
    int status;
    status = shader.compile( "../dev/zdev01/shader_gl2.vsh"
        , "../dev/zdev01/shader_gl2.fsh", nullptr );
    status += shader.link();

    if (status == gw::ShaderStatus::SHADER_OK){
        unifShade = glGetUniformLocation( shader.program, "Shade" );
        unifModelViewProjMatrix
            = glGetUniformLocation( shader.program, "ModelViewProjMatrix" );

        attPosition = glGetAttribLocation( shader.program, "in_Position" );
        attNormal = glGetAttribLocation( shader.program, "in_Normal" );
        attTexCoord = glGetAttribLocation( shader.program, "in_TexCoord" );
        attScalar = glGetAttribLocation( shader.program, "in_Scalar" );
        attVector = glGetAttribLocation( shader.program, "in_Vector" );
     }

    /* Create the buffer objects */
    vbo = create_vbo( surface->vertex );
    ibos = create_ibo( surface->index );
    ibow = create_ibo( wireframe->index );

    /* Unbind the buffers, so they cannot be changed by a later process */
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

/* Enables client vertex data format */
inline void vertex_enable
( const int attPosition, const int attTexCoord, const int attNormal )
{
    glEnableVertexAttribArray( attPosition );

    if (attNormal >= 0){
        glEnableVertexAttribArray( attNormal );
    }

    if (attTexCoord >= 0){
        glEnableVertexAttribArray( attTexCoord );
    }
}

/* Sets the vertex format */
inline void vertex_format
( const int attPosition, const int attTexCoord, const int attNormal )
{
    glVertexAttribPointer( attPosition, 3, GL_FLOAT, GL_FALSE
        , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, position ) );

    if (attNormal >= 0){
        glVertexAttribPointer( attNormal, 3, GL_FLOAT, GL_TRUE
            , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, normal ) );
    }

    if (attTexCoord >= 0){
        glVertexAttribPointer( attTexCoord, 2, GL_FLOAT, GL_FALSE
            , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, texCoord ) );
    }
}

/* Disables client vertex data format */
inline void vertex_disable
( const int attPosition, const int attNormal, const int attTexture )
{
    glDisableVertexAttribArray( attPosition );

    if (attNormal >= 0){
        glDisableVertexAttribArray( attNormal );
    }
    if (attTexture >= 0){
        glDisableVertexAttribArray( attTexture );
    }
}

void draw_mesh_GL2x()
{
    /* Use VBO only */
    glBindVertexArray( 0 );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    vertex_enable( attPosition, attNormal, attTexCoord );
    vertex_format( attPosition, attNormal, attTexCoord );

    /* Draw the points */
    glPointSize( 12.0f );
    glPolygonMode( GL_FRONT_AND_BACK, GL_POINTS );
    glUniform1f( unifShade, 0.8f );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glDrawArrays( GL_POINTS, 0, surface->vertex->length );

    /* Draw the wireframe */
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glUniform1f( unifShade, 0.0f );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibow );
    glDrawElements( wireframe->index->primitive
        , wireframe->index->length
        , GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );

    /* Draw the surface */
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glUniform1f( unifShade, 1.0f );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibos );
    glDrawElements( surface->index->primitive
        , surface->index->length
        , GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );

    vertex_disable( attPosition, attNormal, attTexCoord );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

/* This is how geometries can be drawn in former OpenGL 2.x.
* Actually is somehow the intersection between OpenGL 3 and OpenGL 2 */
void render_GL2x()
{
    glUseProgram( shader.program );

    int screen_width, screen_height;
    glfwGetWindowSize( window, &screen_width, &screen_height );
    gwMatrix4f m_scale = gwMatrix4f_scale
        ( (float)screen_height / screen_width, 1, 1 );
    gwMatrix4f matrix = gwCamera_calculate_view( camera );
    matrix = gwMatrix4f_mul( &m_scale, &matrix );

    glUniformMatrix4fv( unifModelViewProjMatrix, 1, GL_FALSE, (gwFloat*)(&matrix) );

    draw_mesh_GL2x();
}

void dispose_GL2x()
{
    glDeleteBuffers( 1, &vbo );
    glDeleteBuffers( 1, &ibos );
    glDeleteBuffers( 1, &ibow );
}

/******************************************************************************/

/* Sets the vertex data format with a vertex array */
inline void attribFormatArray
( const int attPosition, const int attNormal, const int attTexture )
{
    glVertexAttribPointer( attPosition, 3, GL_FLOAT, GL_FALSE
        , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, position ) );
    glEnableVertexAttribArray( attPosition );
    if (attNormal >= 0){
        glVertexAttribPointer( attNormal, 3, GL_FLOAT, GL_TRUE
            , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, normal ) );
        glEnableVertexAttribArray( attNormal );
    }
    if (attTexture >= 0){
        glVertexAttribPointer( attTexture, 2, GL_FLOAT, GL_FALSE
            , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, texCoord ) );
        glEnableVertexAttribArray( attTexture );
    }
}

GLuint create_vao
( const gwVertexStream* vertex
, const gwIndexStream* index 
)
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData
        ( GL_ARRAY_BUFFER
        , sizeof( gwVertex ) * vertex->length
        , vertex->stream
        , GL_DYNAMIC_DRAW
        );

    /* These numbers are the attrib locations: position, normal, textcoord */
    attribFormatArray( 0, 1, 2 );

    if (index != nullptr){
        glGenBuffers( 1, &ibos );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibos );
        glBufferData
            ( GL_ELEMENT_ARRAY_BUFFER
            , sizeof( gwIndex ) * index->length
            , index->stream, GL_STATIC_DRAW
            );
    }
    glBindVertexArray( 0 );

    return vao;
}

void startGL3x()
{
    /* Load the shader program */
    int status;
    status = shader.compile( "../dev/zdev01/shader_gl3.vsh"
        , "../dev/zdev01/shader_gl3.fsh", nullptr );

    /* Forces the attrib location, prior to link. */
    glBindAttribLocation( shader.program, 0, "in_Position" );
    glBindAttribLocation( shader.program, 1, "in_Normal" );
    glBindAttribLocation( shader.program, 2, "in_TexCoord" );
    glBindAttribLocation( shader.program, 3, "in_Scalar" );
    glBindAttribLocation( shader.program, 4, "in_Vector" );

    status += shader.link();

    if (status == gw::ShaderStatus::SHADER_OK){
        unifShade = glGetUniformLocation( shader.program, "Shade" );
        unifModelViewProjMatrix
            = glGetUniformLocation( shader.program, "ModelViewProjMatrix" );

        /* Should be the locations defined before */
        attPosition = glGetAttribLocation( shader.program, "in_Position" );
        attNormal = glGetAttribLocation( shader.program, "in_Normal" );
        attTexCoord = glGetAttribLocation( shader.program, "in_TexCoord" );
        attScalar = glGetAttribLocation( shader.program, "in_Scalar" );
        attVector = glGetAttribLocation( shader.program, "in_Vector" );

        frgColor = glGetFragDataLocation( shader.program, "out_Color" );
        frgDepth = glGetFragDataLocation( shader.program, "out_Depth" );
    }

    /* Create the buffer objects */
    vaos = create_vao( surface->vertex, surface->index );
    vaow = create_vao( wireframe->vertex, wireframe->index );
    vaop = create_vao( surface->vertex, nullptr );
}

void draw_mesh_GL3x()
{
    glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
    glDepthRange( 0.0f, 1.0f - 0.02f );
    glPointSize( 12.0f );
    glUniform1f( unifShade, 0.8f );
    glBindVertexArray( vaop );
    glDrawArrays( GL_POINTS, 0, surface->vertex->length );

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDepthRange( 0.0f, 1.0f - 0.01f );
    glUniform1f( unifShade, 0.0f );
    glBindVertexArray( vaow );
    glDrawElements( wireframe->index->primitive
        , wireframe->index->length
        , GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDepthRange( 0.0f, 1.0f );
    glUniform1f( unifShade, 1.0f );
    glBindVertexArray( vaos );
    glDrawElements( surface->index->primitive
        , surface->index->length
        , GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );

    glBindVertexArray( 0 );
}

/* This is how geometries can be drawn in former OpenGL 2.x.
* Actually is somehow the intersection between OpenGL 3 and OpenGL 2 */
void render_GL3x()
{
    const GLenum att[] = { GL_COLOR_ATTACHMENT0 };
    glBindFramebuffer( GL_FRAMEBUFFER, 0 ); /* Default framebuffer */
    glDrawBuffers( 1, att );

    glUseProgram( shader.program );

    int screen_width, screen_height;
    glfwGetWindowSize( window, &screen_width, &screen_height );
    gwMatrix4f m_scale = gwMatrix4f_scale
        ( (float)screen_height / screen_width, 1, 1 );
    gwMatrix4f matrix = gwCamera_calculate_view( camera );
    matrix = gwMatrix4f_mul( &m_scale, &matrix );

    glUniformMatrix4fv( unifModelViewProjMatrix, 1, GL_FALSE, (gwFloat*)(&matrix) );

    draw_mesh_GL3x();
}

void dispose_GL3x()
{
    glDeleteBuffers( 1, &vaos );
    glDeleteBuffers( 1, &vaow );
    glDeleteBuffers( 1, &vaop );
    glDeleteBuffers( 1, &vbo );
    glDeleteBuffers( 1, &ibos );
    glDeleteBuffers( 1, &ibow );
}

/******************************************************************************/

void render( GLFWwindow* window, const int gl_version )
{
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glDepthFunc( GL_LESS );
    glEnable( GL_DEPTH_TEST );

    glDisable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    /* The front face follows the counter clock wise rule (OpenGL default)*/
    glFrontFace( GL_CCW );
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );

    /* If enabled, the -wc≤zc≤wc plane equation is ignored by view volume clipping
    *(effectively, there is no near or far plane clipping). */
    glDisable( GL_DEPTH_CLAMP );

    /* The antialiasing does not work very well */
    glDisable( GL_POLYGON_SMOOTH );
    glDisable( GL_LINE_SMOOTH );
    glClearDepth( 1.0f );
    glClearStencil( 0 );   

    glClearColor( 0.0f, 0.0f, 1.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    int width, height;
    glfwGetWindowSize( window, &width, &height );
    glViewport( 0, 0, (GLsizei)width, (GLsizei)height );

    if (gl_version >= 30){
        render_GL3x();
    }
    else if (gl_version >= 20){
        render_GL2x();
    }
    else{
        render_GL1x();
    }
}

void camera_init( gwCamera* camera )
{
    camera->type = GW_CAMERA_VIEW_CAD;

    camera->eye.x = 0.0f;
    camera->eye.y = 0.0f;
    camera->eye.z = 0.0f;

    camera->rotation_angles.x = 0.0f;
    camera->rotation_angles.y = 0.0f;
    camera->rotation_angles.z = 0.0f;

    camera->scale.x = 1.0f;
    camera->scale.y = 1.0f;
    camera->scale.z = 1.0f;

    camera->zoom = 1.0f;
    camera->angle_rounding = 0;
    camera->farClip = 1.0f;
    camera->nearClip = 0.0f;
}

int main()
{
    int gl_version = 32;
    int width, height;
    int screen_width = 0, screen_height = 0;

    glfwSetErrorCallback( error_callback );

    /* Initialize the camera */
    camera_init( &camera );

    /* Creates the window context */
    window = create_window( gl_version );
    if (window == nullptr){
        glfwTerminate();
        getchar();
        return 1;
    }

    glfwGetFramebufferSize( window, &screen_width, &screen_height );

    glViewport( 0, 0, screen_width, screen_height );
    glfwSetInputMode( window, GLFW_STICKY_MOUSE_BUTTONS, 1 );
    glfwSetKeyCallback( window, key_callback );
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetScrollCallback( window, scroll_callback );
    glfwSetWindowSizeCallback( window, resize_callback );
    glfwSetCursorPosCallback( window, mouse_move_callback );

    glfwSetWindowUserPointer( window, malloc( sizeof( int ) ) );
    int* pslot = (int*)glfwGetWindowUserPointer( window );
    if (pslot != nullptr){
        *pslot = 1001;
    }
    /* Make the window's context current */
    glfwMakeContextCurrent( window );

    glClearDepth( 1.0f );

    /* Initializes the visualization */
    if (gl_version >= 30){
        startGL3x();
    }
    else  if (gl_version >= 20){
        startGL2x();
    }

    /* Loop until the user closes the window */
    rendering = 1;
    while (!glfwWindowShouldClose( window ))
    {
        glfwSetTime( 0.0 );

        int visible = glfwGetWindowAttrib( window, GLFW_VISIBLE );
        if (rendering != 0 && visible != 0){
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glfwGetFramebufferSize( window, &width, &height );
            if (width != screen_width || height != screen_height){
                /* resize */
                screen_width = width;
                screen_height = height;
            }

            /* Render here */
            render( window, gl_version );

            /* Swap front and back buffers */
            glfwSwapBuffers( window );
            int time_rendering = int( glfwGetTime() * 1000 );
            if (time_rendering < 50){
                gw_sleep( 50 - time_rendering );
            }
            /* Poll for all process events */
            glfwPollEvents();
        }
    }

    /* Initializes the visualization */
    if (gl_version == 2){
        dispose_GL2x();
    }
    else  if (gl_version == 3){
        dispose_GL3x();
    }

    glfwDestroyWindow( window );
    glfwTerminate();

    return 0;
}
