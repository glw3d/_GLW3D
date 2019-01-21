/**
Author: Mario J. Martin <dominonurbs$gmail.com>

There are some default shaders for the most common rendering

*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "context_glfw/context_glfw.h"
#include "glw/gwmath.h"
#include "glw/glw.h"

int gl_hint = 32;

int wid;
gwVector4f control_points[] =
{ -1.0, 0.0, 0.0, 1.0
, -0.5, 0.0, 0.0, 1.0
, 0.0, 1.0, 0.0, 1.0
, 0.5, 0.0, 0.0, 1.0
, 1.0, 0.0, 0.0, 1.0 };

gwMesh* mesh_points = nullptr;
gwMesh* mesh_lines_0 = nullptr;
gwMesh* mesh_lines[32] = { nullptr };

gwMesh* mesh_spring = nullptr;
gwMesh* mesh_autum = nullptr;
gwMesh* mesh_winter = nullptr;
gwMesh* mesh_summer = nullptr;

/* Create one scene for solid + lighting */
void first_scene()
{
    /* Create a frame */
    gwFrame* frame = gwFrame_create( wid );
    frame->size_in_pixels1_relative0 = 0;
    frame->x0 = 0;
    frame->y0 = 0.5;
    frame->x1 = 0.5;
    frame->y1 = 1.0;
    frame->background_color = gwColorAqua;

    /* Create a camera for the scene */
    gwCamera* camera = gwCamera_create();

    /* The camera is attached to the screen to be controlled by mouse/keyboard. 
     * In addition, it is linked to a frame. 
     * So it is only sensitive within the boundaries of the frame */
    camera->boundary_frame = frame;
    gwWindowContext_attachCamera( wid, camera );
    gwCamera_behaviour_cad( camera );
    gwFrame_addCamera( frame, camera );

    /* There is the posibility to create a geometry from scratch.
     * Here we create simply square */
    gwMesh* mesh_quad = gwMesh_create();
    mesh_quad->vertex = gwVertexStream_create( 4 );

    mesh_quad->vertex->stream[0].position = { 1, 1, 0 };
    mesh_quad->vertex->stream[1].position = { 1, -1, 0 };
    mesh_quad->vertex->stream[2].position = { -1, 1, 0 };
    mesh_quad->vertex->stream[3].position = { -1, -1, 0 };

    mesh_quad->vertex->stream[0].normal = { 0, 0, 1 };
    mesh_quad->vertex->stream[1].normal = { 0, 0, 1 };
    mesh_quad->vertex->stream[2].normal = { 0, 0, 1 };
    mesh_quad->vertex->stream[3].normal = { 0, 0, 1 };

    gwInt index_quad[6] = { 0, 1, 2, 1, 2, 3 };
    mesh_quad->index = gwIndexStream_create( 6, GW_INDEX_TRIANGLES );
    /* The index stream indicates the conectivities; it is not always required */
    memcpy( mesh_quad->index->stream, index_quad, sizeof( int ) * 6 );

    mesh_quad->size = 0.20f;
    mesh_quad->position.x = 0.0f;
    mesh_quad->position.y = 0.5f;
    mesh_quad->position.z = -0.5f;
    mesh_quad->primary_color = gwColorGray;
    mesh_quad->shaders = GW_SHADER_SOLID + GW_SHADER_WIREFRAME;
    mesh_quad->culling = 0;
    gwCamera_addMesh( camera, mesh_quad );

    /* Load from a .stl file (very heavy file) */ 
    //gwMesh* mesh_bottle = gwShapeLoad_stl( "../dev/zdev04/bottle.stl" );
    //mesh_bottle->primary_color = gwColorGreen;
    //mesh_bottle->size = 5e-4f;  
    //mesh_bottle->position.x = 0.5f;
    //mesh_bottle->position.y = 0.0f;
    //mesh_bottle->position.z = -0.5f;
    //mesh_bottle->shaders = GW_SHADER_SOLID;
    //gwCamera_addMesh( camera, mesh_bottle );

    /* Load from a .obj file */
    gwMesh* mesh_teapot = gwShapeLoad_obj( "../dev/zdev04/teapot.obj" );
    mesh_teapot->primary_color = gwColorRed;
    mesh_teapot->size = 5e-3f;  
    mesh_teapot->position.x = 0.5f;
    mesh_teapot->position.y = 0.5f;
    mesh_teapot->position.z = 0.5f;
    mesh_teapot->shaders = GW_SHADER_SOLID;
    gwCamera_addMesh( camera, mesh_teapot );

    /* There are some basic predefined geometries */
    gwMesh* mesh_cube = gwShape_cube();
    mesh_cube->primary_color = gwColorYellow;
    mesh_cube->size = 0.4f;
    mesh_cube->position.x = -0.5f;
    mesh_cube->position.y = 0.5f;
    mesh_cube->position.z = 0.5f;
    mesh_cube->shaders = GW_SHADER_SOLID + GW_SHADER_WIREFRAME;
    gwCamera_addMesh( camera, mesh_cube );

    gwMesh* mesh_icosahedron = gwShape_icosahedron();
    mesh_icosahedron->primary_color = gwColorMagenta;
    mesh_icosahedron->size = 0.25f;
    mesh_icosahedron->position.x = 0.5f;
    mesh_icosahedron->position.y = -0.5f;
    mesh_icosahedron->position.z = 0.5f;
    mesh_icosahedron->shaders = GW_SHADER_SOLID + GW_SHADER_WIREFRAME;
    gwCamera_addMesh( camera, mesh_icosahedron );

    gwMesh* mesh_sphere = gwShape_sphere( 16 );
    mesh_sphere->primary_color = gwColorBlue;
    mesh_sphere->size = 0.20f;
    mesh_sphere->position.x = -0.5f;
    mesh_sphere->position.y = -0.5f;
    mesh_sphere->position.z = 0.5f;
    mesh_sphere->shaders = GW_SHADER_SOLID;
    gwCamera_addMesh( camera, mesh_sphere );

    gwMesh* mesh_cilinder = gwShape_cilinder( 16, 1, 0 );
    mesh_cilinder->primary_color = gwColorCyan;
    mesh_cilinder->scale.z = 5.0f;
    mesh_cilinder->size = 0.15f;

    mesh_cilinder->position.x = -0.5f;
    mesh_cilinder->position.y = -0.0f;
    mesh_cilinder->position.z = -0.5f;
    mesh_cilinder->shaders = GW_SHADER_SOLID;
    gwCamera_addMesh( camera, mesh_cilinder );

    gwMesh* mesh_cone = gwShape_cilinder( 16, 0, 0 );
    mesh_cone->primary_color = gwColorOrange;
    mesh_cone->scale.z = 2.0f;
    mesh_cone->size = 0.2f;

    mesh_cone->position.x = 0.0f;
    mesh_cone->position.y = -0.5f;
    mesh_cone->position.z = -0.5f;
    mesh_cone->shaders = GW_SHADER_SOLID;
    gwCamera_addMesh( camera, mesh_cone );
}

