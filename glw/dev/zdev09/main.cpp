/**
Author: Mario J. Martin <dominonurbs$gmail.com>

By default the rendering is directly done on the screen.
But we can draw the scene on a offscreen frame buffer 
with the same dimensions as the screeen and use it for post-processing.
The offscreen is rendered as a texture on a quad that fills the screen.

In this example, we simply draw the depth buffer.
This can be used for effects, like blur the objects based on the focal distance.

There are two options:
   * Draw the entire scene in an offscreen buffer.
   * Draw the scene, copy the scene in the buffer, clear and draw a quad.

In both cases, the sffect is achieved by drawing a quad 
with the offscreen as a texture.
*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "glw/gwgl.h"

#include "context_glfw/context_glfw.h"
#include "glw/glw.h"
#include "glw/gwmath.h"
#include "glw/gwthreads.h"

int main()
{
    /* Create a the OpenGL context using GLFW.*/
    int wid = glfwWindow_open();
    if (wid == 0){
        /* Failed to create the context */
        return 1;
    }

    int gl_version = gwWindowContext_glver( wid );
    gwWindowContext* context = gw_getWindowContext( wid );
    context->screen_width = 800;
    context->screen_height = 600;

    /* Load the default shaders */
    gw_load_default_shaders( wid, "../res/shaders" );

    /* Create the scene */
    gwMesh* teapot1 = gwShapeLoad_obj( "../dev/zdev09/teapot.obj" );
    teapot1->primary_color = gwColorSilver;
    teapot1->size = 0.005f;
    teapot1->shaders = GW_SHADER_SOLID + GW_SHADER_PICKING;
    teapot1->position.x = 0.0f;
    teapot1->position.y = 0.0f;
    teapot1->position.z = 5.0f;

    /* Create a lot of more teapots */
    gwMesh* teapot2 = gwMesh_create();
    *teapot2 = *teapot1; /* Reuse the vertex and index streams */ 
    teapot2->size = 0.005f;
    teapot2->primary_color = gwColorRed;
    teapot2->position.x = 0.5f;
    teapot2->position.y = 0.5f;
    teapot2->position.z = 1.0f;
    teapot2->shaders = GW_SHADER_SOLID + GW_SHADER_PICKING;

    gwMesh* teapot3 = gwMesh_create();
    *teapot3 = *teapot1; /* Reuse the vertex and index streams */ 
    teapot3->size = 0.005f;
    teapot3->primary_color = gwColorGreen;
    teapot3->position.x = 0.5f;
    teapot3->position.y = -0.5f;
    teapot3->position.z = 6.0f;
    teapot3->shaders = GW_SHADER_SOLID + GW_SHADER_PICKING;

    gwMesh* teapot4 = gwMesh_create();
    *teapot4 = *teapot1; /* Reuse the vertex and index streams */
    teapot4->size = 0.005f;
    teapot4->primary_color = gwColorBlue;
    teapot4->position.x = -0.5f;
    teapot4->position.y = -0.5f;
    teapot4->position.z = 8.0f;
    teapot4->shaders = GW_SHADER_SOLID + GW_SHADER_PICKING;

    gwMesh* teapot5 = gwMesh_create();
    *teapot5 = *teapot1; /* Reuse the vertex and index streams */
    teapot5->size = 0.005f;
    teapot5->primary_color = gwColorYellow;
    teapot5->position.x = -0.5f;
    teapot5->position.y = 0.5f;
    teapot5->position.z = 3.0f;
    teapot5->shaders = GW_SHADER_SOLID + GW_SHADER_PICKING;

    /* Create the camera */
    gwCamera* camera = gwCamera_create();
    gwWindowContext_attachCamera( wid, camera );
    gwCamera_behaviour_cad( camera );
    camera->type = GW_CAMERA_VIEW_CAD + GW_CAMERA_VIEW_FLAG_PERSPECTIVE;
    camera->farClip = 1.0f;
    camera->nearClip = 0.01f;

    gwCamera_addMesh( camera, teapot1 );
    gwCamera_addMesh( camera, teapot2 );
    gwCamera_addMesh( camera, teapot3 );
    gwCamera_addMesh( camera, teapot4 );
    gwCamera_addMesh( camera, teapot5 );

    /* Load the shader for post-processing */
    int sh_postproc_id = gwShader_load( wid
        , "../dev/zdev09", "depth_of_field.vsh", "depth_of_field.fsh", nullptr
        , GW_POLYGON_FILL );
    gwShader* sh_postproc = gw_getShader( wid, sh_postproc_id );

    /* Create a render target. We assign dimensions {0,0}
    * so it will updated with the dimensions of the screen */
    gwRenderTarget* target = gwRenderTarget_create( wid, 0, 0 );

    /* Create a quad to render the postprocessing */
    gwMesh* mesh_quad = gwShape_quad01();

    /* Assign the frame buffer as a texture of the quad */
    mesh_quad->texture0 = target->color_texture;

    /* The render loop */
    int status = 1;
    while (status > 0){

        /* Check the window status */
        status = glfwWindow_render_begin( wid );
        if (status == 0){
            break;
        }

        /* Draw the scene in a offscreen buffer */
        gwCamera_render( camera );

        /* Switch to the default target (the screen) */
        gwWindowContext_blitFramebuffer( target );

        /* Instead of clear, it is more efficient to disable depth test */
        glDisable( GL_DEPTH_TEST );
        //gwWindowContext_clear( context );

        /* Render the quad using the post-processing shader */
        gwMatrix4f model_matrix = gwMatrix4f_identity();
        gwMesh_draw( mesh_quad, sh_postproc, model_matrix, model_matrix );

        glEnable( GL_DEPTH_TEST );

        /* Update the render buffer */
        glfwWindow_render_end();

        /* Sleeps to adjust the framerate */
        gw_adjust_framerate( 25 );

        /* The other option is
        gwRenderTarget_use( target );
        gwCamera_draw( camera );
        gwRenderTarget_use( nullptr );
        draw_the_quad();
        */
    }

    /* Run the visualization in another thread */
    //glfw_render_thread();

    printf( "Press ENTER to exit\n" );
    getchar();
}
