/**
Author: Mario J. Martin <dominonurbs$gmail.com>

In some applications, we have to launch the render in another thread,
so, we can watch the visualization while the main application is still running.


*/

#include <stdio.h>

#include "context_glfw/context_glfw.h"
#include "glw/gwmath.h"
#include "glw/glw.h"

int gl_hint = 32;

/* This is the render loop */
void render_loop()
{
    int status = 1;
    while (status > 0){
        glfw_render();
    }
}

int main()
{
    /* Creates a window, but this is not created here, 
     * but request a new instance that is opened in a different thread.*/
    int wid = glfwWindow_open();

    gwWindowContext* context = gw_getWindowContext( wid );
    context->screen_width = 640;
    context->screen_height = 640;
    context->background_color = gwColorNavy;
    context->gl_version_hint = 11;  

    /* The frame is used to especify the render area */
    gwFrame* frame = gwFrame_create( wid );
    frame->size_in_pixels1_relative0 = 0;
    frame->x0 = 0.1f;
    frame->y0 = 0.1f;
    frame->x1 = 0.9f;
    frame->y1 = 0.9f;
    frame->background_color = gwColorGray;

    /* The definition of the scene remains the same */
    gwMesh* surface = gwShape_cube();
    gwMesh* wireframe = gwWireframe_cube();
    gwMesh* points = gwMesh_create();

    surface->primary_color.a = 0;
    wireframe->primary_color.a = 255;
    points->vertex = surface->vertex;
    points->primary_color.a = 128;
    points->point_size = 16;

    /* Camera */
    gwCamera* camera = gwCamera_create();
    gwWindowContext_attachCamera( wid, camera );
    gwCamera_behaviour_cad( camera );

	/* To attach cameras to frames is only necesary if we call glfw_render() 
	 * to do all the render. 
	 * If we call gwCamera_render() use gwFrame_use() before the call */
    gwFrame_addCamera( frame, camera );

    /* Then, add the mesh objects to the camera */
    gwCamera_addMesh( camera, points );
    gwCamera_addMesh( camera, wireframe );
    gwCamera_addMesh( camera, surface );

    /* Because the context is not yet created, the shader is not loaded here,
     * but at the next render loop, instead */
    int sh = gwShader_load
        ( wid, "../dev/zdev03", "shader_gl3.vsh", "shader_gl3.fsh"
        , nullptr, GW_POLYGON_FILL );

    gwShader* shader = gw_getShader( wid, sh );
    if (shader != nullptr){
        shader->polygon = GW_POLYGON_FILL;
        shader->target = GW_TARGET_SCREEN;
        shader->ambient_light = gwColorWhite;
        shader->ambient_light.a = 60; /* Intensity of the ambient light */
        shader->light_direction = { 1, 1, -1 };
    }

    /* Assign the shader to the mesh
    * Several shaders can be added simultaneously: shader1 + shader2 + ... */
    surface->shaders = sh;
    wireframe->shaders = sh;
    points->shaders = sh;

    /* The render_loop looks like:
            int status = 1;
            while (status > 0){
                glfw_render();
            }

     * Then call: 
            gw_launch_thread( [](){ while (glfw_render() > 0); } ); */
     
	/* In asyncronous mode, the render runs in a different thread
	* and we do not have direct control of the render loop. */
    glfw_render_thread();

    /* Then we halt the main thread execution or the program will exit */
    printf( "Running... PRESS ENTER\n" );
    getchar();

    return 0;
}
