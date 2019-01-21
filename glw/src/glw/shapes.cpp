/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Some basic geometries.

*******************************************************************************/

#include <math.h>
#include <memory.h>
#include <stdlib.h>

#include "gwdata.h"
#include "glw.h"

/* Creates a quad */
extern "C"
gwMesh* gwShape_quad( const gwFloat size )
{
    const gwFloat x0 = -size / 2;
    const gwFloat y0 = -size / 2;
    const gwFloat width = size;
    const gwFloat height = size;
    gwIndex index_triangles[] = { 0, 1, 2, 0, 2, 3 };

    gwMesh* mesh = gwMesh_create();
    mesh->vertex = gwVertexStream_create( 4 );
    mesh->index = gwIndexStream_create( 6, GW_INDEX_TRIANGLES );
    mesh->culling = 0;

    gwVertexStream_set( mesh->vertex, 0, x0, y0, 0, 0, 0, 1, 0, 0 );
    gwVertexStream_set( mesh->vertex, 1, x0 + width, y0, 0, 0, 0, 1, 1, 0 );
    gwVertexStream_set( mesh->vertex, 2, x0 + width, y0 + height, 0, 0, 0, 1, 1, 1 );
    gwVertexStream_set( mesh->vertex, 3, x0, y0 + height, 0, 0, 0, 1, 0, 1 );

    memcpy( mesh->index->stream, index_triangles, sizeof( gwIndex ) * 6 );

    return mesh;
}

/* Creates a 1 by 1 quad wireframe */
extern "C"
gwMesh* gwWireframe_quad( const gwFloat size )
{
    const gwFloat x0 = -size / 2;
    const gwFloat y0 = -size / 2;
    const gwFloat width = size;
    const gwFloat height = size;
    gwIndex index_lines[] = { 0, 1, 1, 2, 2, 3, 3, 0 };

    gwMesh* mesh = gwMesh_create();
    mesh->vertex = gwVertexStream_create( 4 );
    mesh->index = gwIndexStream_create( 8, GW_INDEX_LINES );
    mesh->culling = 0;

    gwVertexStream_set( mesh->vertex, 0, x0, y0, 0, 0, 0, 1, 0, 0 );
    gwVertexStream_set( mesh->vertex, 1, x0 + width, y0, 0, 0, 0, 1, 1, 0 );
    gwVertexStream_set( mesh->vertex, 2, x0 + width, y0 + height, 0, 0, 0, 1, 1, 1 );
    gwVertexStream_set( mesh->vertex, 3, x0, y0 + height, 0, 0, 0, 1, 0, 1 );

    memcpy( mesh->index->stream, index_lines, sizeof( gwIndex ) * 8 );

    return mesh;
}

/* Creates a [0,1] quad. 
 * This is convenient to render to the full screen without any transformation */
extern "C"
gwMesh* gwShape_quad01()
{
    const gwFloat x0 = 0;
    const gwFloat y0 = 0;
    const gwFloat width = 1;
    const gwFloat height = 1;
    gwIndex index_triangles[] = { 0, 1, 2, 0, 2, 3 };

    gwMesh* mesh = gwMesh_create();
    mesh->vertex = gwVertexStream_create( 4 );
    mesh->index = gwIndexStream_create( 6, GW_INDEX_TRIANGLES );
    mesh->culling = 0;

    gwVertexStream_set( mesh->vertex, 0, x0, y0, 0, 0, 0, 1, 0, 0 );
    gwVertexStream_set( mesh->vertex, 1, x0 + width, y0, 0, 0, 0, 1, 1, 0 );
    gwVertexStream_set( mesh->vertex, 2, x0 + width, y0 + height, 0, 0, 0, 1, 1, 1 );
    gwVertexStream_set( mesh->vertex, 3, x0, y0 + height, 0, 0, 0, 1, 0, 1 );

    memcpy( mesh->index->stream, index_triangles, sizeof( gwIndex ) * 6 );

    return mesh;
}

