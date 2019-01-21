/**
Author: Mario J. Martin <dominonurbs$gmail.com>

This example shows a shader for drawing isolines from a scalar map; 
e.g. the pressure obtained from a fluid simulation.
This example calculates the laplacian using a Jacobi algorithm and draw the
results asyncronously. The simulation is running in a different thread.

To obtain this effect a multi pass shader is required, which requires a buffer.
Each context has an auxiliary buffer, so the process is transparent to the user.

*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "glw/gwgl.h"

#include "context_glfw/context_glfw.h"
#include "glw/glw.h"
#include "glw/gwmath.h"
#include "glw/gwthreads.h"

struct Grid
{
    float* data;
    int* bc;
    int width, height;

    float* cell_data( int i, int j )
    {
        int icell = i + j * width;
        return &(data[icell]);
    }
    int* cell_bc( int i, int j )
    {
        int icell = i + j * width;
        return &(bc[icell]);
    }
}grid;

gwMesh* mesh_grid = nullptr;

float absf( const float c )
{
    return (c < 0) ? -c : c;
}

void init_grid( Grid* grid, const int width, const int height )
{
    grid->width = width;
    grid->height = height;

    grid->data = (float*)malloc( sizeof( float ) * grid->width  * grid->height );
    grid->bc = (int*)malloc( sizeof( int ) * grid->width  * grid->height );

    for (int i = 0; i < grid->width; i++){
        for (int j = 0; j < grid->height; j++){
            *(grid->cell_data( i, j )) = 0;
            if (i != 0 && i != grid->width - 1 && j != 0 && j != grid->height - 1){
                *(grid->cell_bc( i, j )) = 0;
            }
            else{
                *(grid->cell_bc( i, j )) = 1;
            }
        }
    }

    srand( (int)time( NULL ) );
    for (int k = 0; k < 32; k++){
        int i = rand() % (grid->width - 2) + 1;
        int j = rand() % (grid->height - 2) + 1;

        *(grid->cell_bc( i, j )) = 1;
        *(grid->cell_data( i, j )) = 1;
    }
}

void jacobi()
{
    float residual = 1;
    float alpha = 0.5f;

    while (residual > 1e-3){
        residual = 0;
        for (int i = 0; i < grid.width; i++){
            for (int j = 0; j < grid.height; j++){
                int bc = *(grid.cell_bc( i, j ));
                if (bc == 0){
                    float c0 = *(grid.cell_data( i, j ));
                    float a0 = *(grid.cell_data( i + 1, j ));
                    float a1 = *(grid.cell_data( i - 1, j ));
                    float a2 = *(grid.cell_data( i, j + 1 ));
                    float a3 = *(grid.cell_data( i, j - 1 ));

                    float cell_value = alpha * 0.25f * (a0 + a1 + a2 + a3) + (1 - alpha) * c0;
                    *(grid.cell_data( i, j )) = cell_value;
                    float res = absf( cell_value - c0 );
                    if (res > residual){
                        residual = res;
                    }
                }
            }
        }
        printf( "%g\n", residual );

        /* Artificially slow down the simulation */
        gw_sleep( 25 );
    }
}

void create_mesh( gwMesh* mesh, const Grid* grid )
{
    /* Create the vertex stream */
    mesh->vertex = gwVertexStream_create( grid->width * grid->height );
    int ivert = 0;
    for (int i = 0; i < grid->height; i++){
        for (int j = 0; j < grid->width; j++){
            float x = (float)(j) / (grid->width - 1);
            float y = (float)(i) / (grid->height - 1);
            mesh->vertex->stream[ivert].position.x = 2 * x - 1.0f;
            mesh->vertex->stream[ivert].position.y = 2 * y - 1.0f;
            mesh->vertex->stream[ivert].position.z = 0;
            mesh->vertex->stream[ivert].normal.x = 0;
            mesh->vertex->stream[ivert].normal.y = 0;
            mesh->vertex->stream[ivert].normal.z = 1;
            mesh->vertex->stream[ivert].texCoord.u = x;
            mesh->vertex->stream[ivert].texCoord.v = y;
            ivert++;
        }
    }
    mesh->vertex->update = 1;

    /* Create the index stream */
    int index_len = 6 * (grid->height - 1) * (grid->width - 1);
    mesh->index = gwIndexStream_create( index_len, GW_INDEX_TRIANGLES );
    int in = 0;
    for (int i = 0; i < grid->height - 1; i++){
        for (int j = 0; j < grid->width - 1; j++){
            int i0 = i * grid->width + j;
            int i1 = i * grid->width + j + 1;
            int i2 = (i + 1) * grid->width + j + 1;
            int i3 = (i + 1) * grid->width + j;

            mesh->index->stream[in] = i0; in++;
            mesh->index->stream[in] = i1; in++;
            mesh->index->stream[in] = i2; in++;

            mesh->index->stream[in] = i0; in++;
            mesh->index->stream[in] = i2; in++;
            mesh->index->stream[in] = i3; in++;
        }
    }
    mesh->index->update = 1;
}

