/**
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#ifndef _HGW_MESH_HPP
#define _HGW_MESH_HPP

#include "signatures.h"
#include "defines.h"
#include "gwdata.h"

#include "VertexStream.hpp"
#include "IndexStream.hpp"
#include "Texture.hpp"
#include "RenderBuffer.hpp"

namespace gw
{
    struct Mesh : public gwMesh
    {
        /* Signature for safer upcasting */
        int signature;

        /* id assigned to identify the object with picking. */
        int id;

        /* Vertex Array Object */
        GLuint vao;

        /* This is used to check that the VBOs are the same to update the VAO */
        GLuint vertex_vbo;
        GLuint scalar_vbo;
        GLuint vector_vbo;
        GLuint index_ibo;

		/* Transformation matrix based on the mesh position */
		gwMatrix4f local_matrix;

		/* Transformation matrix based on the mesh position and the hierarchy */
		gwMatrix4f model_matrix;

        Mesh()
        {
            label[0] = '\0';
            show = 1;
            attachment = nullptr;

            vertex = nullptr;
            index = nullptr;
            scalar = nullptr;
            vector = nullptr;
            update = 0;

            position.x = 0;
            position.y = 0;
            position.z = 0;
            rotation.x = 0;
            rotation.y = 0;
            rotation.z = 0;
            scale.x = 1;
            scale.y = 1;
            scale.z = 1;
            size = 1;

            primary_color.r = 0;
            primary_color.g = 0;
            primary_color.b = 0;
            primary_color.a = 255;

            secondary_color.r = 255;
            secondary_color.g = 255;
            secondary_color.b = 255;
            secondary_color.a = 0;

            self_ilumination = 0;
            culling = 0;
            point_size = 5;
            line_width = 1;
            shaders = 0;

            blit_coords.x = 0;
            blit_coords.y = 0;
            blit_coords.z = 0;
            texture0 = nullptr;
            texture_blend = 0;
            scalar_range_min = 0;
            scalar_range_max = 1;

            vao = 0;
            vertex_vbo = 0;
            scalar_vbo = 0;
            vector_vbo = 0;
            index_ibo = 0;

			parent = nullptr;

            /************************/ 
            f_on_mouse_release = nullptr;
            f_on_mouse_click = nullptr;
            f_on_mouse_enter = nullptr;
            f_on_mouse_leave = nullptr;
            f_on_dispose = nullptr;

            signature = _GW_SIGNATURE_MESH;
            id = 0;

            vao = 0;
            vertex_vbo = 0;
            scalar_vbo = 0;
            vector_vbo = 0;
            index_ibo = 0;

			local_matrix = gwMatrix4f_identity();
			model_matrix = gwMatrix4f_identity();
        }

        ~Mesh()
        {
            if (f_on_dispose != nullptr){
                f_on_dispose( this );
            }
        }

        void calculateNormals()
        {
            if (index->primitive != GL_TRIANGLES){
                _handle_error_( "Calculation of normals is only implemented for triangles!" );
                return;
            }

            /* Initialize */
            for (size_t i = 0; i < vertex->length; i++){
                vertex->stream[i].normal.x = 0;
                vertex->stream[i].normal.y = 0;
                vertex->stream[i].normal.z = 0;
            }

            for (size_t i = 0; i < index->length;){
                gwIndex ia = index->stream[i];
                i++;
                gwIndex ib = index->stream[i];
                i++;
                gwIndex ic = index->stream[i];
                i++;

                gwVector3f va =
                { vertex->stream[ia].position.x
                , vertex->stream[ia].position.y
                , vertex->stream[ia].position.z
                };

                gwVector3f vb =
                { vertex->stream[ib].position.x
                , vertex->stream[ib].position.y
                , vertex->stream[ib].position.z
                };

                gwVector3f vc =
                { vertex->stream[ic].position.x
                , vertex->stream[ic].position.y
                , vertex->stream[ic].position.z
                };

                // calculate cross product
                gwVector3f n;
                n.x = va.y*vb.z - vc.z*vb.y;
                n.y = va.z*vb.x - vc.x*vb.z;
                n.z = va.x*vb.y - vc.y*vb.x;

                vertex->stream[ia].normal.x += n.x;
                vertex->stream[ia].normal.y += n.y;
                vertex->stream[ia].normal.z += n.z;

                vertex->stream[ib].normal.x += n.x;
                vertex->stream[ib].normal.y += n.y;
                vertex->stream[ib].normal.z += n.z;

                vertex->stream[ic].normal.x += n.x;
                vertex->stream[ic].normal.y += n.y;
                vertex->stream[ic].normal.z += n.z;
            }

            /* Normalize */
            for (size_t i = 0; i < vertex->length; i++){
                gwFloat imod = gwVector3f_imod( &(vertex->stream[i].normal) );
                vertex->stream[i].normal.x *= imod;
                vertex->stream[i].normal.y *= imod;
                vertex->stream[i].normal.z *= imod;
            }
        }

        gwMatrix4f calculate_transformation_matrix
            ( const gwMatrix4f parent_matrix
            , const gwVector3f parent_blit
            , const int screen_width
            , const int screen_height
            ) const
        {
            gwMatrix4f model_matrix = gwMatrix4f_transform
                ( position.x, position.y, position.z
                , rotation.x, rotation.y, rotation.z
                , scale.x * size
                , scale.y * size
                , scale.z * size
                );

            gwMatrix4f matrix = gwMatrix4f_mul
                ( &parent_matrix, &(model_matrix) );

            gwVector3f blitCoords;
            blitCoords.x = parent_blit.x + blit_coords.x;
            blitCoords.y = parent_blit.y + blit_coords.y;
            blitCoords.z = parent_blit.z + blit_coords.z;

            matrix.data[12] += (2 * blitCoords.x) / screen_width;
            matrix.data[13] += (2 * blitCoords.y) / screen_width;
            matrix.data[14] += blitCoords.z;

            return matrix;
        }

        void update_GL2x()
        {
            VertexStream* vertex = (VertexStream*)this->vertex;
            IndexStream* index = (IndexStream*)this->index;
            ScalarStream* scalar = (ScalarStream*)this->scalar;
            VectorStream* vector = (VectorStream*)this->vector;

            if (vertex == nullptr){
                return;
            }

            if (vertex->vbo == 0 || vertex->update != 0){
                vertex->updateBuffer( vertex->length );
                vertex->update = 0;
            }
            if (index != nullptr && (index->ibo == 0 || index->update != 0)){
                index->updateBuffer();
                index->update = 0;
            }
            if (scalar != nullptr && (scalar->vbo == 0 || scalar->update != 0)){
                scalar->updateBuffer( vertex->length );
                scalar->update = 0;
            }
            if (vector != nullptr && (vector->vbo == 0 || vector->update != 0)){
                vector->updateBuffer( vertex->length );
                vector->update = 0;
            }

            /* Update the textures, if neccesary */
            Texture* tex0 = (Texture*)this->texture0;
            if (tex0 != nullptr && tex0->signature == _GW_SIGNATURE_TEXTURE){
                if (tex0->image.update != 0 || tex0->tex_name == 0){
                    tex0->updateTexture();
                }
            }

            /* Unbind the buffers, so the vertex array can not be used
            * in a later process accidentally */
            glBindBuffer( GL_ARRAY_BUFFER, 0 );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }

        void draw_GL2x( const ShaderGeneric* shader ) const
        {
            VertexStream* vertex = (VertexStream*)this->vertex;
            IndexStream* index = (IndexStream*)this->index;
            ScalarStream* scalar = (ScalarStream*)this->scalar;
            VectorStream* vector = (VectorStream*)this->vector;

            if (vertex == nullptr)
                return;

            /* Select the primitive from the shader or the index stream */
            int polygon = GW_POLYGON_POINT;
            if (shader->polygon != GW_POLYGON_NONE){
                polygon = shader->polygon;
            }
            else{
                if (index != nullptr){
                    if (index->gl_primitive() == GL_POINTS){
                        polygon = GW_POLYGON_POINT;
                    }
                    else if (index->gl_primitive() == GL_TRIANGLES){
                        polygon = GW_POLYGON_FILL;
                    }
                    else{ /* GL_LINES GL_LINES_ADJACENCY GL_LINE_LOOP */
                        polygon = GW_POLYGON_LINE;
                    }
                }
            }

            /* Change the depth range so points are looked forward */
            const GLfloat zdepth = 1.0f / 64;
            if (polygon == GW_POLYGON_POINT){
                glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
                glDepthRange( 0.0f, 1.0f - 2 * zdepth );
            }
            else if (polygon == GW_POLYGON_LINE){
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                glDepthRange( 0.0f, 1.0f - zdepth );
            }
            else{
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                glDepthRange( 0.0f, 1.0f );
            }

            Texture* tex0 = (Texture*)texture0;

            glBindVertexArray( 0 );
            glBindBuffer( GL_ARRAY_BUFFER, vertex->vbo );

            vertex->enable
                ( shader->attPosition
                , shader->attNormal
                , shader->attTexCoord
                );

            vertex->attribFormat
                ( shader->attPosition, shader->attNormal, shader->attTexCoord );

            if (scalar != nullptr && scalar->vbo > 0 && shader->attScalar >= 0){
                scalar->enable( shader->attScalar );
                glBindBuffer( GL_ARRAY_BUFFER, scalar->vbo );
                scalar->attribFormat( shader->attScalar );
            }

            if (vector != nullptr && vector->vbo > 0 && shader->attVector >= 0){
                vector->enable( shader->attVector );
                glBindBuffer( GL_ARRAY_BUFFER, vector->vbo );
                vector->attribFormat( shader->attVector );
            }

            /* Draw */
            GLenum gl_primitive = GL_POINTS;
            if (shader->polygon != GW_POLYGON_POINT && index != nullptr){
                gl_primitive = index->gl_primitive();
            }
            if (index != nullptr && index->ibo > 0){
                glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index->ibo );
                glDrawElements( gl_primitive, index->buffer_count
                    , index->indexType(), BUFFER_OFFSET( 0 ) );
            }
            else{
                glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
                if (index == nullptr){
                    glDrawArrays( GL_POINTS, 0, vertex->buffer_count );
                }
                else{
                    glDrawArrays( gl_primitive, 0, vertex->buffer_count );
                }
            }

            vertex->disable
                ( shader->attPosition
                , shader->attNormal
                , shader->attTexCoord
                );

            if (scalar != nullptr && scalar->vbo > 0 && shader->attScalar > 0){
                scalar->disable( shader->attScalar );
            }
            if (vector != nullptr && vector->vbo > 0 && shader->attVector > 0){
                vector->disable( shader->attVector );
            }

            glBindTexture( GL_TEXTURE_2D, 0 );
        }

        void update_vao
            ( VertexStream* vertex
            , IndexStream* index
            , ScalarStream* scalar
            , VectorStream* vector
            )
        {
            if (vao != 0){
                glDeleteVertexArrays( 1, &vao );
            }
            glGenVertexArrays( 1, &vao );
            glBindVertexArray( vao );

            glBindBuffer( GL_ARRAY_BUFFER, vertex->vbo );
            vertex->attribFormatArray( ATT0_POSITION, ATT1_NORMAL, ATT2_TEXCOORD );

            if (scalar != nullptr && scalar->vbo != 0){
                glBindBuffer( GL_ARRAY_BUFFER, scalar->vbo );
                scalar->attribFormatArray( ATT3_SCALAR );
            }

            if (vector != nullptr && vector->vbo != 0){
                glBindBuffer( GL_ARRAY_BUFFER, vector->vbo );
                vector->attribFormatArray( ATT4_VECTOR );
            }

            if (index != nullptr && index->ibo > 0){
                glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index->ibo );
            }

            /* Unbind the buffers, so the vertex array can not be used
            * in a later process accidentally */
            glBindVertexArray( 0 );
            glBindBuffer( GL_ARRAY_BUFFER, 0 );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }

        void update_GL3x()
        {
            VertexStream* vertex = (VertexStream*)this->vertex;
            IndexStream* index = (IndexStream*)this->index;
            ScalarStream* scalar = (ScalarStream*)this->scalar;
            VectorStream* vector = (VectorStream*)this->vector;

            if (vertex == nullptr){
                return;
            }

            if (vertex->vbo == 0 || vertex->update != 0){
                vertex->updateBuffer( vertex->length );
                vertex->update = 0;
            }
            if (index != nullptr && (index->ibo == 0 || index->update != 0)){
                index->updateBuffer();
                index->update = 0;
            }
            if (scalar != nullptr && (scalar->vbo == 0 || scalar->update != 0)){
                scalar->updateBuffer( vertex->length );
                scalar->update = 0;
            }
            if (vector != nullptr && (vector->vbo == 0 || vector->update != 0)){
                vector->updateBuffer( vertex->length );
                vector->update = 0;
            }

            int _vbo = (vertex != nullptr) ? vertex->vbo : 0;
            int _ibo = (index != nullptr) ? index->ibo : 0;
            int _sbo = (scalar != nullptr) ? scalar->vbo : 0;
            int _xbo = (vector != nullptr) ? vector->vbo : 0;
            if (vao == 0
                || vertex_vbo != _vbo
                || index_ibo != _ibo
                || scalar_vbo != _sbo
                || vector_vbo != _xbo
                ){
                update_vao( vertex, index, scalar, vector );
            }
        }

        void draw_GL3x( const ShaderGeneric* shader ) const
        {
            VertexStream* vertex = (VertexStream*)this->vertex;
            IndexStream* index = (IndexStream*)this->index;
            ScalarStream* scalar = (ScalarStream*)this->scalar;
            VectorStream* vector = (VectorStream*)this->vector;

            if (vertex == nullptr)
                return;

            /* Select the primitive from the shader or the index stream */
            int polygon = GW_POLYGON_POINT;
            if (shader->polygon != GW_POLYGON_NONE){
                polygon = shader->polygon;
            }
            else{
                if (index != nullptr){
                    if (index->gl_primitive() == GL_POINTS){
                        polygon = GW_POLYGON_POINT;
                    }
                    else if (index->gl_primitive() == GL_TRIANGLES){
                        polygon = GW_POLYGON_FILL;
                    }
                    else{ /* GL_LINES GL_LINES_ADJACENCY GL_LINE_LOOP */
                        polygon = GW_POLYGON_LINE;
                    }
                }
            }

            /* Change the depth range so points are looked forward */
            const GLfloat zdepth = 1.0f / 64;
            if (polygon == GW_POLYGON_POINT){
                glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
                glDepthRange( 0.0f, 1.0f - 2 * zdepth );
            }
            else if (polygon == GW_POLYGON_LINE){
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                glDepthRange( 0.0f, 1.0f - zdepth );
            }
            else{
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                glDepthRange( 0.0f, 1.0f );
            }

            glBindVertexArray( vao );

            GLenum gl_primitive = GL_POINTS;
            if (shader->polygon != GW_POLYGON_POINT && index != nullptr){
                gl_primitive = index->gl_primitive();
            }
            if (index != nullptr && index->ibo > 0){
                glDrawElements( gl_primitive, index->buffer_count
                    , index->indexType(), BUFFER_OFFSET( 0 ) );
            }
            else{
                if (index == nullptr){
                    glDrawArrays( GL_POINTS, 0, vertex->buffer_count );
                }
                else{
                    glDrawArrays( gl_primitive, 0, vertex->buffer_count );
                }
            }

            glBindTexture( GL_TEXTURE_2D, 0 );
        }

		gwMatrix4f update_transform_recursive( Mesh* mesh )
		{
			if ( mesh->parent == nullptr ){
				return mesh->local_matrix;
			}

			gwMatrix4f parent_matrix = update_transform_recursive( (Mesh*)mesh->parent );
			gwMatrix4f transf_matrix = gwMatrix4f_mul( &parent_matrix, &mesh->local_matrix );

			return transf_matrix;
		}

		void update_transformation_matrix()
		{
			model_matrix = update_transform_recursive( this );
		}

		void update_local_matrix()
		{
			local_matrix = gwMatrix4f_transform
				( position.x, position.y, position.z
				, rotation.x, rotation.y, rotation.z
				, scale.x * size, scale.y * size, scale.z * size
				);
		}

        void set_uniforms
            ( const ShaderGeneric* shader
            , const Mesh* mesh
            , const gwMatrix4f model_matrix
            , const gwMatrix4f camera_matrix
            , const int screen_width
            , const int screen_height
            ) const
        {
            _check_gl_error_
            if (mesh->culling > 0){
                glEnable( GL_CULL_FACE );
                glCullFace( GL_BACK );
            }
            else if (mesh->culling < 0){
                glEnable( GL_CULL_FACE );
                glCullFace( GL_FRONT );
            }
            else{
                glDisable( GL_CULL_FACE );
            }

            glPointSize( mesh->point_size );
            glLineWidth( mesh->line_width );

            gwColor ambient_light = shader->ambient_light;
            float intensity = shader->ambient_light.a + 255 * mesh->self_ilumination;
            if (intensity > 255){
                intensity = 255;
            }
            ambient_light.a = (uint8_t)intensity;

            gwMatrix4f model_view_matrix = gwMatrix4f_mul( &camera_matrix, &model_matrix );
            shader->setModelViewProjMatrix( model_view_matrix.data );
            shader->setViewMatrix( camera_matrix.data );
            shader->setModelMatrix( model_matrix.data );
            shader->setAmbientLight( ambient_light );
            shader->setLightDirection( shader->light_direction );
            shader->setScreenSize( (gwFloat)screen_width, (gwFloat)screen_height );
            shader->setPrimaryColor( mesh->primary_color );
            shader->setSecondaryColor( mesh->secondary_color );
            shader->setCulling( mesh->culling );
            shader->setPickingId( mesh->id );
            shader->setScalarRange( mesh->scalar_range_min, mesh->scalar_range_max );
            shader->setSelfIlumination( mesh->self_ilumination );

            if (mesh->texture0 != nullptr){
                shader->setTextureBlend( mesh->texture_blend );
            }
            else{
                shader->setTextureBlend( 1.0f );
            }

            /* Updates the texture if required and bind the texture */
            Texture* tex0 = (Texture*)mesh->texture0;
            if (tex0 != nullptr){
                if (tex0->signature == _GW_SIGNATURE_TEXTURE){
                    /* Conventional texture */
                    if (shader->unifTexture0 >= 0 && shader->unifTexture0 >= 0){
                        if (tex0->image.update != 0 || tex0->tex_name == 0){
                            tex0->updateTexture();
                            tex0->image.update = 0;
                        }
                        tex0->bindTexture( shader->unifTexture0, 0 );
                    }
                }
                else if (tex0->signature == _GW_SIGNATURE_RENDER_BUFFER){
                    /* Render buffer, which also gets access to the depth */
                    RenderBuffer* render_buffer = (RenderBuffer*)mesh->texture0;
                    int unit0 = 0;
                    int unit1 = 1;
                    if (render_buffer->color.tex_name > 0 && shader->unifTexture0 >= 0){
                        glActiveTexture( GL_TEXTURE0 + unit0 );
                        glUniform1i( shader->unifTexture0, unit0 );
                        glBindTexture( GL_TEXTURE_2D, render_buffer->color.tex_name );
                    }
                    if (render_buffer->tex_depth > 0 && shader->unifDepthBuffer >= 0){
                        glActiveTexture( GL_TEXTURE0 + unit1 );
                        glUniform1i( shader->unifDepthBuffer, unit1 );
                        glBindTexture( GL_TEXTURE_2D, render_buffer->tex_depth );
                    }
                }
            }

            /* If the gl_useProgram(shader) was not called, it will trigger an error */
            _check_gl_error_
        }

        void draw( const ShaderGeneric* shader, const int gl_version
            , const gwMatrix4f* model_matrix, const gwMatrix4f* camera_matrix )
        {
            if (gl_version >= 30){
                /* OpenGL 3x */
                update_GL3x();
                draw_GL3x( shader );
            }
            else if (gl_version >= 20){
                /* OpenGL 2x */
                update_GL2x();
                draw_GL2x( shader );
            }
            else{
                /* OpenGL 1x */
                if (shader->f_gl1x != nullptr){
                    shader->f_gl1x( this, shader, model_matrix, camera_matrix );
                }
            }
        }

        void render( Mesh* mesh
            , const ShaderGeneric* shader, const gwMatrix4f camera_matrix
            , const int screen_width, const int screen_height
            , const int gl_version )
        {
            /* Render this object */
            if (mesh->show > 0 && (mesh->shaders & shader->mask) != 0){
               mesh->set_uniforms( shader, mesh, mesh->model_matrix, camera_matrix
                    , screen_width, screen_height );

                mesh->draw( shader, gl_version, &mesh->model_matrix, &camera_matrix );
            }
        }
    };
}

#endif _HGW_MESH_HPP
/**/