/* Creates a quad. If it is for blitting, the dimensions are in pixels. */
extern "C"
gwMesh* gwShape_sprite( const int width, const int height )
{
    const gwFloat x0 = 0;
    const gwFloat y0 = 0;
    const gwFloat w = (gwFloat)width;
    const gwFloat h = (gwFloat)height;
    gwIndex index_triangles[] = { 0, 1, 2, 0, 2, 3 };

    gwMesh* mesh = gwMesh_create();
    mesh->vertex = gwVertexStream_create( 4 );
    mesh->index = gwIndexStream_create( 6, GW_INDEX_TRIANGLES );
    mesh->culling = 0;

    gwVertexStream_set( mesh->vertex, 0, x0, y0, 0, 0, 0, 1, 0, 0 );
    gwVertexStream_set( mesh->vertex, 1, x0 + w, y0, 0, 0, 0, 1, 1, 0 );
    gwVertexStream_set( mesh->vertex, 2, x0 + w, y0 + h, 0, 0, 0, 1, 1, 1 );
    gwVertexStream_set( mesh->vertex, 3, x0, y0 + h, 0, 0, 0, 1, 0, 1 );

    memcpy( mesh->index->stream, index_triangles, sizeof( gwIndex ) * 6 );

    return mesh;
}

/* Creates a quad. If it is for blitting, the dimensions are in pixels. */
extern "C"
gwMesh* gwWireframe_sprite( const int width, const int height )
{
    const gwFloat x0 = -(gwFloat)width / 2;
    const gwFloat y0 = -(gwFloat)height / 2;
    const gwFloat w = (gwFloat)width;
    const gwFloat h = (gwFloat)height;
    gwIndex index_lines[] = { 0, 1, 1, 2, 2, 3, 3, 0 };

    gwMesh* mesh = gwMesh_create();
    mesh->vertex = gwVertexStream_create( 4 );
    mesh->index = gwIndexStream_create( 8, GW_INDEX_LINES );
    mesh->culling = 0;

    gwVertexStream_set( mesh->vertex, 0, x0, y0, 0, 0, 0, 1, 0, 0 );
    gwVertexStream_set( mesh->vertex, 1, x0 + w, y0, 0, 0, 0, 1, 1, 0 );
    gwVertexStream_set( mesh->vertex, 2, x0 + w, y0 + h, 0, 0, 0, 1, 1, 1 );
    gwVertexStream_set( mesh->vertex, 3, x0, y0 + h, 0, 0, 0, 1, 0, 1 );

    memcpy( mesh->index->stream, index_lines, sizeof( gwIndex ) * 8 );

    return mesh;
}

/******************************************************************************/

/* Vertices position for a box (a box has independant faces)*/
static const gwVector3f cube_vertices[] =
{
    // front
    { 0.5f, 0.5f, -0.5f },      // (2)    0
    { 0.5f, 0.5f, 0.5f },       // (5)    1
    { -0.5f, 0.5f, 0.5f },      // (3)    2
    { -0.5f, 0.5f, -0.5f },     // (4)    3

    // top
    { 0.5f, 0.5f, 0.5f },       // (5)    4
    { 0.5f, -0.5f, 0.5f },      // (1)    5
    { -0.5f, -0.5f, 0.5f },     // (7)    6
    { -0.5f, 0.5f, 0.5f },      // (3)    7

    // left
    { -0.5f, -0.5f, 0.5f },     // (7)    8
    { -0.5f, -0.5f, -0.5f },    // (0)    9
    { -0.5f, 0.5f, -0.5f },     // (4)    10
    { -0.5f, 0.5f, 0.5f },      // (3)    11

    // right
    { 0.5f, 0.5f, 0.5f },       // (5)    12
    { 0.5f, 0.5f, -0.5f },      // (2)    13
    { 0.5f, -0.5f, -0.5f },     // (6)    14
    { 0.5f, -0.5f, 0.5f },      // (1)    15    

    // botton
    { 0.5f, -0.5f, -0.5f },     // (6)    16
    { 0.5f, 0.5f, -0.5f },      // (2)    17
    { -0.5f, 0.5f, -0.5f },     // (4)    18
    { -0.5f, -0.5f, -0.5f },    // (0)    19

    // back
    { -0.5f, -0.5f, -0.5f },    // (0)    20
    { -0.5f, -0.5f, 0.5f },     // (7)    21
    { 0.5f, -0.5f, 0.5f },      // (1)    22
    { 0.5f, -0.5f, -0.5f },     // (6)    23
};

