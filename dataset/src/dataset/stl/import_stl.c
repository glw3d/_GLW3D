/*
 * Author: Mario J. Martin <dominonurbs$gmail.com>
 *
 * A routine that loads the models in .stl file format
 *

 STL files should have an structure like
 solid <optional name>
  facet normal -9.770496e-001  3.037601e-006  2.130122e-001
    outer loop
      vertex -2.220173e+002 -1.730550e+000  1.681795e+003
      vertex -2.220147e+002  1.037158e+000  1.681807e+003
      vertex -2.220402e+002 -4.496186e+000  1.681690e+003
    endloop
  endfacet
 endsolid <optional name>

 The format allow the definition of quads and polygons, but in practice 
 only triangles are used.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "import_stl.h"

#include "common/definitions.h"
#include "common/check_malloc.h"
#include "common/log.h"

char* keywords[] =
{ "solid"
, "facet normal"
, "outer loop"
, "vertex"
, "endloop"
, "endfacet"
, "endsolid"
, "\0"
};

#define KEY_SOLID       0
#define KEY_FACET       1
#define KEY_OUTER_LOOP  2
#define KEY_VERTEX      3
#define KEY_ENDLOOP     4
#define KEY_ENDFACET    5
#define KEY_ENDSOLID    6

/* Initializes the data */
static gwSTLMesh* gw_stl_init( gwSTLMesh* p_stl )
{
    gwSTLMesh* stl = p_stl;
    if (stl == nullptr){
        stl = (gwSTLMesh*)malloc( sizeof( gwSTLMesh ) );
    }
    stl->next = nullptr;
    stl->num_triangles = 0;
    stl->vertex = nullptr;
    stl->normal = nullptr;
    stl->name[0] = '\0';

    return stl;
}

/* Trims all spaces at the beginning of the string */
inline static char* str_left_trim( char* str )
{
    char* out_ptr = &str[0];

    for (; *out_ptr == ' ' || *out_ptr == '\t'; out_ptr++);

    return out_ptr;
}