/* Create second scene for solid + texture */
void second_scene()
{
    gwFrame* frame_scene = gwFrame_create( wid );
    frame_scene->size_in_pixels1_relative0 = 0;
    frame_scene->x0 = 0.5;
    frame_scene->y0 = 0.5;
    frame_scene->x1 = 1.0;
    frame_scene->y1 = 1.0;

    /* Set the alpha channel to 0, to avoid cleaning */
    frame_scene->background_color = { 0, 0, 0, 0 };

    /* We assign a high preference To make sure that is rendered after */
    frame_scene->preference = 1;

    gwCamera* camera_scene = gwCamera_create();
    camera_scene->boundary_frame = frame_scene;
    gwWindowContext_attachCamera( wid, camera_scene );
    gwCamera_behaviour_cad( camera_scene );
    gwFrame_addCamera( frame_scene, camera_scene );

    gwMesh* mesh_cube = gwShape_cube();
    mesh_cube->shaders = GW_SHADER_TEXTURE;
    gwCamera_addMesh( camera_scene, mesh_cube );

    gwTexture* tex_dice = gwTexture_loadBMP( "../dev/zdev04/dice.bmp" );
    mesh_cube->texture0 = tex_dice;

    /* Adjust the texture coordinates */
    float r = 64.0f / 512;
    mesh_cube->vertex->stream[0].texCoord.u *= r;
    mesh_cube->vertex->stream[1].texCoord.u *= r;
    mesh_cube->vertex->stream[2].texCoord.u *= r;
    mesh_cube->vertex->stream[3].texCoord.u *= r;

    mesh_cube->vertex->stream[4].texCoord.u *= r;
    mesh_cube->vertex->stream[5].texCoord.u *= r;
    mesh_cube->vertex->stream[6].texCoord.u *= r;
    mesh_cube->vertex->stream[7].texCoord.u *= r;
    mesh_cube->vertex->stream[4].texCoord.u += r;
    mesh_cube->vertex->stream[5].texCoord.u += r;
    mesh_cube->vertex->stream[6].texCoord.u += r;
    mesh_cube->vertex->stream[7].texCoord.u += r;

    mesh_cube->vertex->stream[8].texCoord.u *= r;
    mesh_cube->vertex->stream[9].texCoord.u *= r;
    mesh_cube->vertex->stream[10].texCoord.u *= r;
    mesh_cube->vertex->stream[11].texCoord.u *= r;
    mesh_cube->vertex->stream[8].texCoord.u += 2 * r;
    mesh_cube->vertex->stream[9].texCoord.u += 2 * r;
    mesh_cube->vertex->stream[10].texCoord.u += 2 * r;
    mesh_cube->vertex->stream[11].texCoord.u += 2 * r;

    mesh_cube->vertex->stream[12].texCoord.u *= r;
    mesh_cube->vertex->stream[13].texCoord.u *= r;
    mesh_cube->vertex->stream[14].texCoord.u *= r;
    mesh_cube->vertex->stream[15].texCoord.u *= r;
    mesh_cube->vertex->stream[12].texCoord.u += 3 * r;
    mesh_cube->vertex->stream[13].texCoord.u += 3 * r;
    mesh_cube->vertex->stream[14].texCoord.u += 3 * r;
    mesh_cube->vertex->stream[15].texCoord.u += 3 * r;

    mesh_cube->vertex->stream[16].texCoord.u *= r;
    mesh_cube->vertex->stream[17].texCoord.u *= r;
    mesh_cube->vertex->stream[18].texCoord.u *= r;
    mesh_cube->vertex->stream[19].texCoord.u *= r;
    mesh_cube->vertex->stream[16].texCoord.u += 4 * r;
    mesh_cube->vertex->stream[17].texCoord.u += 4 * r;
    mesh_cube->vertex->stream[18].texCoord.u += 4 * r;
    mesh_cube->vertex->stream[19].texCoord.u += 4 * r;

    mesh_cube->vertex->stream[20].texCoord.u *= r;
    mesh_cube->vertex->stream[21].texCoord.u *= r;
    mesh_cube->vertex->stream[22].texCoord.u *= r;
    mesh_cube->vertex->stream[23].texCoord.u *= r;
    mesh_cube->vertex->stream[20].texCoord.u += 5 * r;
    mesh_cube->vertex->stream[21].texCoord.u += 5 * r;
    mesh_cube->vertex->stream[22].texCoord.u += 5 * r;
    mesh_cube->vertex->stream[23].texCoord.u += 5 * r;
}

