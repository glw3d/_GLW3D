/***
Author: Mario J. Martin <dominonurbs$gmail.com>

In shaders data is transferred through vertex attributes and uniform variables.
Each attribute or uniform is associate which a corresponding definition in the
shader program. If an attribute or uniform is not in the shader program its
location is -1

This shader pattern works with the vertex format
{(GLfloat)position[3], (GLfloat)normal[3], (GLfloat)texCoord[2]}

Additional streams that might also have are scalar and vector.

*******************************************************************************/


#ifndef _HGW_SHADER_GENERIC_H
#define _HGW_SHADER_GENERIC_H

#include "common/string_ext.h" 

#include "gwdata.h" 
#include "gwmath.h" 
#include "signatures.h" 
#include "ShaderProgram.hpp"

namespace gw
{

#define ATT0_POSITION 0
#define ATT1_NORMAL   1
#define ATT2_TEXCOORD 2
#define ATT3_SCALAR   3
#define ATT4_VECTOR   4

    class ShaderGeneric : public gwShader
    {
    public:
        int signature;
        int mask;

        /* Loads and links the shader */
        ShaderProgram shader;

        char path[GW_LABEL_LENGTH];
        char vertex_filename[GW_LABEL_LENGTH];
        char fragment_filename[GW_LABEL_LENGTH];
        char geometric_filename[GW_LABEL_LENGTH];

        /* Vertex attributes */
        GLint attPosition;
        GLint attNormal;
        GLint attTexCoord;
        GLint attScalar;
        GLint attVector;

        /* Primary texture location */
        GLint unifTexture0;
        
        /* Secondary texture location */
        GLint unifTexture1;

        /* Depth buffer */
        GLint unifDepthBuffer;

        /* Size of the screen in pixels */
        GLint unifScreenSize;

        /* Transformation matrix */
        GLint unifModelViewProjMatrix;

        /* Camera matrix */
        GLint unifViewMatrix;

        /* Model matrix */
        GLint unifModelMatrix;

        /* Primary color */
        GLint unifPrimaryColor;

        /* Secondary color */
        GLint unifSecondaryColor;

        /* Ambient light color filter */
        GLint unifAmbientLight;

        /* Direction of the light */
        GLint unifLightDirection;

        /* The strength the primary color blends with the texture.
        * A value of 0 means that the primary color is ignored. 
        * However, this convention depends especifically on the shader */
        GLint unifTextureBlend;

        /* Indicates the sign of the normals.
        * If culling is 0, the object has both sides */
        GLint unifCulling;

        /* Unique id that identifies the entity for picking techniques */
        GLint unifPickingId;

        /* Used to normalize the scalar streams  */
        GLint unifScalarRange;

        /* Set to 1 to anulate lighting */
        GLint unifSelfIlumination;

        ShaderGeneric()
        {
            signature = _GW_SIGNATURE_SHADER;
            mask = 0;

            f_gl1x = nullptr;
            label[0] = '\0';
            path[0] = '\0';
            vertex_filename[0] = '\0';
            fragment_filename[0] = '\0';
            geometric_filename[0] = '\0';

            update = 0;
            target = GW_TARGET_SCREEN;
            polygon = GW_POLYGON_NONE;
            alpha_test = 1;
            depth_test = 1;
            ambient_light = gwColorWhite;
            ambient_light.a = 60;
            light_direction.x = 1;
            light_direction.y = 1;
            light_direction.z = -1;
            multipass = nullptr;

            attPosition = -1;
            attNormal = -1;
            attTexCoord = -1;
            attScalar = -1;
            attVector = -1;

            unifTexture0 = -1;
            unifTexture1 = -1;
            unifDepthBuffer = -1;
            unifScreenSize = -1;
            unifModelViewProjMatrix = -1;
            unifViewMatrix = -1;
            unifModelMatrix = -1;
            unifPrimaryColor = -1;
            unifSecondaryColor = -1;
            unifAmbientLight = -1;
            unifTextureBlend = -1;
            unifCulling = -1;
            unifPickingId = -1;
            unifScalarRange = -1;
            unifSelfIlumination = -1;
        }

        void setScreenSize( const GLfloat width, const GLfloat height ) const
        {
            if (unifScreenSize > 0){
                glUniform2f( unifScreenSize, width, height );
            }
        }

        void setModelViewProjMatrix( const GLfloat matrix[16] ) const
        {
            if (unifModelViewProjMatrix >= 0){
                glUniformMatrix4fv( unifModelViewProjMatrix, 1, GL_FALSE, matrix );
            }
        }

