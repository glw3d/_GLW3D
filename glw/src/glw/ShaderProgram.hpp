/*
* Author: Mario J. Martin <dominonurbs$gmail.com>
*
* Class to load and links a generic shader program.
* It is also the base class for more especific shaders.
*/

#ifndef _HGW_SHADERPROGRAM_H
#define _HGW_SHADERPROGRAM_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/definitions.h"
#include "common/check_malloc.h"
#include "common/log.h"

#include "gwgl.h" /* GLchar is defined in glew.h */

namespace gw
{
/* Posssible shader status, after loading and compiling.  */
enum ShaderStatus
{
    SHADER_OK = 0,
    SHADER_LOAD_ERROR = 1,
    SHADER_CREATION_ERROR = 2,
    SHADER_COMPILATION_ERROR = 4,
    SHADER_LINK_ERROR = 8,
    SHADER_VALIDATION_ERROR = 16,
    SHADER_UNLOADED = 32,
    SHADER_OTHER_ERROR = 256
};

/* Class for basic shader programs. This class only loads and links.
* Attributes and uniforms are usually especified in an inherited object */
class ShaderProgram
{
public:
    /* Id of the vertex program */
    GLuint vertex_shader;

    /* Id of the geometric shader. (Not available in OpengL ES 2.0)*/
    GLuint geometry_shader;

    /* Id of the fragment program */
    GLuint fragment_shader;

    /* Id of the shader program */
    GLuint program;

    /* Lines with the vertex source code */
    GLchar** vertex_source;

    /* Number of lines of the vertex source code */
    GLsizei vertex_count;

    /* Lines with the geometric source code */
    GLchar** geometry_source;

    /* Number of lines of the geometric source code */
    GLsizei geometry_count;

    /* Lines with the fragment source code */
    GLchar** fragment_source;

    /* Number of lines of the fragment source code */
    GLsizei fragment_count;

    /* Max number of expected characters in a line of the source code */
    static const int MAX_LINE_LENGTH = 256;

public:
    /* Default constructor */
    ShaderProgram()
    {
        vertex_shader = 0;
        geometry_shader = 0;
        fragment_shader = 0;
        program = 0;
        vertex_count = 0;
        geometry_count = 0;
        fragment_count = 0;

        vertex_source = nullptr;
        geometry_source = nullptr;
        fragment_source = nullptr;
    }

    /* Destructor */
    ~ShaderProgram()
    {
        if (program != 0){
            _warning_( "Shader program %i is not deleted before destructor call", program );
            program = 0;
        }
        if (vertex_shader != 0){
            _warning_( "Vertex shader %i is not deleted before destructor call", vertex_shader );
            vertex_shader = 0;
        }
        if (geometry_shader != 0){
            _warning_( "Geometric shader %i is not deleted before destructor call", geometry_shader );
            geometry_shader = 0;
        }
        if (fragment_shader != 0){
            _warning_( "Fragment shader %i is not deleted before destructor call", fragment_shader );
            fragment_shader = 0;
        }
    }

private:
    /* Loads a source code from a file. Returns the pointer to the source. */
    GLchar** loadSource( GLsizei* count, const char* filename )
    {
        char line[MAX_LINE_LENGTH];
        FILE* fd = NULL;
        int length = 0, line_length;
        GLchar** source;

        *count = 0;
        fd = fopen( filename, "r" );
        if (fd == NULL){
            _handle_error_( "Cannot open file: %s", filename );
            return nullptr;
        }

        // first get the total length of the code
        while (fgets( line, MAX_LINE_LENGTH, fd ) != NULL){
            line_length = strlen( line );
            if (line_length > MAX_LINE_LENGTH){
                _handle_error_
                    ( "Cannot read lines longer that %i characters in file: %s"
                    , MAX_LINE_LENGTH, filename 
                    );
                *count = 0;
                fclose( fd );
                return nullptr;
            }
            length += line_length + 1;
            (*count)++;
        }

        rewind( fd );
        source = (GLchar**)malloc( sizeof( GLchar* ) * (*count) );
        if (source == nullptr){
            _handle_error_( "Out of memory!" );
            return nullptr;
        }

        length = 0;
        int i = 0;
        while (fgets( line, MAX_LINE_LENGTH, fd ) != NULL){
            line_length = strlen( line );
            source[i] = (GLchar*)malloc( sizeof( GLchar ) * (line_length + 1) );
            if (source[i] == nullptr){
                _handle_error_( "Out of memory!" );
                return nullptr;
            }

            GLchar* ps = source[i];
            GLchar* pl = line;

            // concatenates the string
            while (*pl != '\0'){
                *ps++ = *pl++;
            }
            *ps++ = '\0';
            i++;
        }

        fclose( fd );
        return source;
    }