static const gwVector3f cube_normals[] =
{
    // front
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },

    // top
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f },

    // left
    { -1.0f, 0.0f, 0.0f },
    { -1.0f, 0.0f, 0.0f },
    { -1.0f, 0.0f, 0.0f },
    { -1.0f, 0.0f, 0.0f },

    // right
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },

    // botton
    { 0.0f, 0.0f, -1.0f },
    { 0.0f, 0.0f, -1.0f },
    { 0.0f, 0.0f, -1.0f },
    { 0.0f, 0.0f, -1.0f },

    // back
    { 0.0f, -1.0f, 0.0f },
    { 0.0f, -1.0f, 0.0f },
    { 0.0f, -1.0f, 0.0f },
    { 0.0f, -1.0f, 0.0f },
};

const gwTexCoord cube_textures[] =
{
    // front
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 1.0f },
    { 0.0f, 0.0f },

    // top
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 1.0f },
    { 0.0f, 0.0f },

    // left
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 1.0f },
    { 0.0f, 0.0f },

    // right 
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 1.0f },
    { 0.0f, 0.0f },

    // botton
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 1.0f },
    { 0.0f, 0.0f },

    // back
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 1.0f },
    { 0.0f, 0.0f },
};

const gwIndex cube_surface_indexes[] =
{
    0, 1, 2, 0, 2, 3,           // front
    4, 5, 6, 4, 6, 7,           // top
    8, 9, 10, 8, 10, 11,        // left
    12, 13, 14, 12, 14, 15,     // right
    16, 17, 18, 16, 18, 19,     // botton
    20, 21, 22, 20, 22, 23,     // back
};

const gwIndex cube_wireframe_indexes[] =
{
    0, 1,
    1, 2,
    2, 3,
    3, 0,
    20, 21,
    21, 22,
    22, 23,
    23, 20,
    0, 23,
    1, 22,
    2, 21,
    3, 20,
};

/* A cube has independant faces, so the normals are more realistic. */
extern "C"
gwMesh* gwShape_cube()
{
    const gwFloat sx = (gwFloat)1.0f;
    const gwFloat sy = (gwFloat)1.0f;
    const gwFloat sz = (gwFloat)1.0f;

    gwMesh* mesh = gwMesh_create();

    mesh->vertex = gwVertexStream_create( 24 );
    for (size_t i = 0; i < 24; i++){
        mesh->vertex->stream[i].position.x = cube_vertices[i].x * sx;
        mesh->vertex->stream[i].position.y = cube_vertices[i].y * sy;
        mesh->vertex->stream[i].position.z = cube_vertices[i].z * sz;
        mesh->vertex->stream[i].normal = cube_normals[i];
        mesh->vertex->stream[i].texCoord = cube_textures[i];
    }
    mesh->culling = 1;

    mesh->index = gwIndexStream_create( 36, GW_INDEX_TRIANGLES );
    memcpy( mesh->index->stream
        , cube_surface_indexes, sizeof( gwIndex ) * 36 );

    return mesh;
}

/* A cube has independant faces, so the normals are more realistic. */
extern "C"
gwMesh* gwWireframe_cube()
{
    const gwFloat sx = (gwFloat)1.0f;
    const gwFloat sy = (gwFloat)1.0f;
    const gwFloat sz = (gwFloat)1.0f;

    gwMesh* mesh = gwMesh_create();

    mesh->vertex = gwVertexStream_create( 24 );
    for (size_t i = 0; i < 24; i++){
        mesh->vertex->stream[i].position.x = cube_vertices[i].x * sx;
        mesh->vertex->stream[i].position.y = cube_vertices[i].y * sy;
        mesh->vertex->stream[i].position.z = cube_vertices[i].z * sz;
        mesh->vertex->stream[i].normal = cube_normals[i];
        mesh->vertex->stream[i].texCoord = cube_textures[i];
    }
    mesh->culling = 1;

    mesh->index = gwIndexStream_create( 24, GW_INDEX_LINES );
    memcpy( mesh->index->stream
        , cube_wireframe_indexes, sizeof( gwIndex ) * 24 );

    return mesh;
}

