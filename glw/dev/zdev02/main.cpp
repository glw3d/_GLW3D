/**
Author: Mario J. Martin <dominonurbs$gmail.com>

This wraps most funtions of the previous example in a library.
So we do the same with much fewer lines of code.

GW works as machine state library, where much of the behaviour is controlled
by modifying the variables and fields; with one exception. 
Variables that start with '_' are read only and should not be modified,
unless the programmer is sure to know what is doing. 

*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "common/definitions.h"
#include "common/log.h"

#include "context_glfw/context_glfw.h"

#include "glw/gwgl.h"
#include "glw/gwthreads.h"
#include "glw/glw.h"
#include "glw/gwcamera.h"
#include "glw/gwmath.h"

#include "glw/ShaderProgram.hpp"

const int gl_hint = 11;

void draw_basic_gl1x( const gwMesh* mesh )
{
    glUseProgram( 0 );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    gwVertexStream* vertex = mesh->vertex;
    if (vertex == nullptr){
        return;
    }

    gwIndexStream* index = mesh->index;
    GLint primitive;
    if (index == nullptr){
        primitive = GL_POINTS;
    }
    else{
        primitive = index->primitive;
    }

    glShadeModel( GL_SMOOTH );

    glPointSize( (GLfloat)mesh->point_size );
    glLineWidth( (GLfloat)mesh->line_width );

    glBegin( primitive );
    if (index != nullptr){
        for (size_t i = 0; i < index->length; i++){
            gwIndex j = index->stream[i];
            gwVector3f pos = vertex->stream[j].position;
            glColor3f( pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f );
            glVertex3f( pos.x, pos.y, pos.z );
        }
    }
    else{
        for (size_t i = 0; i < vertex->length; i++){
            gwVector3f pos = vertex->stream[i].position;
            glColor3f( pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f );
            glVertex3f( pos.x, pos.y, pos.z );
        }
    }

    glEnd();
}


void draw_advanced_gl1x( const gwMesh* mesh )
{
    glUseProgram( 0 );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    const int lighting = 1;

    gwVertexStream* vertex = mesh->vertex;
    if (vertex == nullptr){
        return;
    }

    gwIndexStream* index = mesh->index;
    GLint primitive;
    if (index == nullptr){
        primitive = GL_POINTS;
    }
    else{
        primitive = index->primitive;
    }

    glShadeModel( GL_SMOOTH );

    glPointSize( (GLfloat)mesh->point_size );
    glLineWidth( (GLfloat)mesh->line_width );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    size_t offset_point = offsetof( gwVertex, position );
    offset_point /= sizeof( gwFloat );

    size_t offset_normal = offsetof( gwVertex, normal );
    offset_normal /= sizeof( gwFloat );

    size_t offset_texCoords = offsetof( gwVertex, texCoord );
    offset_texCoords /= sizeof( gwFloat );

    glVertexPointer
        ( 3, GL_FLOAT, sizeof( gwVertex )
        , (GLfloat*)vertex->stream + offset_point );

    glNormalPointer
        ( GL_FLOAT, sizeof( gwVertex )
        , (GLfloat*)vertex->stream + offset_normal );

    glTexCoordPointer
        ( 2, GL_FLOAT, sizeof( gwVertex )
        , (GLfloat*)vertex->stream + offset_texCoords );

    glBegin( primitive );

    if (index != nullptr){
        for (size_t i = 0; i < index->length; i++){
            int j = index->stream[i];
            gwVector3f pos = vertex->stream[j].position;
            glColor3f( pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f );
            glArrayElement( index->stream[i] );
        }
    }
    else{
        for (size_t i = 0; i < vertex->length; i++){
            gwVector3f pos = vertex->stream[i].position;
            glColor3f( pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f );
            glArrayElement( i );
        }
    }

    glEnd();

    glDisable( GL_LIGHTING );
    glDisable( GL_LIGHT0 );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}

void draw_rainbowfancy_gl1x
( const gwMesh* mesh, const gwShader* shader
, const gwMatrix4f* model_matrix, const gwMatrix4f* camera_matrix )
{
    glUseProgram( 0 );

    if (shader->polygon == GW_POLYGON_FILL){
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
    else if (shader->polygon == GW_POLYGON_LINE){
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (shader->polygon == GW_POLYGON_POINT){
        glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
    }
    else{
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    glMatrixMode( GL_PROJECTION );
    gwMatrix4f model_view_matrix 
        = gwMatrix4f_mul( camera_matrix, model_matrix );
    glLoadMatrixf( model_view_matrix.data );

    gwVertexStream* vertex = mesh->vertex;
    if (vertex == nullptr){
        return;
    }

    gwIndexStream* index = mesh->index;
    GLint primitive = GL_POINTS;
    if (index != nullptr){
        primitive = index->primitive;
    }

    glShadeModel( GL_SMOOTH );
    glPointSize( (GLfloat)mesh->point_size );
    glLineWidth( (GLfloat)mesh->line_width );

    /* There is no lighting effect */
    glDisable( GL_LIGHTING );
    glDisable( GL_LIGHT0 );

    gwFloat r = mesh->primary_color.r;
    gwFloat g = mesh->primary_color.g;
    gwFloat b = mesh->primary_color.b;
    gwFloat a = mesh->primary_color.a;

    glBegin( primitive );
    if (mesh->index != nullptr && mesh->index->stream != nullptr){
        for (size_t i = 0; i < mesh->index->length; i++){
            unsigned int iv = mesh->index->stream[i];
            gwFloat cr = mesh->vertex->stream[iv].position.x + 0.5f;
            gwFloat cg = mesh->vertex->stream[iv].position.y + 0.5f;
            gwFloat cb = mesh->vertex->stream[iv].position.z + 0.5f;
            glColor3f( r*a + cr*(1 - a), g*a + cg*(1 - a), b*a + cb*(1 - a) );
            glVertex3f
                ( mesh->vertex->stream[iv].position.x
                , mesh->vertex->stream[iv].position.y
                , mesh->vertex->stream[iv].position.z
                );
        }
    }
    else{
        for (size_t iv = 0; iv < mesh->vertex->length; iv++){
            gwFloat cr = mesh->vertex->stream[iv].position.x + 0.5f;
            gwFloat cg = mesh->vertex->stream[iv].position.y + 0.5f;
            gwFloat cb = mesh->vertex->stream[iv].position.z + 0.5f;
            glColor3f( r*a + cr*(1 - a), g*a + cg*(1 - a), b*a + cb*(1 - a) );
            glVertex3f
                ( mesh->vertex->stream[iv].position.x
                , mesh->vertex->stream[iv].position.y
                , mesh->vertex->stream[iv].position.z
                );
        }
    }
    glEnd();
}