        void setViewMatrix( const GLfloat matrix[16] ) const
        {
            if (unifViewMatrix >= 0){
                glUniformMatrix4fv( unifViewMatrix, 1, GL_FALSE, matrix );
            }
        }

        void setModelMatrix( const GLfloat matrix[16] ) const
        {
            if (unifModelMatrix >= 0){
                glUniformMatrix4fv( unifModelMatrix, 1, GL_FALSE, matrix );
            }
        }

        void setPrimaryColor( const gwColor color ) const
        {
            if (unifPrimaryColor >= 0){
                GLfloat b = GLfloat( color.b ) / 255;
                GLfloat g = GLfloat( color.g ) / 255;
                GLfloat r = GLfloat( color.r ) / 255;
                GLfloat a = GLfloat( color.a ) / 255;

                glUniform4f( unifPrimaryColor, r, g, b, a );
            }
        }

        void setSecondaryColor( const gwColor color ) const
        {
            if (unifSecondaryColor >= 0){
                GLfloat b = GLfloat( color.b ) / 255;
                GLfloat g = GLfloat( color.g ) / 255;
                GLfloat r = GLfloat( color.r ) / 255;
                GLfloat a = GLfloat( color.a ) / 255;

                glUniform4f( unifSecondaryColor, r, g, b, a );
            }
        }

        void setAmbientLight( const gwColor color ) const
        {
            if (unifAmbientLight >= 0){
                GLfloat b = GLfloat( color.b ) / 255;
                GLfloat g = GLfloat( color.g ) / 255;
                GLfloat r = GLfloat( color.r ) / 255;
                GLfloat a = GLfloat( color.a ) / 255;

                glUniform4f( unifAmbientLight, r, g, b, a );
            }
        }

        void setLightDirection( const gwVector3f ld ) const
        {
            if (unifLightDirection >= 0){
                glUniform3f( unifLightDirection, ld.x, ld.y, ld.z );
            }
        }

        void setTextureBlend( const gwFloat blend ) const
        {
            if (unifTextureBlend >= 0){
                glUniform1f( unifTextureBlend, blend );
            }
        }

        void setCulling( const int culling ) const
        {
            if (unifCulling >= 0){
                glUniform1i( unifCulling, culling );
            }
        }

        void setPickingId( const int id ) const
        {
            if (unifPickingId >= 0){
                glUniform1i( unifPickingId, id );
            }
        }

        void setScalarRange( const gwFloat min, const gwFloat max ) const
        {
            if (unifScalarRange >= 0){
                glUniform2f( unifScalarRange, min, max );
            }
        }

        void setSelfIlumination( const gwFloat value ) const
        {
            if (unifSelfIlumination >= 0){
                glUniform1f( unifSelfIlumination, value );
            }
        }