    /* Loads and compiles the vertex shader. Returns SHADER_OK on success */
#if GL_VERSION_2_0
    ShaderStatus loadVertex( const char* filename )
    {
        if (filename == nullptr){
            _handle_error_( "File not found %s", filename );
            return SHADER_LOAD_ERROR;
        }

        /* Loads the source from a file */
        vertex_source = loadSource( &vertex_count, filename );
        if (vertex_source == nullptr){
            return SHADER_LOAD_ERROR;
        }

        /* Creates a vertex source */
        vertex_shader = glCreateShader( GL_VERTEX_SHADER );
        if (vertex_shader == 0){
            _handle_error_( "Cannot create shader" );
            return SHADER_CREATION_ERROR;
        }

        /* Creates a shader object */
        glShaderSource( vertex_shader, vertex_count
            , (const GLchar**)vertex_source, NULL );

        /* Compile the source */
        glCompileShader( vertex_shader );

        /* Check compilation success */
        GLint status = GL_FALSE;
        glGetShaderiv( vertex_shader, GL_COMPILE_STATUS, &status );

        /* Check the log */
        int logLength = 1;
        glGetShaderiv( vertex_shader, GL_INFO_LOG_LENGTH, &logLength );

        /* The maxLength should already includes the null character */
        char* infoLog = (char*)malloc( logLength + 1 );
        infoLog[0] = '\0';

        glGetShaderInfoLog( vertex_shader, logLength, &logLength, infoLog );

        if (status != GL_TRUE){
            _handle_error_( "Failed to compile vertex shader %s : %s"
                , filename, infoLog );
            free( infoLog );
            return SHADER_COMPILATION_ERROR;
        }
        else{
            free( infoLog );
            return SHADER_OK;
        }
    }

    /* Loads and compiles the fragment shader. Returns SHADER_OK if success */
    ShaderStatus loadFragment( const char* filename )
    {
        if (filename == nullptr){
            return SHADER_LOAD_ERROR;
        }

        /* Loads the source from a file */
        fragment_source = loadSource( &fragment_count, filename );
        if (fragment_source == nullptr){
            return SHADER_LOAD_ERROR;
        }

        /* Creates a fragment source */
        fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
        if (fragment_shader == 0){
            _handle_error_( "Cannot create shader" );
            return SHADER_CREATION_ERROR;
        }

        /* Creates a shader object */
        glShaderSource( fragment_shader, fragment_count
            , (const GLchar**)fragment_source, NULL );

        /* Compile the source */
        glCompileShader( fragment_shader );

        /* Check compilation success */
        GLint status = GL_FALSE;
        glGetShaderiv( fragment_shader, GL_COMPILE_STATUS, &status );

        /* Check the log */
        int logLength = 1;
        glGetShaderiv( fragment_shader, GL_INFO_LOG_LENGTH, &logLength );

        /* The maxLength should already includes the null character */
        char* infoLog = (char*)malloc( logLength + 1 );
        infoLog[0] = '\0';

        glGetShaderInfoLog( fragment_shader, logLength, &logLength, infoLog );
        
        if (status != GL_TRUE){
            _handle_error_( "Failed to compile fragment shader %s: %s"
                , filename, infoLog );
            free( infoLog );
            return SHADER_COMPILATION_ERROR;
        }
        else{
            free( infoLog );
            return SHADER_OK;
        }
    }

#else
    ShaderStatus loadVertex( const char* filename )
    {
        _handle_error_( "Shaders are only supported since OpenGL 2.x" )
    }