/* Create third scene for lines and dots */
void third_scene()
{
    /* Two frames are overlapped. The first one renders a quad with a texture
    * to create a background image; then, the scene */
    gwFrame* frame = gwFrame_create( wid );
    frame->size_in_pixels1_relative0 = 0;
    frame->x0 = 0.0;
    frame->y0 = 0.0;
    frame->x1 = 0.5;
    frame->y1 = 0.5;
    frame->background_color = gwColorBlack;

    mesh_points = gwShape_cloudpoint( 5 );
    mesh_points->primary_color = gwColorWhite;
    mesh_points->shaders = GW_SHADER_DOTCLOUD;
    mesh_points->point_size = 8;

    mesh_lines_0 = gwShape_line( 128 );
    mesh_lines_0->primary_color = gwColorWhite;
    mesh_lines_0->shaders = GW_SHADER_WIREFRAME;

    gwColor colors[8] =
    { gwColorRed
    , gwColorOrange
    , gwColorYellow
    , gwColorGreen
    , gwColorCyan
    , gwColorBlue
    , gwColorMagenta
    , gwColorGray
    };

    for (int imesh = 0; imesh < 32; imesh++){
        mesh_lines[imesh] = gwShape_line( 128 );
        int c = int( (float(imesh) / 31 ) * 256 );
        mesh_lines[imesh]->primary_color = colors[imesh % 8];
        mesh_lines[imesh]->shaders = GW_SHADER_WIREFRAME;
    }

    gwCamera* camera = gwCamera_create();
    camera->type = GW_CAMERA_VIEW_SCREEN;
    gwFrame_addCamera( frame, camera );
    gwCamera_addMesh( camera, mesh_points );
    gwCamera_addMesh( camera, mesh_lines_0 );
    for (int imesh = 0; imesh < 32; imesh++){
        gwCamera_addMesh( camera, mesh_lines[imesh] );
    }
}

