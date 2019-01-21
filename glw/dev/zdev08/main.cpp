/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Picking is a technique to identify the object under the mouse coordinates.
*/

#include <stdio.h>
#include <time.h>

#include "glw/gwgl.h"

#include "context_glfw/context_glfw.h"
#include "glw/glw.h"
#include "font/font.h"


void func_click( gwMesh* mesh, const int vertex_id, const int button )
{
    mesh->primary_color = gwColorYellow;
}
void func_release( gwMesh* mesh, const int vertex_id, const int button )
{
    mesh->primary_color = gwColorGreen;
	printf(mesh->label);
	printf("\n");
}
void func_light( gwMesh* mesh, const int vertex_id, const int button )
{
    mesh->primary_color = gwColorGreen;
}
void func_shade( gwMesh* mesh, const int vertex_id, const int button )
{
    mesh->primary_color = gwColorWhite;
}

int main()
{
    const int gl_hint = 31;

    /* Create a the OpenGL context using GLFW.*/
    int wid = glfwWindow_open();
    if (wid == 0){
        /* Failed to create the context */
        return 1;
    }
    int gl_version = gwWindowContext_glver( wid );
    gwWindowContext* context = gw_getWindowContext( wid );
    context->screen_width = 800;
    context->screen_height = 640;

    /* Load predefined shaders */
    gw_load_default_shaders( wid, "../res/shaders" );

    /* Create the scene */
    gwFrame* frame_scene = gwFrame_create( wid );
    frame_scene->background_color = gwColorGray;

    gwCamera* camera_scene = gwCamera_create();
    gwWindowContext_attachCamera( wid, camera_scene );
    gwCamera_behaviour_cad( camera_scene );
    gwFrame_addCamera( frame_scene, camera_scene );

    /* Load a font */
    int ftid_regular = font_load( "../res/fonts/oldstandard/OldStandard-Regular.ttf" );

    int ibutton = 1;
    for (int i = -1; i < 2; i++){
        for (int j = -1; j < 2; j++){
            gwMesh* mesh = gwShape_cube();

            /* THIS IS THE TRICK.
            * Besides the shader for rendering, add the shader for picking */
            mesh->shaders = GW_SHADER_TEXTURE + GW_SHADER_PICKING;

            /* Now the mesh will respond to mouse events */
            mesh->f_on_mouse_click = func_click;
            mesh->f_on_mouse_release = func_release;
            mesh->f_on_mouse_enter = func_light;
            mesh->f_on_mouse_leave = func_shade;

			sprintf(mesh->label, "[%i]", ibutton);

            mesh->primary_color = gwColorWhite;
            mesh->position.x = (float)j / 4;
            mesh->position.y = (float)i / 4;
            mesh->size = 0.2f;

            /* Create a texture with a number on each block */
            char number[] = "  ";
            sprintf( number, "%i", ibutton );
            mesh->texture0 = gwText( ftid_regular, number, 128, gwColorBlack );

            /* Invert the texture orientation and add a bit offset to center */
            gwMesh_textureSlab( mesh, 0.1f, -0.1f, 1.0f, -1.0f );

            /* Add the mesh to the scene */
            gwCamera_addMesh( camera_scene, mesh );

            ibutton++;
        }
    }

    int status = 1;
    while (status > 0){

        /* Start the rendering */
        status = glfwWindow_render_begin( wid );

        gwFrame_use( frame_scene );
        gwCamera_render( camera_scene );

        /* Update the render buffer */
        glfwWindow_render_end();

        gw_adjust_framerate( 25 );
    }
}

