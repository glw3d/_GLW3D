/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Meshes can be set in a hyerarchy structure. 
Take for example the solar system. The Sun is the parent object, 
while the planets are the childs, which rotate around the parent. 
Then the moons are childs, which rotate around the planets.

*/

#include <stdio.h>
#include <math.h>

#include "context_glfw/context_glfw.h"
#include "glw/gwmath.h"
#include "glw/glw.h"

int main()
{
    const int gl_hint = 31;

    /* Create a the OpenGL context using GLFW.*/
    int wid = glfwWindow_create_gl_context( gl_hint );
    if (wid == 0){
        /* Failed to create the context */
        return 1;
    }
    int gl_version = gwWindowContext_glver( wid );
    gwWindowContext* context = gw_getWindowContext( wid );
    context->screen_width = 800;
    context->screen_height = 640;
    context->background_color = gwColorWhite;

    /* Load the shader */
    int shid = gwShader_load
        ( wid, "../dev/zdev06", "spot_light.vsh", "spot_light.fsh"
        , nullptr, GW_POLYGON_FILL );
    gwShader* shader = gw_getShader( wid, shid );
    shader->ambient_light = gwColorBlack;

    /* Create a frame for the background */
    gwFrame* frame_background = gwFrame_create( wid );
    frame_background->background_color = gwColorNavy;

    /* Load the texture for the background */
    gwTexture* tex_background = gwTexture_loadBMP( "../dev/zdev06/night_sky.bmp" );

    /* Load a quad to render the background */
    gwMesh* mesh_background = gwShape_quad( 1.0f );
    mesh_background->primary_color = gwCrayolaSepia;
    mesh_background->size = 2;
    mesh_background->texture0 = tex_background;

    /* Create the scene objects */
    gwMesh* mesh_sun = gwShape_sphere( 36 );
    gwTexture* tex_sun = gwTexture_loadBMP( "../dev/zdev06/sun.bmp" );
    mesh_sun->texture0 = tex_sun;
    mesh_sun->size = 1.2f;
    mesh_sun->self_ilumination = 1;
    mesh_sun->shaders = shid;

    gwMesh* mesh_earth = gwShape_sphere( 36 );
    gwTexture* tex_earth = gwTexture_loadBMP( "../dev/zdev06/earth.bmp" );
    mesh_earth->texture0 = tex_earth;
    mesh_earth->shaders = shid;

    gwMesh* mesh_moon = gwShape_sphere( 36 );
    gwTexture* tex_moon = gwTexture_loadBMP( "../dev/zdev06/moon.bmp" );
    mesh_moon->texture0 = tex_moon;
    mesh_moon->shaders = shid;

    gwMesh* mesh_mars = gwShape_sphere( 36 );
    gwTexture* tex_mars = gwTexture_loadBMP( "../dev/zdev06/mars.bmp" );
    mesh_mars->texture0 = tex_mars;
    mesh_mars->shaders = shid;

    gwMesh* mesh_deimos = gwShape_cloudpoint( 1 );
    mesh_deimos->primary_color = { 255, 128, 128 };
    mesh_deimos->self_ilumination = 1;
    mesh_deimos->shaders = shid;
    mesh_deimos->point_size = 2;

    gwMesh* mesh_fobos = gwShape_cloudpoint( 1 );
    mesh_fobos->primary_color = gwColorWhite;
    mesh_fobos->self_ilumination = 1;
    mesh_fobos->shaders = shid;
    mesh_fobos->point_size = 2;

    gwMesh* mesh_teapot = gwShapeLoad_obj( "../dev/zdev06/teapot.obj" );
    mesh_teapot->shaders = shid;
    mesh_teapot->primary_color = gwColorGreen;
    mesh_teapot->texture_blend = 1;
    mesh_teapot->size = 0.005f;
    mesh_teapot->rotation.x = 60.0f;
    mesh_teapot->rotation.y = 30.0f;

    /* Create the hierarchy */
	mesh_earth->parent = mesh_sun;
	mesh_moon->parent = mesh_earth;

	mesh_mars->parent = mesh_sun;
	mesh_deimos->parent = mesh_mars;
	mesh_fobos->parent = mesh_mars;

	mesh_teapot->parent = mesh_fobos;

    /* Create a frame for the scene */
    gwFrame* frame_scene = gwFrame_create( wid );
    frame_scene->background_color.a = 0;  /* Makes the frame transparent */

    /* Create a camera for the scene */
    gwCamera* camera_scene = gwCamera_create();
    gwFrame_addCamera( frame_scene, camera_scene );
    camera_scene->zoom = 0.1f;
    camera_scene->rotation_angles.z = 90;

    /* Register the camera, so it can detect mouse inputs */
    gwWindowContext_attachCamera( wid, camera_scene );

    /* Set the camera behaviour */
    gwCamera_behaviour_cad( camera_scene );
	gwCamera_addMesh( camera_scene, mesh_sun );
	gwCamera_addMesh( camera_scene, mesh_earth );
	gwCamera_addMesh( camera_scene, mesh_mars );
	gwCamera_addMesh( camera_scene, mesh_moon );
	gwCamera_addMesh( camera_scene, mesh_deimos );
	gwCamera_addMesh( camera_scene, mesh_fobos );
	gwCamera_addMesh( camera_scene, mesh_teapot );

    /* The Earth distance to the Sun is approx. 150 million kilometres */
    const double earth_dist = 5;

    /* The render loop */
    int t0 = 0;
    int status = glfwWindow_render_begin( wid );
    while (status > 0){
        /* Animate the scene */
        t0 += 1;

        /* Earth is a child of the Sun and its coordinates are relative to it */
        double earth_rotation = double( t0 );
        double earth_orbit = double( t0 ) / 365.0;
        mesh_earth->position.x = (float)(earth_dist * cos( earth_orbit / 57.3 ));
        mesh_earth->position.y = (float)(earth_dist * sin( earth_orbit / 57.3 ));
        mesh_earth->position.z = 0.0f;
        mesh_earth->rotation.y = 23.5f;
        mesh_earth->rotation.z = (float)earth_rotation;
        mesh_earth->size = 0.25f;
        mesh_earth->self_ilumination = 0.1f;

        /* As the Moon is a child, its position is relative to Earth 
         * The childs propagate parent position, rotation, and size.*/

        /* The Moon orbit is 27.3 (Earth)days */
        double lunar_orbit = double( t0 ) / 27.322;
        /* The distance to the Moon is 60 times the Earth radius */
        double moon_dist = 60.34/20;
        mesh_moon->position.x = (float)(moon_dist * cos( lunar_orbit / 57.3 ));
        mesh_moon->position.y = (float)(moon_dist * sin( lunar_orbit / 57.3 ));
        /* Undo the Earth inclination. The Moon orbit is also inclined 5º */
        mesh_moon->position.z = (float)(1 * sin( 23.5f )*cos( (earth_rotation - lunar_orbit) / 57.3 ));
        mesh_moon->rotation.z = (float)lunar_orbit;
        mesh_moon->size = 0.27f;
        mesh_moon->self_ilumination = 0.1f;

        /* Siblings are at the same level of the hierarchy */
        double mars_rotation = double( t0 ) * (24.0 / 24.6);
        double mars_orbit = double( t0 ) / (687.0f*57.3f);
        double mars_dist = 1.6 * earth_dist;
        mesh_mars->position.x = (float)(mars_dist * cos( mars_orbit / 57.3 ));
        mesh_mars->position.y = (float)(mars_dist * sin( mars_orbit / 57.3 ));
        mesh_mars->rotation.z = (float)mars_rotation;
        mesh_mars->size = 0.532f * mesh_earth->size;
        mesh_mars->self_ilumination = 0.1f;

        /* Phobos orbit is 0.31 Mars days */
        double phobos_orbit = double( t0 ) / 0.31;
        /* Phobos orbit is 2.77 Mars radius */
        double fobos_dist = 2.77;
        mesh_fobos->position.x = float( fobos_dist * cos( -phobos_orbit / 57.3 ) );
        mesh_fobos->position.y = float( fobos_dist * sin( -phobos_orbit / 57.3 ) );

        /* Deimos orbit is 1.23 Mars days */
        double deimos_orbit = double( t0 ) / 1.23;
        double deimos_dist = 6.91;
        mesh_deimos->position.x = float( deimos_dist * cos( -deimos_orbit / 57.3 ) );
        mesh_deimos->position.y = float( deimos_dist * sin( -deimos_orbit / 57.3 ) );

        /* A teapot in the Mars orbit */
        double teapot_orbit = double( t0 ) / 2.0;
        double teapot_dist = 9.6;
        mesh_teapot->position.x = float( teapot_dist * cos( -teapot_orbit / 57.3 ) );
        mesh_teapot->position.y = float( teapot_dist * sin( -teapot_orbit / 57.3 ) );
        mesh_teapot->rotation.z = float(t0 / 4);

        /* Check the window status */
        status = glfwWindow_render_begin( wid );

        /* Draw in the background frame */
        gwFrame_use( frame_background );

        /* The camera for the background is just the identity */
        gwMatrix4f background_view_matrix = gwMatrix4f_identity();
        gwMatrix4f background_model_matrix = gwMesh_calculate_local_matrix( mesh_background );
        gwMesh_draw( mesh_background, shader, background_model_matrix, background_view_matrix );

        /* Draw in the scene frame */
        gwFrame_use( frame_scene );

        /* Draw the scene */
		gwCamera_render( camera_scene );

        /* To update the render buffer */
        glfwWindow_render_end();

        /* Sleeps to adjust the framerate */
        gw_adjust_framerate( 25 );

        status = glfwWindow_render_begin( wid );
    }

    return 0;
}