void update_grid( gwWindowContext* context )
{
    mesh_grid->scalar->update = 1;
}

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
    context->screen_height = 800;

    /* Load the shader */
    int sh_isolines = gwShader_load( wid
        , "../res/shaders", "scalar.vsh", "scalar0.fsh", nullptr
        , GW_POLYGON_FILL );
    gwShader* shader_isolines_pass1 = gw_getShader( wid, sh_isolines );

    /* Add a multi pass */
    gwShader* sh_isolines_pass2 = gwShader_addpass( wid, shader_isolines_pass1
        , "../res/shaders", "scalar1.fsh" );

    /* Load other other shaders */
    int sh_solid = gwShader_load( wid
        , "../res/shaders", "lambertian_lighting.vsh", "lambertian_lighting.fsh", nullptr
        , GW_POLYGON_FILL );
    gwShader* shader_solid = gw_getShader( wid, sh_solid );

    int sh_wired = gwShader_load( wid
        , "../res/shaders", "wireframe.vsh", "wireframe.fsh", nullptr
        , GW_POLYGON_LINE );
    gwShader* shader_wired = gw_getShader( wid, sh_wired );

    /* Initialize the grid */
    init_grid( &grid, 64, 64 );

    /* Create an equivalent mesh to draw the grid */
    mesh_grid = gwMesh_create();
    create_mesh( mesh_grid, &grid );    
    mesh_grid->primary_color = gwColorGray;

    /* Assign the scalar to a stream of float values
    * and assign the shader for isolines
    * Very important is to assign the min and max values to normalize */
    mesh_grid->scalar = gwScalarStream_create( grid.data, grid.width * grid.height );
    mesh_grid->scalar_range_min = 0;
    mesh_grid->scalar_range_max = 1;
    mesh_grid->shaders = sh_isolines;

    /* Create the frame and the camera */
    gwCamera* camera = gwCamera_create();
    gwWindowContext_attachCamera( wid, camera );
    gwCamera_behaviour_cad( camera );
    camera->type = GW_CAMERA_VIEW_CAD;

    /* Create an object in the scene to check that the stencil in working */
    gwMesh* mesh_solid = gwShape_sphere( 8 );
    gwMesh* mesh_wired = gwMesh_create();
    mesh_wired->vertex = mesh_solid->vertex;
    mesh_wired->index = mesh_solid->index;

    mesh_solid->size = 0.5f;
    mesh_solid->shaders = sh_solid;
    mesh_solid->primary_color = gwColorRed;

    mesh_wired->size = 0.5f;
    mesh_wired->primary_color = gwColorYellow;
    mesh_wired->shaders = sh_wired;

    gwCamera_addMesh( camera, mesh_solid );
    gwCamera_addMesh( camera, mesh_wired );
    gwCamera_addMesh( camera, mesh_grid );

    /* Run the simulation in another thread */
    gw_launch_thread( jacobi );

    gwFrame* frame = gwFrame_create( wid );
    gwFrame_addCamera( frame, camera );
    gwCamera_addMesh( camera, mesh_solid );
    gwCamera_addMesh( camera, mesh_wired );
    gwCamera_addMesh( camera, mesh_grid );

    int status = 0;
    while (status > 0){

        /* Update the scalar values */
        if (mesh_grid->scalar != nullptr){
            mesh_grid->scalar->update = 1;
        }

        /* Check the window status */
        status = glfwWindow_render_begin( wid );
        if (status == 0){
            break;
        }

        /* Draw the scene */
        gwMatrix4f model_matrix = gwMatrix4f_identity();
        gwMatrix4f view_matrix = gwCamera_calculate_view_matrix( camera );

        /* Draw the entities as usual */
        gwCamera_render( camera );

        //model_matrix = gwMesh_calculate_model_matrix( mesh_wired );
        //gwMesh_draw( mesh_wired, shader_wired, model_matrix, view_matrix );

        //model_matrix = gwMesh_calculate_model_matrix( mesh_grid );
        //gwMesh_draw( mesh_grid, shader_isolines_pass1, model_matrix, view_matrix );

        //model_matrix = gwMesh_calculate_model_matrix( mesh_solid );
        //gwMesh_draw( mesh_solid, shader_solid, model_matrix, view_matrix );

        /* Update the render buffer */
        glfwWindow_render_end();

        /* Sleeps to adjust the framerate */
        gw_adjust_framerate( 25 );
    }

    /* Run the visualization */
    context->f_on_render = update_grid;
    glfw_render_thread();

    printf( "Press ENTER to exit\n" );
    getchar();
}