/******************************************************************************/

static gwVertexStream* icosahedron_vertices()
{
    gwVertexStream* vertex = gwVertexStream_create( 12 );

    const gwFloat r = gwFloat( 1.0 );   /* radius */
    const double pi = 4 * atan( 1.0 );
    const gwFloat phi = gwFloat( pi / 5 );
    const gwFloat c = gwFloat( 1. / sqrt( 5.0 ) );
    const gwFloat c1 = (gwFloat)cos( 2 * phi );
    const gwFloat c2 = (gwFloat)cos( phi );
    const gwFloat s = 2 * c;
    const gwFloat s1 = (gwFloat)sin( 2 * phi );
    const gwFloat s2 = (gwFloat)sin( phi );
    const gwFloat ss1 = s*s1;
    const gwFloat ss2 = s*s2;
    const gwFloat sc1 = s*c1;
    const gwFloat sc2 = s*c2;
    const gwFloat ssr1 = s*s1*r;
    const gwFloat ssr2 = s*s2*r;
    const gwFloat cr = c*r;
    const gwFloat sr = s*r;
    const gwFloat sc1r = s*c1*r;
    const gwFloat sc2r = s*c2*r;

    gwVertexStream_set( vertex, 0, 0, r, 0, 0, 1, 0, 0.0f, 0.0f );
    gwVertexStream_set( vertex, 1, 0, cr, sr, 0, c, s, 0.3333f, 0.0f );
    gwVertexStream_set( vertex, 2, ssr1, cr, sc1r, ss1, c, sc1, 0.3333f, 0.25f );
    gwVertexStream_set( vertex, 3, ssr2, cr, -sc2r, ss2, c, -sc2, 0.3333f, 0.50f );
    gwVertexStream_set( vertex, 4, -ssr2, cr, -sc2r, -ss2, c, -sc2, 0.3333f, 0.75f );
    gwVertexStream_set( vertex, 5, -ssr1, cr, sc1r, -ss1, c, sc1, 0.3333f, 1.0f );
    gwVertexStream_set( vertex, 6, ssr2, -cr, sc2r, ss2, -c, sc2, 0.6667f, 0.0f );
    gwVertexStream_set( vertex, 7, ssr1, -cr, -sc1r, ss1, -c, -sc1, 0.6667f, 0.25f );
    gwVertexStream_set( vertex, 8, 0, -cr, -sr, 0, -c, -s, 0.6667f, 0.50f );
    gwVertexStream_set( vertex, 9, -ssr1, -cr, -sc1r, -ss1, -c, -sc1, 0.6667f, 0.75f );
    gwVertexStream_set( vertex, 10, -ssr2, -cr, sc2r, -ss2, -c, sc2, 0.6667f, 1.0f );
    gwVertexStream_set( vertex, 11, 0, -r, 0, 0, -1, 0, 1.0f, 1.0f );

    return vertex;
}

static gwIndexStream* icosahedron_index_triangles()
{
    const gwIndex index_triangles[] =
    {
        2, 1, 0,
        3, 2, 0,
        4, 3, 0,
        5, 4, 0,
        1, 5, 0,
        11, 6, 7,
        11, 7, 8,
        11, 8, 9,
        11, 9, 10,
        11, 10, 6,
        1, 2, 6,
        2, 3, 7,
        3, 4, 8,
        4, 5, 9,
        5, 1, 10,
        2, 7, 6,
        3, 8, 7,
        4, 9, 8,
        5, 10, 9,
        1, 6, 10
    };

    gwIndexStream* index_surface = gwIndexStream_create( 60, GW_INDEX_TRIANGLES );
    memcpy( index_surface->stream, index_triangles, sizeof( gwIndex ) * 60 );

    return index_surface;
}