/* Trims all spaces at the beginning of the string */
inline static char* str_trim( char* str )
{
    char* begin = &str[0];
    char* end = &str[strlen(str)];

    for (; (begin < end) 
        && (*end == '\0' || *end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')
        ; end--);
    if (*end != '\0'){
        *(end+1) = '\0';
    }

    for (; (begin <= end) 
        && (*begin == ' ' || *begin == '\t' || *begin == '\n' || *begin == '\r')
        ; begin++);

    return begin;
}

inline static char* check_keyword( char* line, const char* keyword )
{
    const char* k = &keyword[0];
    char* s = &line[0];

    while (*k != '\0'){
        if (*k != *s) return nullptr;
        k++;
        s++;
    }

    return s;
}

static char* get_next_keyword
( char* buffer, int* line_number, int* ikey
, FILE* file_handle, const char* keywords[], const int buffer_length)
{
    int i;
    char* value = nullptr;
    const char* key = nullptr;
    while (fgets( buffer, buffer_length, file_handle ) != NULL){
        *line_number += 1;
        buffer = str_left_trim( buffer );
        i = 0;
        key = keywords[i];
        while (key[0] != '\0'){
            value = check_keyword( buffer, key );
            if (value != nullptr) {
                *ikey = i;
                return value;
            }
            i++;
            key = keywords[i];
        }
    }

    *ikey = -1;
    return nullptr;
}

/* First pass is to know the number of facets */
static int first_pass( FILE* file_handle, gwSTLMesh* data )
{
    const char* key_solid[] = { "solid", "\0" };
    const char* key_facet[] = { "facet normal", "\0" };
    int ikey = -1;
    char buffer[256];
    int line_number = 0;
    int nfacets = 0;
    char* facet = nullptr;

    /* look for the keyword 'solid' */
    char* name = get_next_keyword( buffer, &line_number, &ikey, file_handle, key_solid, 256 );
    if (name == nullptr){
        _handle_error_( "Not a STL file!" );
        return 0;
    }
    name = str_trim( name );

    /* Count the keywords 'facet normal' */
    facet = get_next_keyword( buffer, &line_number, &ikey, file_handle, key_facet, 256 );
    while (facet != nullptr){
        nfacets++;
        facet = get_next_keyword( buffer, &line_number, &ikey, file_handle, key_facet, 256 );
    }

    return nfacets;
}

/* The second pass add the vertex information */
static void second_pass( FILE* file_handle, gwSTLMesh* data )
{
    char buffer[256];
    int ikey = -1;
    int line_number = 0;
    int n = 0, nfacets = 0;
    char* facet = nullptr;
    double x, y, z;

    /* look for the keyword 'solid' */
    char* line = get_next_keyword( buffer, &line_number, &ikey, file_handle, keywords, 256 );
    if (line == nullptr || ikey != KEY_SOLID){
        _handle_error_( "Expected %s keyword in line %i", keywords[KEY_SOLID], line_number );
        return;
    }
    line = str_trim( line );
    strcpy( data->name, line );

    /* Get the vertex */
    while (line != nullptr && nfacets < data->num_triangles)
    {
        line = get_next_keyword( buffer, &line_number, &ikey, file_handle, keywords, 256 );
        if (ikey == KEY_ENDSOLID){
            return;
        }

        if (ikey != KEY_FACET){
            _handle_error_( "Expected %s keyword in line %i", keywords[KEY_FACET], line_number );
            return;
        }

        /* Read the normal */
        if (sscanf( line, "%lf %lf %lf", &x, &y, &z ) != 3){
            _handle_error_( "Error in line %i", line_number );
            return;
        }
        data->normal[n + 0] = x;
        data->normal[n + 1] = y;
        data->normal[n + 2] = z;

        data->normal[n + 3] = x;
        data->normal[n + 4] = y;
        data->normal[n + 5] = z;

        data->normal[n + 6] = x;
        data->normal[n + 7] = y;
        data->normal[n + 8] = z;

        nfacets++;

        /* Read the vertices */
        line = get_next_keyword( buffer, &line_number, &ikey, file_handle, keywords, 256 );
        if (ikey != KEY_OUTER_LOOP){
            _handle_error_( "Expected %s keyword in line %i", keywords[KEY_OUTER_LOOP], line_number );
            return;
        }

        /* First vertex */
        line = get_next_keyword( buffer, &line_number, &ikey, file_handle, keywords, 256 );
        if (ikey != KEY_VERTEX){
            _handle_error_( "Expected %s keyword in line %i", keywords[KEY_VERTEX], line_number );
            return;
        }
        if (sscanf( line, "%lf %lf %lf", &x, &y, &z ) != 3){
            _handle_error_( "Error in line %i", line_number );
            return;
        }
        data->vertex[n + 0] = x;
        data->vertex[n + 1] = y;
        data->vertex[n + 2] = z;

        /* Second vertex */
        line = get_next_keyword( buffer, &line_number, &ikey, file_handle, keywords, 256 );
        if (ikey != KEY_VERTEX){
            _handle_error_( "Expected %s keyword in line %i", keywords[KEY_VERTEX], line_number );
            return;
        }
        if (sscanf( line, "%lf %lf %lf", &x, &y, &z ) != 3){
            _handle_error_( "Error in line %i", line_number );
            return;
        }
        data->vertex[n + 3] = x;
        data->vertex[n + 4] = y;
        data->vertex[n + 5] = z;

        /* Third vertex */
        line = get_next_keyword( buffer, &line_number, &ikey, file_handle, keywords, 256 );
        if (ikey != KEY_VERTEX){
            _handle_error_( "Expected %s keyword in line %i", keywords[KEY_VERTEX], line_number );
            return;
        }
        if (sscanf( line, "%lf %lf %lf", &x, &y, &z ) != 3){
            _handle_error_( "Error in line %i", line_number );
            return;
        }
        data->vertex[n + 6] = x;
        data->vertex[n + 7] = y;
        data->vertex[n + 8] = z;

        n += 9;

        line = get_next_keyword( buffer, &line_number, &ikey, file_handle, keywords, 256 );
        if (ikey != KEY_ENDLOOP){
            _handle_error_( "Expected %s keyword in line %i", keywords[KEY_ENDLOOP], line_number );
            return;
        }
        line = get_next_keyword( buffer, &line_number, &ikey, file_handle, keywords, 256 );
        if (ikey != KEY_ENDFACET){
            _handle_error_( "Expected %s keyword in line %i", keywords[KEY_ENDFACET], line_number );
            return;
        }
    }
}

gwSTLMesh* gw_stl_import( const char* filename )
{
    FILE *file_handler;
    gwSTLMesh* data = nullptr;
    int nfacets = 0;

	file_handler = fopen( filename, "rt" );
	if(file_handler == NULL){
		_handle_error_("Cannot open filename %s", filename);
        return data;
	}

    nfacets = first_pass( file_handler, data );
    if (nfacets == 0){
        return nullptr;
    }

    /* Reserve memory */
    data = gw_stl_init( nullptr );
    data->num_triangles = nfacets;
    _check_( data->normal = (double*)_calloc_( 9 * nfacets, sizeof( double ) ) );
    _check_( data->vertex = (double*)_calloc_( 9 * nfacets, sizeof( double ) ) );

    rewind( file_handler );
    second_pass( file_handler, data );

    fclose( file_handler );

    return data;
}