int main()
{
    /* We can define the vertex and index stream 
     * or use some predefined geometries */
    gwMesh* surface = gwShape_cube();
    gwMesh* wireframe = gwWireframe_cube();
    gwMesh* points = gwMesh_create();

    surface->primary_color.a = 0;
    wireframe->primary_color.a = 255;
    points->vertex = surface->vertex;
    points->primary_color.a = 128;
    points->point_size = 16;

    /* The first step is always to create a the OpenGL context.
     * This requires third part libraries to create a window with 
     * OpenGL capabilities, like Qt or Wx. Here we use GLFW */
    int wid = glfwWindow_create_gl_context( gl_hint );
    if (wid == 0){
        /* Failed to create the context */
        return 1;
    }

    /* The hint is a request about the OpenGL version, but the version
     * is actually created depends on the drivers and the GPU device */
    int gl_version = gwWindowContext_glver( wid );

    /* This function return an object that represents the render window.
     * that exposes some fields to work with, such as the dimensions. 
     * To change the window size (in pixels), just assign the new dimension. */
    gwWindowContext* context = gw_getWindowContext( wid );
    context->screen_width = 640;
    context->screen_height = 640;
    context->background_color = gwColorNavy;

    /* In most applications there is only one window. 
     * and all operations are referred to that context.
     * A mesh created in onecontext should be rendered in that context,
     * otherwise it might create undefined results. */

    /* Once the context is created, we can load the shaders.
     * Shaders are linked to one GL context. 
     * Depending on OpenGL versions there are different shader versions */
    int sh = 0;
    if (gl_version >= 30){
        sh = gwShader_load
            ( wid, "../dev/zdev02", "shader_gl3.vsh", "shader_gl3.fsh"
            , nullptr, GW_POLYGON_FILL );
    }
    else if (gl_version >= 20){
        sh = gwShader_load
            ( wid, "../dev/zdev02", "shader_gl2.vsh", "shader_gl2.fsh"
            , nullptr, GW_POLYGON_FILL );
    }
    else{
        /* Former OpenGL1x versions do not use shaders 
         * but we use functions in OpenGL 1x to emule them */
        sh = gwShader_loadGL1x( wid, draw_rainbowfancy_gl1x );
    }

    /* The method to load the shader returns a mask number. 
     * This is used to assign shaders to the mesh and automate the rendering
     * although this feature will be used in another example. 
     * The shader as well has several properties to work with. */
    gwShader* shader = gw_getShader( wid, sh );
    if (shader != nullptr){
        shader->polygon = GW_POLYGON_FILL;
        shader->target = GW_TARGET_SCREEN;
        shader->ambient_light = gwColorWhite;
        shader->ambient_light.a = 60; /* Intensity of the ambient light */
        shader->light_direction = { 1, 1, -1 };
    }

    /* In addition, one camera is also required for rendering. 
     * The camera is used to calculate the view matrix 
     * based on eye (camera position), lookup, target, zoom, or clipping */
    gwCamera* camera = gwCamera_create();

    /* To modify the view with the camera, there are some funtions,
     * which are called by attaching the camera to the window */
    gwWindowContext_attachCamera( wid, camera );

    /* Now we can work on the callbacks of the camera:
     * on_mouse_click, on_mouse_mouse, on_keyboard, ...
     * But in this case we set a predefined ones */
    gwCamera_behaviour_cad( camera );
     
    /* Frames are squares, which are useful if we want
     * to divide the screen in different views. 
     * The also work as layers to overlap renderings.
     * If we are interested only to render directly to the screen,
     * the frames dimensions are [0,1] to cover the whole window */
    gwFrame* frame = gwFrame_create( wid );
    frame->size_in_pixels1_relative0 = 0;
    frame->x0 = 0.1f;
    frame->y0 = 0.1f;
    frame->x1 = 0.9f;
    frame->y1 = 0.9f;

    /* If the alpha component of the background color is 0, 
     * the color is not cleared and keeps whatever is rendered before */
    frame->background_color = { 0, 0, 0, 0 };

    /* The render loop */
    int status = 1;
    while (status > 0){
        /* This method is required to be called to check the window status
         * or resizes and clears the screen background ad buffers. */
        status = glfwWindow_render_begin( wid );

        /* Activate the frame, if there is one. */
        gwFrame_use( frame );

        /* Calculate the view matrix from the camera */
        gwMatrix4f view_matrix = gwCamera_calculate_view_matrix( camera );

        /* To draw a mesh three steps are required:
         * calculate the model transformation matrix,
         * set the shaders uniforms (only required for OpenGL versions > 1 ),
         * and call gwMesh_dra() */
        gwMatrix4f model_matrix = gwMesh_calculate_local_matrix( points );
        gwMesh_draw( points, shader, model_matrix, view_matrix );

		model_matrix = gwMesh_calculate_local_matrix( wireframe );
        gwMesh_draw( wireframe, shader, model_matrix, view_matrix );

		model_matrix = gwMesh_calculate_local_matrix( surface );
        gwMesh_draw( surface, shader, model_matrix, view_matrix );

        /* To update the render buffer */
        glfwWindow_render_end();

        /* Sleep a while to avoid overheating the GPU */
        gw_sleep( 42 );

        /* There is another function that better adjust to framerate */
        /* gw_adjust_framerate( 25 ); */
    }

    return 0;
}
