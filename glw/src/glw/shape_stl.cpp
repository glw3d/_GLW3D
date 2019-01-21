/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Loads a geometry from stl file format

*******************************************************************************/

#include "gwdata.h"
#include "glw.h"

#include "common/log.h"
#include "dataset/stl/import_stl.h"
#include "gwmath.h"

extern "C"
gwMesh* gwShapeLoad_stl( const char* filename )
{
    gwSTLMesh* obj = gw_stl_import( filename );
    gwMesh* mesh = gwMesh_create();

    mesh->vertex = gwVertexStream_create( obj->num_triangles * 3 );
    mesh->index = gwIndexStream_create( 0, GW_INDEX_TRIANGLES );

    int iv = 0, ntri = 0, j = 0;
    for (int ntri = 0; ntri < obj->num_triangles; ntri++){
        mesh->vertex->stream[iv].position.x = (gwFloat)obj->vertex[j + 0];
        mesh->vertex->stream[iv].position.y = (gwFloat)obj->vertex[j + 1];
        mesh->vertex->stream[iv].position.z = (gwFloat)obj->vertex[j + 2];

        mesh->vertex->stream[iv].normal.x = (gwFloat)obj->normal[j + 0];
        mesh->vertex->stream[iv].normal.y = (gwFloat)obj->normal[j + 1];
        mesh->vertex->stream[iv].normal.z = (gwFloat)obj->normal[j + 2];
        iv++;

        mesh->vertex->stream[iv].position.y = (gwFloat)obj->vertex[j + 4];
        mesh->vertex->stream[iv].position.z = (gwFloat)obj->vertex[j + 5];
        mesh->vertex->stream[iv].position.x = (gwFloat)obj->vertex[j + 3];

        mesh->vertex->stream[iv].normal.x = (gwFloat)obj->normal[j + 0];
        mesh->vertex->stream[iv].normal.y = (gwFloat)obj->normal[j + 1];
        mesh->vertex->stream[iv].normal.z = (gwFloat)obj->normal[j + 2];
        iv++;

        mesh->vertex->stream[iv].position.x = (gwFloat)obj->vertex[j + 6];
        mesh->vertex->stream[iv].position.y = (gwFloat)obj->vertex[j + 7];
        mesh->vertex->stream[iv].position.z = (gwFloat)obj->vertex[j + 8];

        mesh->vertex->stream[iv].normal.x = (gwFloat)obj->normal[j + 0];
        mesh->vertex->stream[iv].normal.y = (gwFloat)obj->normal[j + 1];
        mesh->vertex->stream[iv].normal.z = (gwFloat)obj->normal[j + 2];
        iv++;

        j += 9;
    }

    mesh->culling = 1;

    return mesh;
}