    ShaderStatus loadFragment( const char* filename )
    {
        _handle_error_( "Shaders are only supported since OpenGL 2.x" )
    }
#endif

#if GL_VERSION_3_2
    /* Loads and compiles the vertex shader. Returns SHADER_OK if success */
    ShaderStatus loadGeometry( const char* filename )
    {
        if (filename == nullptr){
            _handle_error_( "File not found %s", filename );
            return SHADER_LOAD_ERROR;
        }

        /* Loads the source from a file */
        geometry_source = loadSource( &geometry_count, filename );
        if (geometry_source == nullptr){
            _handle_error_( "Cannot open file %s", filename );
            return SHADER_LOAD_ERROR;
        }

        /* Creates a vertex source */
        geometry_shader = glCreateShader( GL_GEOMETRY_SHADER );
        if (geometry_shader == 0){
            _handle_error_( "Cannot create shader" );
            return SHADER_CREATION_ERROR;
        }

        /* Creates a shader object */
        glShaderSource( geometry_shader, geometry_count
            , (const GLchar**)geometry_source, NULL );

        /* Compile the source */
        glCompileShader( geometry_shader );

        /* Check compilation success */
        GLint status = GL_FALSE;
        glGetShaderiv( geometry_shader, GL_COMPILE_STATUS, &status );

        /* Check the log */
        int logLength = 1;
        glGetShaderiv( geometry_shader, GL_INFO_LOG_LENGTH, &logLength );

        /* The maxLength should already includes the null character */
        char* infoLog = (char*)malloc( logLength + 1 );
        infoLog[0] = '\0';

        glGetShaderInfoLog( geometry_shader, logLength, &logLength, infoLog );

        if (status != GL_TRUE){
            _handle_error_( "Failed to compile geometry shader %s: %s"
                , filename, infoLog );
            free( infoLog );
            return SHADER_COMPILATION_ERROR;
        }
        else{
            _trace_( "Geometry shader %s loaded: %s\n", filename, infoLog );
            free( infoLog );
            return SHADER_OK;
        }
    }
#else
    ShaderStatus loadGeometry( const char* filename )
    {
        _handle_error_( "OpenGL 3 is required for geometry shaders" );
        return SHADER_LOAD_ERROR;
    }
#endif

public:
    /* Releases sources */
    void unloadSources()
    {
        if (vertex_source != nullptr){
            for (int i = 0; i < vertex_count; i++){
                if (vertex_source[i] != nullptr){
                    free( vertex_source[i] );
                }
            }
            vertex_count = 0;
            free( vertex_source );
            vertex_source = nullptr;
        }
        if (geometry_source != nullptr){
            for (int i = 0; i < geometry_count; i++){
                if (geometry_source[i] != nullptr){
                    free( geometry_source[i] );
                }
            }
            geometry_count = 0;
            free( geometry_source );
            geometry_source = nullptr;
        }
        if (fragment_source != nullptr){
            for (int i = 0; i < fragment_count; i++){
                if (fragment_source[i] != nullptr){
                    free( fragment_source[i] );
                }
            }
            fragment_count = 0;
            free( fragment_source );
            fragment_source = nullptr;
        }
    }

    /* Releases resources */
    void dispose()
    {
        if (vertex_shader != 0){
            glDeleteShader( vertex_shader );
            if (program != 0){
                glDetachShader( program, vertex_shader );
            }
            vertex_shader = 0;
        }
        if (geometry_shader != 0){
            glDeleteShader( geometry_shader );
            if (program != 0){
                glDetachShader( program, geometry_shader );
            }
            geometry_shader = 0;
        }
        if (fragment_shader != 0){
            glDeleteShader( fragment_shader );
            if (program != 0){
                glDetachShader( program, fragment_shader );
            }
            fragment_shader = 0;
        }
        if (program != 0){
            /* Release the program */
            glDeleteProgram( program );
            program = 0;
        }

        /* Delete the sources */
        unloadSources();
    }


    /* Loads and compiles a shader. */
    int load
        ( const char* vertex_filename
        , const char* fragment_filename
        , const char* geometry_filename
        )
    {
        /* Load and compile the sources  */
        int status = ShaderProgram::compile
            ( vertex_filename, fragment_filename, geometry_filename );
        if (status != SHADER_OK){
            return status;
        }

        /* Link the shader */
        status = ShaderProgram::link();
        if (status != SHADER_OK){
            return status;
        }

        return SHADER_OK;
    }


    /* Unloads (deletes) the shader program in the GPU */
    void unload()
    {
        if (program != 0){
            /* Release the program */
            glDeleteProgram( program );
            program = 0;
        }
        if (vertex_shader != 0){
            /* Release the vertex shader.
            * If the vertex source is no nullptr,
            * we know that the vertex was generated here */
            glDeleteShader( vertex_shader );
            vertex_shader = 0;
        }
        if (geometry_shader != 0){
            /* Release the geometry shader.
            * If the geometry source is no nullptr,
            * we know that the vertex was generated here */
            glDeleteShader( geometry_shader );
            geometry_shader = 0;
        }
        if (fragment_shader != 0){
            /* Release the fragment shader.
            * If the fragment source is no nullptr,
            * we know that the fragment was generated here */
            glDeleteShader( fragment_shader );
            fragment_shader = 0;
        }
    }