void fourth_scene()
{
    /* Two frames are overlapped. The first one renders a quad with a texture
    * to create a background image. It is renderend in a separated frame,
    * so the scene overlaps. We can also use the same frame and use the depth */
    gwFrame* frame_background = gwFrame_create( wid );
    frame_background->size_in_pixels1_relative0 = 0;
    frame_background->x0 = 0.5;
    frame_background->y0 = 0.0;
    frame_background->x1 = 1.0;
    frame_background->y1 = 0.5;
    frame_background->preference = -1;

    gwCamera* camera_background = gwCamera_create();
    camera_background->type = GW_CAMERA_VIEW_FLAG_NO_SCREEN_RATIO;
    gwFrame_addCamera( frame_background, camera_background );

    gwTexture* tex_bluesky = gwTexture_loadBMP( "../dev/zdev04/blue_sky.bmp" );
    gwMesh* mesh_background = gwShape_quad( 1.0f );
    mesh_background->size = 2;
    mesh_background->shaders = GW_SHADER_TEXTURE;
    mesh_background->texture0 = tex_bluesky;
    mesh_background->self_ilumination = 1;
    gwCamera_addMesh( camera_background, mesh_background );

    /* For rendering sprites, there is an especific shader, although
     * this scene can be rendered with the texture shader, as above */
    gwFrame* frame = gwFrame_create( wid );
    frame->size_in_pixels1_relative0 = 0;
    frame->x0 = 0.5;
    frame->y0 = 0.0;
    frame->x1 = 1.0;
    frame->y1 = 0.5;
    /* Let the color to 0, to avoid overlap the previous frame */
    frame->background_color.a = 0;

    gwTexture* tex = gwTexture_loadTGA( "../dev/zdev04/seasons.tga" );
    tex->interpolation = 0;
    gwTexture_transparencyMask( tex, gwColorMagenta, 0 );

    /* In OpenGL the {0,0} corner is the down left.
    * For convenience, the 'y' position is inverted in the blit camera */
    mesh_spring = gwShape_sprite( 64, 64 );
    mesh_spring->shaders = GW_SHADER_BLIT;
    mesh_spring->texture0 = tex;
    mesh_spring->position.x = 64;
    mesh_spring->position.y = 64;

    /* We can make sprites bigger, but usually they ought to be multiple of 2 */
    mesh_summer = gwShape_sprite( 128, 128 );
    mesh_summer->shaders = GW_SHADER_BLIT;
    mesh_summer->texture0 = tex;
    mesh_summer->position.x = 128;
    mesh_summer->position.y = 0;
    /* The first sprite drawn overlaps the next ones.
     * Use the z coordinate to control how sprites overlap each other. */
    mesh_summer->position.z = 1.0f;

    mesh_winter = gwShape_sprite( 64, 64 );
    mesh_winter->shaders = GW_SHADER_BLIT;
    mesh_winter->texture0 = tex;
    mesh_winter->position.x = 0;
    mesh_winter->position.y = 0;
    /* A transparency can blend with the primary color with the alpha channel */
    mesh_winter->primary_color.a = 64;

    mesh_autum = gwShape_sprite( 64, 64 );
    mesh_autum->shaders = GW_SHADER_BLIT;
    mesh_autum->texture0 = tex;
    mesh_autum->position.x = 128;
    mesh_autum->position.y = 196;
    /* Rotations can be set using the 'z' axis */
    mesh_autum->rotation.z = 30;

    /* It is a common practice to have several textures stored in the same image
     * to improve GPU performance. Then the texture coordinates are adjusted
     * to get the slab. This function helps to do that. */
    //gwMesh_textureSlab( mesh_spring, 0, 0, 2, 2 );
    gwMesh_textureSlab( mesh_summer, 1, 1, 2, 2 );
    gwMesh_textureSlab( mesh_winter, 1, 0, 2, 2 );
    gwMesh_textureSlab( mesh_autum, 0, 1, 2, 2 );

    gwCamera* camera = gwCamera_create();
    camera->type = GW_CAMERA_VIEW_BLIT;
    gwFrame_addCamera( frame, camera );
    gwCamera_addMesh( camera, mesh_spring );
    gwCamera_addMesh( camera, mesh_summer );
    gwCamera_addMesh( camera, mesh_winter );
    gwCamera_addMesh( camera, mesh_autum );

    /* There are some variables to be adjusted in the shader */
    gwShader* shader_blit = gw_getShader( wid, GW_SHADER_BLIT );
    shader_blit->alpha_test = 1;
}

