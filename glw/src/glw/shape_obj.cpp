/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Loads a geometry from obj file format

*******************************************************************************/

#include "gwdata.h"
#include "glw.h"

#include "common/log.h"
#include "dataset/obj/import_obj.h"
#include "gwmath.h"

/* Extracts the vertex information from the obj data */
static void parse_obj_vertex( gwMesh* mesh, const gwObjGrid* const obj )
{
    mesh->vertex = gwVertexStream_create( obj->num_vertices );
    gwVertex* vertex = mesh->vertex->stream;
    if (vertex == nullptr){
        /* Out of memory */
        return;
    }

    /* Copy the vertex position */
    for (int i = 0; i < obj->num_vertices; i++){
        vertex[i].position.x = obj->position[i * 4];
        vertex[i].position.y = obj->position[i * 4 + 1];
        vertex[i].position.z = obj->position[i * 4 + 2];
        /* the optional parameter w is not used */
    }

    /* Copy the textures */
    if (obj->texture != nullptr){
        if (obj->index_texture == nullptr){
            /* The textures are taken directly (this is the nicest format) */
            if (obj->num_textures != obj->num_vertices){
                /* is the file corrupted? */
                _handle_error_( "Cannot extract textures from obj file" );
            }
            else{
                for (int i = 0; i < obj->num_vertices; i++){
                    vertex[i].texCoord.u = obj->texture[i * 3];
                    vertex[i].texCoord.v = obj->texture[i * 3 + 1];
                    /* the optional parameter w is not used */
                }
            }
        }
        else{
            /* The textures are referenced by an index */
            for (int i = 0; i < obj->num_indexes; i++){
                int ref = obj->index[i] - 1;
                int rtx = obj->index_texture[i] - 1;
                if (ref >= 0 && (size_t)ref < mesh->vertex->length
                    && rtx >= 0 && rtx < obj->num_textures)
                {
                    vertex[ref].texCoord.u = obj->texture[rtx * 3];
                    vertex[ref].texCoord.v = obj->texture[rtx * 3 + 1];
                }
            }
        }
    }
    else{
        /* There is no information of the texture coordinates */
        for (int i = 0; i < obj->num_vertices; i++){
            vertex[i].texCoord.u = 0;
            vertex[i].texCoord.v = 0;
        }
    }

    /* Copy the normals */
    if (obj->normal != nullptr){
        if (obj->index_normal == nullptr){
            /* The normals are taken directly (this is the nicest format) */
            if (obj->num_normals != obj->num_vertices){
                /* is the file in corrupted? */
                _handle_error_( "Cannot extract normals from obj file" );
            }
            else{
                for (int i = 0; i < obj->num_vertices; i++){
                    vertex[i].normal.x = obj->normal[i * 3];
                    vertex[i].normal.y = obj->normal[i * 3 + 1];
                    vertex[i].normal.z = obj->normal[i * 3 + 2];

                    /* check that the normal is normalized */
                    gwFloat imod = gwVector3f_imod( &(vertex[i].normal) );
                    vertex[i].normal.x *= imod;
                    vertex[i].normal.y *= imod;
                    vertex[i].normal.z *= imod;
                }
            }
        }
        else{
            /* The normals are referenced by an index */
            for (int i = 0; i < obj->num_indexes; i++){
                int ref = obj->index[i] - 1;
                int rtx = obj->index_normal[i] - 1;
                if (ref >= 0 && (size_t)ref < mesh->vertex->length
                    && rtx >= 0 && rtx < obj->num_textures)
                {
                    vertex[i].normal.x = obj->normal[rtx * 3];
                    vertex[i].normal.y = obj->normal[rtx * 3 + 1];
                    vertex[i].normal.z = obj->normal[rtx * 3 + 2];

                    /* Check that the normal is normalized */
                    gwFloat imod = gwVector3f_imod( &(vertex[i].normal) );
                    vertex[i].normal.x *= imod;
                    vertex[i].normal.y *= imod;
                    vertex[i].normal.z *= imod;
                }
            }
        }
    }
    else{
        gwMesh_calculateNormals( mesh );
    }
}

/* Extracts the index information from the obj data */
static void parse_obj_index( gwMesh* mesh, const gwObjGrid* const obj )
{
    /**************************************************************************/
    /* In the obj, indexes can be grouped. We create one single object anyway */
    /**************************************************************************/

    mesh->index = gwIndexStream_create( obj->num_indexes, GW_INDEX_TRIANGLES );
    gwIndex* index = mesh->index->stream;
    if (index == nullptr){
        /* Out of memory */
        return;
    }

    /* Copy the faces. I think they are always triangles... */
    for (int i = 0; i < obj->num_indexes; i++){
        index[i] = obj->index[i] - 1;
    }
}

extern "C"
gwMesh* gwShapeLoad_obj( const char* filename )
{
    gwObjGrid* obj = gw_obj_import( filename );
    gwMesh* mesh = gwMesh_create();
    parse_obj_index( mesh, obj );
    parse_obj_vertex( mesh, obj );

    mesh->culling = 0;

    return mesh;
}