extern "C"
gwMesh* gwShape_icosahedron()
{
    gwMesh* mesh = gwMesh_create();
    gwVertexStream* vertex = icosahedron_vertices();
    mesh->vertex = vertex;
    mesh->index = icosahedron_index_triangles();
    mesh->culling = 1;

    return mesh;
}

/******************************************************************************/

static void sphere
( gwVertexStream* vertex_stream
, gwIndexStream* index_stream
, const int refinement 
){
    const int n = refinement > 3 ? refinement : 3;
    const gwFloat r = 1;
    const gwFloat pi = gwFloat( 4 * atan( 1.0 ) );
    const gwFloat deltaAzimuth = (2 * pi) / (n - 1);

    gwFloat* precal_zenit = (gwFloat*)malloc( sizeof( gwFloat ) * n );

    /* Calculate the zenits */
    for (int i = 0; i < n; i++){
        double c = (double)i / (n - 1);
        precal_zenit[i] = (gwFloat)(-2 * c*c*c + 3 * c*c);
        //precal_zenit[i] = (GLfloat)(sin(pi*(c-0.5))+1)/2;
    }

    /*****************/
    /* Vertex stream */
    /*****************/
    gwVertex* vertex = (gwVertex*)malloc( sizeof( gwVertex )*n*n );
    vertex_stream->stream = vertex;
    vertex_stream->length = n*n;
    int k = 0;
    for (int i = 0; i < n; i++){
        double azimuth = 0;
        double texzenit = precal_zenit[i];
        double zenit = precal_zenit[i] * pi;
        for (int j = 0; j < n; j++){
            azimuth += deltaAzimuth;
            gwFloat x = (gwFloat)(sin( zenit ) * cos( azimuth ));
            gwFloat y = (gwFloat)(sin( zenit ) * sin( azimuth ));
            gwFloat z = (gwFloat)(cos( zenit ));
            vertex[k].position.x = x*r;
            vertex[k].position.y = y*r;
            vertex[k].position.z = z*r;
            vertex[k].normal.x = x;
            vertex[k].normal.y = y;
            vertex[k].normal.z = z;
            vertex[k].texCoord.u = 1.0f - (gwFloat)(n - j) / n;
            vertex[k].texCoord.v = gwFloat(texzenit);
            k++;
        }
    }

    /****************/
    /* Index stream */
    /****************/
    const size_t index_length = 6 * (n - 1)*(n - 1);
    gwIndex* index_surface = (gwIndex*)malloc
        ( sizeof( gwIndex ) * index_length );
    index_stream->stream = index_surface;
    index_stream->length = index_length;

    // set index (triangles) connectivities
    k = 0;
    for (int i = 0; i < n - 1; i++){
        for (int j = 0; j < n - 1; j++){
            gwIndex i0 = (gwIndex)(j + i*n);
            gwIndex i1 = (gwIndex)(j + 1 + i*n);
            gwIndex i2 = (gwIndex)(j + (i + 1)*n);
            gwIndex i3 = (gwIndex)(j + 1 + (i + 1)*n);

            if (i % 2 == 0){
                index_surface[k++] = i0;
                index_surface[k++] = i1;
                index_surface[k++] = i2;

                index_surface[k++] = i3;
                index_surface[k++] = i2;
                index_surface[k++] = i1;
            }
            else{
                index_surface[k++] = i0;
                index_surface[k++] = i3;
                index_surface[k++] = i2;

                index_surface[k++] = i0;
                index_surface[k++] = i1;
                index_surface[k++] = i3;
            }
        }
    }

    free( precal_zenit );
}

extern "C"
gwMesh* gwShape_sphere( const int refinement )
{
    gwMesh* mesh = gwMesh_create();
    mesh->vertex = gwVertexStream_create( 0 );
    mesh->index = gwIndexStream_create( 0, GW_INDEX_TRIANGLES );

    sphere( mesh->vertex, mesh->index, refinement );
    mesh->culling = 1;

    return mesh;
}