/* Used to animate the scene */
void animate_scene( gwWindowContext* context )
{
    static double t = 0;
    static double pi = 4 * atan( 1.0 );

    for (size_t i = 0; i < 5; i++){
        double x = control_points[i].x;
        double y = sin( t + x ) + cos( 1.333*t + x ) + sin( 1.666*t + x );
        control_points[i].y = (gwFloat)(y/2.5);
    }
    t += 0.1;
    if (t >= 1e6 * pi) t = 0;

    if (mesh_points != nullptr){
        for (size_t i = 0; i < mesh_points->vertex->length; i++){
            mesh_points->vertex->stream[i].position.x = control_points[i].x;
            mesh_points->vertex->stream[i].position.y = control_points[i].y;
            mesh_points->vertex->stream[i].position.z = control_points[i].z;
        }
        mesh_points->vertex->update = 1;
    }

    if (mesh_lines_0 != nullptr){
        for (int imesh = 0; imesh < 31; imesh++){
            for (size_t i = 0; i < mesh_lines_0->vertex->length; i++){
                mesh_lines[imesh]->vertex->stream[i] = mesh_lines[imesh + 1]->vertex->stream[i];
            }
            mesh_lines[imesh]->vertex->update = 1;
        }

        for (size_t i = 0; i < mesh_lines_0->vertex->length; i++){
            mesh_lines[31]->vertex->stream[i] = mesh_lines_0->vertex->stream[i];
        }
        mesh_lines[31]->vertex->update = 1;

        for (size_t i = 0; i < mesh_lines_0->vertex->length; i++){
            float t = (float)i / (mesh_lines_0->vertex->length - 1);
            gwVector3f p = gwspline( &control_points[0], 5, t );
            mesh_lines_0->vertex->stream[i].position.x = p.x;
            mesh_lines_0->vertex->stream[i].position.y = p.y;
            mesh_lines_0->vertex->stream[i].position.z = p.z;
        }
        mesh_lines_0->vertex->update = 1;
    }

    if (mesh_autum != nullptr){
        double s = t / 10;
        mesh_autum->position.x = gwFloat(128 * (sin( s ) * cos( 2 * s ) + 1.1));
        mesh_autum->position.y = gwFloat( 128 * (cos( s ) * sin( 2 * s ) + 1) );
        mesh_autum->rotation.z = gwFloat( 10 * t );
    }
}

int main()
{
    /* Create a the OpenGL context using GLFW.*/
    wid = glfwWindow_create_gl_context( gl_hint );
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

    first_scene();
    second_scene();
    third_scene();
    fourth_scene();

    /* To animate, we will use a call that is triggered each render loop */
    context->f_on_render = animate_scene;

    /* The render loop */
    int status = 1;
    while (status > 0){
        status = glfw_render();

        /* Because there is an animation, we need to force an update */
        context->update = 1;
    }

    return 0;
}
