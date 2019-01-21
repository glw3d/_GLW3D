/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Frames can also be used for offscreen renderingd.
The image can be used as a texture for other objects.
This is used for shadows, blur, mirrors, etc...
The render to texture is very straightforward:

This is a simple example to simulates a mirror.
We need two cameras 
A better version should calculate the intersection with the mirror and the view
for each object of the scene.

*/

#include <stdio.h>

#include "context_glfw/context_glfw.h"
#include "glw/gwmath.h"
#include "glw/glw.h"
#include "glw/gwgl.h"

int gl_hint = 32;

int main()
{
    /* Create the scene objects */
    gwMesh* teapot = gwShapeLoad_obj( "../dev/zdev05/teapot.obj" );
    teapot->primary_color = gwColorRed;
    teapot->size = 25e-4f;
    teapot->rotation.x = -90;
    teapot->position.z = 0.25f;

    gwMesh* floor = gwShape_quad( 1.0f );
    floor->primary_color = gwCrayolaSepia;

    gwMesh* sight_line = gwShape_line( 3 );
    sight_line->primary_color = gwColorYellow;

    /* Create a the OpenGL context using GLFW.*/
    int wid = glfwWindow_create_gl_context( gl_hint );
    if (wid == 0){
        /* Failed to create the context */
        return 1;
    }
    int gl_version = gwWindowContext_glver( wid );
    gwWindowContext* context = gw_getWindowContext( wid );
    context->screen_width = 640;
    context->screen_height = 640;
    context->background_color = gwColorNavy;

    /* Load the default shaders */
    gw_load_default_shaders( wid, "../res/shaders" );

    gwVector3f light_direction = { 1, 1, -1 };
    gwShader* sh = gw_getShader( wid, GW_SHADER_TEXTURE );
    if (sh != nullptr){
        sh->ambient_light = gwColorWhite;
        sh->ambient_light.a = 60; /* Intensity of the ambient light */
        sh->light_direction = light_direction;
    }

    /* Create a camera for the main scene */
    gwCamera* camera_scene = gwCamera_create();

    /* Register the camera, so it can detect mouse inputs */
    gwWindowContext_attachCamera( wid, camera_scene );

    /* Set the camera behaviour */
    gwCamera_behaviour_cad( camera_scene );

    camera_scene->rotation_angles.y = 135;
    camera_scene->rotation_angles.x = 45;

    gwCamera* camera_mirror = gwCamera_create();
    gwMesh* reflection_line = gwShape_line( 2 );

    /* Create a target for offscreen rendering. */
    /* For performance, it is recommended that dimensions are power of 2 */
    gwRenderTarget* target = gwRenderTarget_create( wid, 256, 256 );
    target->background_color = gwColorWhite;

    /* Use the texture in the offscreen target as any conventional texture */
    floor->texture0 = target->color_texture;

    /* Notice that we are not using frames, as they are only required 
     * to restrict the render in a region or for convenience to attach cameras
     * and draw everything in one call */
    int status = 1; 
    while (status > 0){
        /* Check the window status */
        status = glfwWindow_render_begin( wid );

        /* Clear the screen. */
        gwWindowContext_clear( context );

        ///* Draw the scene */
        gwMatrix4f view_matrix = gwCamera_calculate_view_matrix( camera_scene );
        
        gwMatrix4f teapot_matrix = gwMesh_calculate_local_matrix( teapot );
		gwMesh_draw( teapot, sh, teapot_matrix, view_matrix );
                
        /* Calculate de mirror */
        gwVector3f mirror_pos = { 0, 0, 0 };
        gwVector3f mirror_normal = { 0, 0, 1 };
        gwVector3f camera_position = { 0, 0, 1 };

        gwMatrix4f matrix_inverted = gwMatrix4f_invert( view_matrix );
        camera_position = gwVector3f_mul( &matrix_inverted, &camera_position );
        camera_position = gwVector3f_norm( &camera_position );
        gwVector3f mirror_lookat = gwCamera_reflection
            ( &mirror_normal, &camera_position, &mirror_pos );

        camera_mirror->target = mirror_lookat;
        camera_mirror->eye = mirror_pos;
        camera_mirror->type = GW_CAMERA_VIEW_LOOKAT + GW_CAMERA_VIEW_FLAG_NO_SCREEN_RATIO;
        camera_mirror->lookup = { 0, 1, 0 }; 

        /* Draw in offscreen buffer */
        gwRenderTarget_use( target ); 

        /* Draw the scene */
        view_matrix = gwCamera_calculate_view_matrix( camera_mirror ); 
		teapot_matrix = gwMesh_calculate_local_matrix( teapot );
		gwMesh_draw( teapot, sh, teapot_matrix, view_matrix );

        /* Return to the default target (the screen) */
        gwRenderTarget_use( nullptr ); 
        view_matrix = gwCamera_calculate_view_matrix( camera_scene ); 
		gwMatrix4f floor_matrix = gwMesh_calculate_local_matrix( floor );
		gwMesh_draw( floor, sh, floor_matrix, view_matrix );

        /* Update the render buffer */
        glfwWindow_render_end();

        /* Animate the scene */
        teapot->rotation.y += 0.2f;
    }

    return 0;
}