extern "C"
gwMesh* gwShape_tiles( const int width, const int height )
{
    gwMesh* mesh = gwMesh_create();
    mesh->vertex = gwVertexStream_create( width * height );
    mesh->index = gwIndexStream_create( 6 * (width - 1) * (height - 1), GW_INDEX_TRIANGLES );
    mesh->culling = 1;

    int k = 0;
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            mesh->vertex->stream[k].position.x = (gwFloat)j / (width - 1);
            mesh->vertex->stream[k].position.y = (gwFloat)i / (width - 1);
            mesh->vertex->stream[k].position.z = 0;

            mesh->vertex->stream[k].normal.x = 0;
            mesh->vertex->stream[k].normal.y = 0;
            mesh->vertex->stream[k].normal.z = 1;

            mesh->vertex->stream[k].texCoord.u = (gwFloat)j / (width - 1);
            mesh->vertex->stream[k].texCoord.v = (gwFloat)i / (width - 1);

            k++;
        }
    }

    k = 0;
    int swap = 1;
    for (int i = 0; i < height - 1; i++){
        for (int j = 0; j < width - 1; j++){
            int gid_0 = i + j*width;
            int gid_1 = i + j*width + 1;
            int gid_2 = i + (j + 1)*width + 1;
            int gid_3 = i + (j + 1)*width;

            if (swap == 0){
                swap = 1;
                mesh->index->stream[k] = gid_0; k++;
                mesh->index->stream[k] = gid_2; k++;
                mesh->index->stream[k] = gid_3; k++;

                mesh->index->stream[k] = gid_0; k++;
                mesh->index->stream[k] = gid_1; k++;
                mesh->index->stream[k] = gid_2; k++;
            }
            else{
                swap = 0;
                mesh->index->stream[k] = gid_1; k++;
                mesh->index->stream[k] = gid_3; k++;
                mesh->index->stream[k] = gid_0; k++;

                mesh->index->stream[k] = gid_1; k++;
                mesh->index->stream[k] = gid_2; k++;
                mesh->index->stream[k] = gid_3; k++;
            }
        }
    }

    return mesh;
}

extern "C"
gwMesh* gwShape_cloudpoint( const int number_points )
{
    gwMesh* mesh = gwMesh_create();
    mesh->vertex = gwVertexStream_create( number_points );
    mesh->index = gwIndexStream_create( 0, GW_INDEX_POINTS );
    mesh->culling = 0;

    return mesh;
}

extern "C"
gwMesh* gwShape_line( const int number_points )
{
    gwMesh* mesh = gwMesh_create();
    mesh->vertex = gwVertexStream_create( number_points );
    mesh->index = gwIndexStream_create( 0, GW_INDEX_LINE_STRIP );
    mesh->culling = 0;

    return mesh;
}