        /* Loads and links the shader program */
        int load
            ( const char* shaders_path
            , const char* vertex_filename
            , const char* fragment_filename
            , const char* geometry_filename
            )
        {
            char* path_vertex = nullptr;
            char* path_fragment = nullptr;
            char* path_geometry = nullptr;

            if (vertex_filename == nullptr || vertex_filename[0] == '\0'){
                _handle_error_( "The vertex shader is required!" )
                    return gw::SHADER_LOAD_ERROR;
            }
            if (fragment_filename == nullptr || fragment_filename[0] == '\0'){
                _handle_error_( "The fragment shader is required!" )
                    return gw::SHADER_LOAD_ERROR;
            }

            if (shaders_path != nullptr){
                int len;

                /* Compose the vertex shader file location */
                len = strlen( shaders_path ) + strlen( vertex_filename ) + 2;
                _check_( path_vertex = (char*)_calloc_( len, sizeof( char ) ) );
                strcpy( path_vertex, shaders_path );
                strcat( path_vertex, "/" );
                strcat( path_vertex, vertex_filename );

                /* Compose the fragment shader file location */
                len = strlen( shaders_path ) + strlen( fragment_filename ) + 2;
                _check_( path_fragment = (char*)_calloc_( len, sizeof( char ) ) );
                strcpy( path_fragment, shaders_path );
                strcat( path_fragment, "/" );
                strcat( path_fragment, fragment_filename );

                /* Compose the geometric shader file location */
                if (geometry_filename != nullptr && strlen( geometry_filename ) > 0){
                    len = strlen( shaders_path ) + strlen( geometry_filename ) + 2;
                    _check_( path_geometry = (char*)_calloc_( len, sizeof( char ) ) );
                    strcpy( path_geometry, shaders_path );
                    strcat( path_geometry, "/" );
                    strcat( path_geometry, geometry_filename );
                }
            }
            else{
                int len;

                len = strlen( vertex_filename ) + 1;
                _check_( path_vertex = (char*)_calloc_( len, sizeof( char ) ) );
                strcpy( path_vertex, vertex_filename );

                len = strlen( fragment_filename ) + 1;
                _check_( path_fragment = (char*)_calloc_( len, sizeof( char ) ) );
                strcpy( path_fragment, fragment_filename );

                if (geometry_filename != nullptr){
                    len = strlen( geometry_filename ) + 1;
                    _check_( path_geometry = (char*)_calloc_( len, sizeof( char ) ) );
                    strcpy( path_geometry, geometry_filename );
                }
            }

            int status;

            /* load and compile the sources */
            status = shader.compile( path_vertex, path_fragment, path_geometry );
            if (status != gw::SHADER_OK){
                free( path_vertex );
                free( path_fragment );
                free( path_geometry );
                return status;
            }
            /* Check errors */
            GLenum er = glGetError();
            if (er != GL_NO_ERROR){
                _handle_error_( "OpenGL - error = %i", er );
            }
            /* Forces the attrib location, prior to link. */
            glBindAttribLocation( shader.program, ATT0_POSITION, "in_Position" );
            glBindAttribLocation( shader.program, ATT1_NORMAL, "in_Normal" );
            glBindAttribLocation( shader.program, ATT2_TEXCOORD, "in_TexCoord" );
            glBindAttribLocation( shader.program, ATT3_SCALAR, "in_Scalar" );
            glBindAttribLocation( shader.program, ATT4_VECTOR, "in_Vector" );

            // link the shader
            status = shader.link();
            if (status != gw::SHADER_OK){
                return status;
            }

            unifTexture0 = glGetUniformLocation( shader.program, "Texture0" );
            unifTexture1 = glGetUniformLocation( shader.program, "Texture1" );
            unifDepthBuffer = glGetUniformLocation( shader.program, "DepthBuffer" );

            unifScreenSize = glGetUniformLocation( shader.program, "ScreenSize" );
            unifModelViewProjMatrix
                = glGetUniformLocation( shader.program, "ModelViewProjMatrix" );
            unifViewMatrix
                = glGetUniformLocation( shader.program, "ViewMatrix" );
            unifModelMatrix
                = glGetUniformLocation( shader.program, "ModelMatrix" );
            unifPrimaryColor
                = glGetUniformLocation( shader.program, "PrimaryColor" );
            unifSecondaryColor
                = glGetUniformLocation( shader.program, "SecondaryColor" );
            unifAmbientLight
                = glGetUniformLocation( shader.program, "AmbientLight" );
            unifLightDirection
                = glGetUniformLocation( shader.program, "LightDirection" );
            unifTextureBlend
                = glGetUniformLocation( shader.program, "TextureBlend" );
            unifCulling
                = glGetUniformLocation( shader.program, "Culling" );
            unifPickingId
                = glGetUniformLocation( shader.program, "PickingId" );
            unifScalarRange
                = glGetUniformLocation( shader.program, "ScalarRange" );
            unifSelfIlumination
                = glGetUniformLocation( shader.program, "SelfIlumination" );

            attPosition = glGetAttribLocation( shader.program, "in_Position" );
            attNormal = glGetAttribLocation( shader.program, "in_Normal" );
            attTexCoord = glGetAttribLocation( shader.program, "in_TexCoord" );
            attScalar = glGetAttribLocation( shader.program, "in_Scalar" );
            attVector = glGetAttribLocation( shader.program, "in_Vector" );
            

            free( path_vertex );
            free( path_fragment );
            free( path_geometry );
            return 0;
        }

        void useProgram() const
        {
            glUseProgram( shader.program );

            if (depth_test != 0){
                glEnable( GL_DEPTH_TEST );
            }
            else{
                glDisable( GL_DEPTH_TEST );
            }

            if (alpha_test != 0){
				glEnable( GL_ALPHA_TEST );
				glAlphaFunc( GL_ALWAYS, 0 );
                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            }
            else{
                glDisable( GL_ALPHA_TEST );
            }

            if (polygon == GW_POLYGON_POINT){
                glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
            }
            else if (polygon == GW_POLYGON_LINE){
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            }
            else{
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            }
        }
    };

}   // end namespace
#endif /*_HGW_SHADER_GENERIC_H */
