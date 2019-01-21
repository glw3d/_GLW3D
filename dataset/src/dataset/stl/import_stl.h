/* 
 * Author: Mario J. Martin <dominonurbs$gmail.com>
 *
 * A routine that loads geometries in stl file format
 * This format is very common for CFD for its simplicity
 * However, for rendering is limited and not very efficient.
 * It lacks texture coordinates and index conectivities. 
 * Normals are defined on facets instead of vertices.
 *
 */


#ifndef _HMG_IMPORT_STL_H
#define _HMG_IMPORT_STL_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _gwSTLMesh
{
    struct _gwSTLMesh* next; /* pointer to the next object in the stack */
    int num_triangles; /* number of triangles  */
    double* vertex;    /* Three component coordinates */
    double* normal;    /* Normal to the element surface */
    char name[256];    /* name of the entity */

} gwSTLMesh;

gwSTLMesh* gw_stl_import( const char* filename );

#ifdef	__cplusplus
} 
#endif

#endif 