extern "C"
gwMesh* gwShape_cilinder
( const int number_points
, const double top_base_ratio
, const double angle_offset
)
{
    gwMesh* mesh = gwMesh_create();
    const double pi = 4 * atan( 1.0 );
    const double af = angle_offset * (pi / 180);

    mesh->vertex = gwVertexStream_create( number_points * 4 + 2 );
    mesh->index = gwIndexStream_create( number_points * 12, GW_INDEX_TRIANGLES );
    mesh->culling = 1;

    /* Create the vertices */
    int j = 0;
    for (int i = 0; i < number_points; i++){
        gwFloat x = (gwFloat)(sin( (i * 2 * pi) / number_points ));
        gwFloat y = (gwFloat)(cos( (i * 2 * pi) / number_points ));

        mesh->vertex->stream[j].position.x = (gwFloat)(x * top_base_ratio);
        mesh->vertex->stream[j].position.y = (gwFloat)(y * top_base_ratio);
        mesh->vertex->stream[j].position.z = 0.5f;
        mesh->vertex->stream[j].normal.x = x;
        mesh->vertex->stream[j].normal.y = y;
        mesh->vertex->stream[j].normal.z = 0;
        j++;

        x = (gwFloat)(sin( (i * 2 * pi) / number_points + af ));
        y = (gwFloat)(cos( (i * 2 * pi) / number_points + af ));

        mesh->vertex->stream[j].position.x = x;
        mesh->vertex->stream[j].position.y = y;
        mesh->vertex->stream[j].position.z = -0.5f;
        mesh->vertex->stream[j].normal.x = x;
        mesh->vertex->stream[j].normal.y = y;
        mesh->vertex->stream[j].normal.z = 0;
        j++;
    }

    /* Caps */
    for (int i = 0; i < number_points; i++){
        gwFloat x = (gwFloat)(sin( (i * 2 * pi) / number_points ));
        gwFloat y = (gwFloat)(cos( (i * 2 * pi) / number_points ));
        mesh->vertex->stream[j].position.x = (gwFloat)(x * top_base_ratio);
        mesh->vertex->stream[j].position.y = (gwFloat)(y * top_base_ratio);
        mesh->vertex->stream[j].position.z = 0.5f;
        mesh->vertex->stream[j].normal.x = 0;
        mesh->vertex->stream[j].normal.y = 0;
        mesh->vertex->stream[j].normal.z = 1;
        j++;
    }
    for (int i = 0; i < number_points; i++){
        gwFloat x = (gwFloat)(sin( (i * 2 * pi) / number_points ));
        gwFloat y = (gwFloat)(cos( (i * 2 * pi) / number_points ));
        mesh->vertex->stream[j].position.x = (gwFloat)(x);
        mesh->vertex->stream[j].position.y = (gwFloat)(y);
        mesh->vertex->stream[j].position.z = -0.5f;
        mesh->vertex->stream[j].normal.x = 0;
        mesh->vertex->stream[j].normal.y = 0;
        mesh->vertex->stream[j].normal.z = -1;
        j++;
    }
    mesh->vertex->stream[j].position.x = 0;
    mesh->vertex->stream[j].position.y = 0;
    mesh->vertex->stream[j].position.z = 0.5f;
    mesh->vertex->stream[j].normal.x = 0;
    mesh->vertex->stream[j].normal.y = 0;
    mesh->vertex->stream[j].normal.z = -1;
    j++;

    mesh->vertex->stream[j].position.x = 0;
    mesh->vertex->stream[j].position.y = 0;
    mesh->vertex->stream[j].position.z = -0.5f;
    mesh->vertex->stream[j].normal.x = 0;
    mesh->vertex->stream[j].normal.y = 0;
    mesh->vertex->stream[j].normal.z = -1;
    j++;

    /* Build the indexes */
    j = 0;
    for (int i = 0; i < number_points; i++){
        int i0 = i * 2;
        int i1 = i * 2 + 1;
        int i2 = i * 2 + 2;
        int i3 = i * 2 + 3;

        if (i2 >= number_points * 2)
            i2 -= number_points * 2;
        if (i3 >= number_points * 2)
            i3 -= number_points * 2;

        mesh->index->stream[j] = i0; j++;
        mesh->index->stream[j] = i1; j++;
        mesh->index->stream[j] = i2; j++;
        mesh->index->stream[j] = i2; j++;
        mesh->index->stream[j] = i1; j++;
        mesh->index->stream[j] = i3; j++;
    }

    /* Caps */
    for (int i = 0; i < number_points; i++){
        int i0 = number_points * 2 + i;
        int i1 = number_points * 2 + i + 1;
        int in = number_points * 4;

        if (i1 >= number_points * 3)
            i1 -= number_points;

        mesh->index->stream[j] = i0; j++;
        mesh->index->stream[j] = i1; j++;
        mesh->index->stream[j] = in; j++;
    }

    for (int i = 0; i < number_points; i++){
        int i0 = number_points * 3 + i;
        int i1 = number_points * 3 + i + 1;
        int in = number_points * 4 + 1;

        if (i1 >= number_points * 4)
            i1 -= number_points;

        mesh->index->stream[j] = i1; j++;
        mesh->index->stream[j] = i0; j++;
        mesh->index->stream[j] = in; j++;
    }

    return mesh;
}

/******************************************************************************/