    /* Loads, compiles and creates a shader program. Then it must be linked. */
    int compile
        ( const char* vertex_filename
        , const char* fragment_filename
        , const char* geometry_filename
        )
    {
        char* path_vertex = nullptr;
        char* path_fragment = nullptr;
        char* path_geometric = nullptr;

        ShaderStatus status_vertex = SHADER_UNLOADED;
        ShaderStatus status_geometric = SHADER_UNLOADED;
        ShaderStatus status_fragment = SHADER_UNLOADED;

        /* Unload the shader if neccesary */
        unload();

        if (vertex_filename == nullptr || vertex_filename[0] == '\0'){
            _handle_error_( "Must define the vertex source filename" );
            return SHADER_LOAD_ERROR;
        }

        if (fragment_filename == nullptr || fragment_filename[0] == '\0'){
            _handle_error_( "Must define the fragment source filename" );
            return SHADER_LOAD_ERROR;
        }

        /* Loads and compiles the vertex source */
        status_vertex = loadVertex( vertex_filename );

        /* Loads and compiles the fragment source */
        status_fragment = loadFragment( fragment_filename );

        if (geometry_filename != nullptr && geometry_filename[0] != '\0'){
            /* Loads and compiles the geometric source */
            status_geometric = loadGeometry( geometry_filename );
        }
        else{
            status_geometric = SHADER_OK;
        }
        
        if (status_vertex == SHADER_OK && status_fragment == SHADER_OK){
            /* The vertex and fragment are always neccesary to link a program */
            program = glCreateProgram();
        }
        
        int shader_status = status_vertex | status_geometric | status_fragment;

        /* Check errors */
        GLenum er = glGetError();
        if (er != GL_NO_ERROR){
            _handle_error_( "OpenGL - error = %i", er );
        }
        
        /* The sources are not longer neccesary */
        unloadSources();

        /* Anything different than 0 means some error in the process */
        return shader_status;
    }

    /* Links the program using the vertex and fragment sources */
    ShaderStatus link()
    {
        GLenum er;

        if (fragment_shader == 0 || vertex_shader == 0){
            /* No programs are compiled */
            return SHADER_UNLOADED;
        }

        if (program == 0){
            _handle_error_( "Cannot create shader program" );
            return SHADER_CREATION_ERROR;
        }

        /* Attach the vertex and fragment shader codes,
        and the geometric if there is one */
        glAttachShader( program, vertex_shader );
        if (geometry_shader != 0){
            glAttachShader( program, geometry_shader );
        }
        glAttachShader( program, fragment_shader );

        glLinkProgram( program );

        /* Check link success */
        GLint status = GL_FALSE;
        glGetProgramiv( program, GL_LINK_STATUS, &status );

        if (status == GL_FALSE){
            /* Check the log */
            int logLen = 1;
            glGetShaderiv( program, GL_INFO_LOG_LENGTH, &logLen );

            /* The maxLength should includes the null character */
            int maxLength = logLen;
            char* infoLog = (char*)calloc( sizeof( char ), maxLength + 1 );
            glGetShaderInfoLog( program, maxLength, &logLen, infoLog );
            _trace_( "Error in the shader. Shader program log: %s\n", infoLog );
            free( infoLog );
        }

#if 0
        /* MAC gives an error: "no vertex array bound" */
        /* Check if the shader will run in the current OpenGL state */
        glValidateProgram( program );

        // Check the status of the compile/link
        glGetProgramiv( program, GL_VALIDATE_STATUS, &status );
        if (status != GL_TRUE){
            _handle_error_( "Failed to validate the shader" );
            glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLen );
            if (logLen > 1){
                // Show any errors as appropriate
                int maxLength = logLen;
                char* infoLog = (char*)calloc( sizeof( char ), maxLength + 1 );
                glGetProgramInfoLog( program, logLen, &logLen, infoLog );
                _trace_( "Shader program log: %s\n", infoLog );
                free( infoLog );
            }
            return SHADER_VALIDATION_ERROR;
        }
#endif

        /* Check errors */
        er = glGetError();
        if (er != GL_NO_ERROR){
            _handle_error_( "OpenGL - error = %i", er );
            return SHADER_LINK_ERROR;
        }

        /* Release resources that are no longer needed */
        if (vertex_shader != 0){
            glDeleteShader( vertex_shader );
        }
        if (geometry_shader != 0){
            glDeleteShader( geometry_shader );
        }
        if (fragment_shader != 0){
            glDeleteShader( fragment_shader );
        }

        return SHADER_OK;
    }
};

} //end namespace

#endif  /*_HGW_SHADERPROGRAM_H*/

/**/
