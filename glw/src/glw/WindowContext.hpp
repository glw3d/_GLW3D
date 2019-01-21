/**
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#ifndef _HGW_WINDOWCONTEXT_HPP
#define _HGW_WINDOWCONTEXT_HPP

#include "signatures.h"
#include "defines.h"
#include "gwdata.h"
#include "Stream.hpp"
#include "PickingBuffer.hpp"
#include "RenderBuffer.hpp"

namespace gw
{
    struct PickingBuffer10{
        GLubyte* data;
        int width;
        int height;

        PickingBuffer10()
        {
            data = nullptr;
            width = 0;
            height = 0;
        }
    };

    struct WindowContext : public gwWindowContext
    {
        /* Signature for safer upcasting */
        int signature;

        /* id assigned when the GL context is first generated. */
        int id;

        /* The OpenGL version the context has been created with */
        int gl_version;

        /* Indicates whether the default shaders should be loaded */
        int request_load_default_shaders;

        char default_shaders_path[1024];

        /* List of shaders attached to this context */
        Stream<ShaderGeneric*> shader_pipeline;

        /* List of cameras in the render list */
        DynamicStack< Camera > list_cameras;

        /* List of frames for rendering to the default frame buffer */
        DynamicStack< Frame > list_frames;

        /* List of frame buffers for rendering */
        DynamicStack< RenderTarget > list_buffers;

        gwMesh* quad;

        /* Buffer used for multi-pass */
        RenderTarget auxiliary_buffer;

        /* Offscreen buffer to identify entities in the scene */
        PickingBuffer picking_buffer;

        /* For OpenGL 2.x integer buffers are not supported, so a texture
         * render buffer is used. The id values are stored in RGBA values,
         * which has some limitations. */
        PickingBuffer21 picking_buffer_21;

        /* For OpenGL 1.x the picking buffer is in the CPU */
        PickingBuffer10 picking_color_buffer_10;

        WindowContext( const int id )
        {
            gl_version_hint = 0;
            is_rendering = 0;
            screen_width = 800;
            screen_height = 600;
            action = 0;
            update = 0;
            background_color = gwColorBlue;
            offscreen_buffer = &auxiliary_buffer;

            f_on_render = nullptr;
            f_on_keycharpress = nullptr;

            signature = _GW_SIGNATURE_CONTEXT;
            this->id = id;
            gl_version = 0;
            request_load_default_shaders = 0;
            default_shaders_path[0] = '\0';
            shader_pipeline.alloc( _GW_SHADERS_PIPELINE_LENGTH );

            quad = gwShape_quad01();
        }

        /* Gets an available shader slot.
         * There are reserved slots for the default shaders */
        int requestShaderSlot() const
        {
            for (int i = _GW_NUM_DEFAULT_SHADERS; i < _GW_SHADERS_PIPELINE_LENGTH; i++){
                if (shader_pipeline[i] == nullptr){
                    return i;
                }
            }

            return -1;
        }

        /* This method is called at startup and everytime the screen is resize
        * to regenerate all framebuffers to match the screen dimensions */
        void regenerate_framebuffers()
        {
            /* Regenerate the picking buffer */
            if (gl_version >= 30){
                if (screen_width != picking_buffer.width
                    || screen_height != picking_buffer.height)
                {
                    picking_buffer.generateBuffers( screen_width, screen_height );
                }
            }
            else if (gl_version >= 20){
                if (screen_width != picking_buffer_21.width
                    || screen_height != picking_buffer_21.height)
                {
                    picking_buffer_21.generateBuffers( screen_width, screen_height );
                }
            }
            else{
                if (screen_width != picking_color_buffer_10.width
                    || screen_height != picking_color_buffer_10.height)
                {
                    free( picking_color_buffer_10.data );
                    _check_( picking_color_buffer_10.data
                        = (GLubyte*)_malloc_( 4 * screen_width*screen_height*sizeof( GLubyte ) ) );
                    picking_color_buffer_10.width = screen_width;
                    picking_color_buffer_10.height = screen_height;
                }
            }

            /* Regenerate the auxiliary buffer */
            auxiliary_buffer.update( screen_width, screen_height );

            /* Regenerate the target buffers */
            const DynamicStack< RenderTarget >* token = &list_buffers;
            while (token != nullptr){
                RenderTarget* render_target = token->obj;
                token = token->next;
                if (render_target != nullptr){
                    render_target->update( screen_width, screen_height );
                }
            }
        }

        /* Rearrange the frames acording to its preference number */
        void arrange_frames()
        {
            int flag = 1;
            while (flag != 0){
                flag = 0;
                DynamicStack< Frame >* token0 = &list_frames;
                while (token0 != nullptr){
                    DynamicStack< Frame >* token1 = token0->next;
                    if (token1 != nullptr){
                        Frame* frame0 = token0->obj;
                        Frame* frame1 = token1->obj;
                        if (frame0->preference > frame1->preference){
                            /* Swap frames */
                            token1->obj = frame0;
                            token0->obj = frame1;
                            flag = 1;
                        }
                    }
                    token0 = token1;
                }
            }
        }

        /* Clears the screen */
        void clear() const
        {
            /* Clear the picking buffer area */
            if (picking_buffer.fbo != 0){
                glBindFramebuffer( GL_FRAMEBUFFER, picking_buffer.fbo );
                const GLenum att[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
                glDrawBuffers( 2, att );
                GLint panel = 0;
                GLint vertex = 0;
                GLfloat depth = 0;
                glClearBufferiv( GL_COLOR, 0, &panel );
                glClearBufferiv( GL_COLOR, 1, &vertex );
                glClear( GL_DEPTH_BUFFER_BIT );
            }
            if (picking_buffer_21.fbo != 0){
                glBindFramebuffer( GL_FRAMEBUFFER, picking_buffer_21.fbo );
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
            }
            if (picking_color_buffer_10.data != nullptr){
                memset( picking_color_buffer_10.data, 0
                    , sizeof( GLuint ) * screen_width * screen_height );
            }

            glEnable( GL_DEPTH_TEST );
            glDepthMask( GL_TRUE );

            /* Set the correct target size */
            glEnable( GL_SCISSOR_TEST );
            glViewport( 0, 0, screen_width, screen_height );
            glScissor( 0, 0, screen_width, screen_height );

            /* Clear the render screen area */
            const GLenum att[] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers( 1, att );
            GLfloat bgc_r = (GLfloat)background_color.r / 255;
            GLfloat bgc_g = (GLfloat)background_color.g / 255;
            GLfloat bgc_b = (GLfloat)background_color.b / 255;
            glClearColor( bgc_r, bgc_g, bgc_b, 1.0f );
            glClearDepth( 1e6 );
            glClearStencil( 0 );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 ); /* Default framebuffer */
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
        }

        /* Reads the picking buffer under the pixel coordinates */
        void get_picking
            ( int* p_panel_id, int* p_vertex_id, float* p_zdepth
            , const int x, const int y )
        {
            GLuint panel_id;
            GLuint vertex_id;
            GLfloat zdepth;

            if (gl_version >= 30){
                picking_buffer.readPixelValue( &panel_id, &vertex_id, &zdepth, x, y );
            }
            else if (gl_version >= 20){
                int32_t color = picking_buffer_21.readPixel( x, y );
                GLubyte* data = (GLubyte*)(&color);
                panel_id = data[1];

                zdepth = picking_buffer_21.readZdepth( x, y );
                vertex_id = -1;
            }
            else{ /* gl_version == 1x */
                vertex_id = -1;
                zdepth = 0;
                panel_id = 0;

                size_t ipixel = (screen_height - y - 1)*screen_width + x;
                GLubyte color = picking_color_buffer_10.data[4 * ipixel];
                panel_id = color;

                glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
                glReadPixels( x, screen_height - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zdepth );
            }

            *p_panel_id = (int)panel_id;
            *p_vertex_id = (int)vertex_id;
            *p_zdepth = (float)zdepth;
        }

        /* Reads the picking buffer depth inside a box */
        GLfloat* get_picking_box( int x0, int y0, int x1, int y1, size_t* buffer_len )
        {
            GLfloat* zdepth_buffer = nullptr;
            if (gl_version >= 30){
                zdepth_buffer = picking_buffer.readSelectionBox_zdepth( x0, x1, y0, y1, buffer_len );
            }
            else if (gl_version >= 20){
                zdepth_buffer = picking_buffer_21.readZdepth( x0, x1, y0, y1, buffer_len );
            }
            else{ /* gl_version == 1x */
                if (x0 > x1){
                    int temp = x0;
                    x0 = x1;
                    x1 = temp;
                }

                if (y0 > y1){
                    int temp = y0;
                    y0 = y1;
                    y1 = temp;
                }

                int w = (x1 - x0);
                int h = (y1 - y0);

                _check_( zdepth_buffer
                    = (GLfloat*)_calloc_( w*h, sizeof( GLfloat ) ) );

                glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
                glReadPixels( x0, y0, x1 - x0, y1 - y0
                    , GL_DEPTH_COMPONENT, GL_FLOAT, zdepth_buffer );
            }

            return zdepth_buffer;
        }

        void check_shader_use( const int mask, int* shader_use ) const
        {
            for (int i = 0; i < _GW_SHADERS_PIPELINE_LENGTH; i++){
                if ((mask & (1<<i)) != 0){
                    shader_use[i] += 1;
                }
            }
        }

        void check_shader_use( const Mesh* mesh, int* shader_use ) const
        {
            check_shader_use( mesh->shaders, shader_use );
        }

        void check_camera_shader_use( const Camera* camera, int* shader_use ) const
        {
            const DynamicStack< Mesh >*token = &camera->render_list;
            Mesh* mesh = token->obj;
            while (mesh != nullptr){
                check_shader_use( mesh, shader_use );
                token = token->next;
                if (token != nullptr){
                    mesh = token->obj;
                }
                else{
                    mesh = nullptr;
                }
            }
        }

        void update_camera_matrix
            ( const DynamicStack<Camera>* list_cameras
            , int* shader_use
            , const int width, const int height ) const
        {
            /* Loop through the cameras */
            const DynamicStack<Camera>* token_camera = list_cameras;
            while (token_camera != nullptr){
                Camera* camera = token_camera->obj;
                token_camera = token_camera->next;
                if (camera != nullptr && camera->active != 0){
                    camera->view_matrix = gwCamera_calculate_matrix( camera, width, height );
                    check_camera_shader_use( camera, shader_use );
                }
            }
        }

        void render_cameras( const DynamicStack<Camera>* list_cameras
            , const ShaderGeneric* shader
            , const int width, const int height ) const
        {
            /* Loop through the cameras */
            const DynamicStack<Camera>* token_camera = list_cameras;
            while (token_camera != nullptr){
                Camera* camera = token_camera->obj;
                token_camera = token_camera->next;
                if (camera != nullptr && camera->active != 0){
                    camera->update_transformations( width, height );
                    camera->render( width, height, shader, gl_version );
                }
            }
        }

        void render_shader( const DynamicStack<Camera>* list_cameras
            , ShaderGeneric* shader
            , const gwViewPort viewport ) const
        {
            /* Load or reload the shader if request */
            if (shader->update != 0 || shader->shader.fragment_shader == 0){

                shader->load( shader->path
                    , shader->vertex_filename
                    , shader->fragment_filename
                    , shader->geometric_filename );


                /* Load multipass shaders */
                ShaderGeneric* next = (ShaderGeneric*)shader->multipass;
                while (next != nullptr){
                    next->load( next->path
                        , next->vertex_filename
                        , next->fragment_filename
                        , next->geometric_filename );

                    next = (ShaderGeneric*)next->multipass;
                }

                shader->update = 0;
            }

            _check_gl_error_

            /* Bind the correct framebuffer */
            if (shader->target == GW_TARGET_PICKING){
                /* Target the picking buffer */
                const GLenum att[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
                glBindFramebuffer( GL_FRAMEBUFFER, picking_buffer.fbo );
                glDrawBuffers( 2, att );
            }
            else{
                const GLenum att[] = { GL_COLOR_ATTACHMENT0 };
                glBindFramebuffer( GL_FRAMEBUFFER, viewport.fbo );
            }

            if (shader->shader.program == 0){
                _handle_error_( "The shader %s is not loaded", shader->fragment_filename ); 
                return;
            }

            /* Activate the shader */
            shader->useProgram();

            /* If it is a multipass shader activate the stencil */
            if (shader->multipass != nullptr){
                glEnable( GL_STENCIL_TEST );
                glStencilMask( 0xFF );
                glDepthMask( GL_TRUE );
                glStencilFunc( GL_ALWAYS, 1, 0xFF );
                glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
                glClearStencil( 0 );
                glClear( GL_STENCIL_BUFFER_BIT );
            }

            /* Render the normal scene */
            render_cameras( list_cameras, shader, viewport.width, viewport.height );

            /* If it is a multipass render the next steps as a texture of a quad */
            if (shader->multipass != nullptr){
                Mesh* quad = (Mesh*)(this->quad);
                RenderTarget* render_target = (RenderTarget*)(&(this->auxiliary_buffer));
                ShaderGeneric* next_pass = (ShaderGeneric*)shader->multipass;
                while (next_pass != nullptr && next_pass->signature == _GW_SIGNATURE_SHADER){
                    /* Copy the current buffer to the auxiliary buffer */
                    int x0 = viewport.x0;
                    int y0 = viewport.y0;
                    int x1 = x0 + viewport.width;
                    int y1 = y0 + viewport.height;
                    int current_fbo = viewport.fbo;

                    int buf_width = render_target->render_buffer.buffer_width;
                    int buf_height = render_target->render_buffer.buffer_height;

                    /* Copy everything, including the depth buffer,
                    * although it will not be accesible in the shader */
                    glBindFramebuffer( GL_READ_FRAMEBUFFER, viewport.fbo );
                    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, render_target->render_buffer.fbo );

                    glBlitFramebuffer
                        ( x0, y0, x1, y1, 0, 0, buf_width, buf_height
                        , GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
                        );

                    /* Draw the quad */
                    glBindFramebuffer( GL_FRAMEBUFFER, viewport.fbo );

                    glDisable( GL_DEPTH_TEST );
                    glStencilFunc( GL_EQUAL, 1, 0xFF );

                    next_pass->useProgram();

                    gwMatrix4f ident = gwMatrix4f_identity();
                    quad->texture0 = render_target->color_texture;
                    quad->set_uniforms( next_pass, quad, ident, ident
                        , screen_width, screen_height );
                    quad->draw( next_pass, this->gl_version
                        , &ident, &ident );

                    next_pass = (ShaderGeneric*)next_pass->multipass;
                }
                glBindFramebuffer( GL_FRAMEBUFFER, viewport.fbo );
                glDisable( GL_STENCIL_TEST );
            }
        }

        void render_pipeline( const DynamicStack<Camera>* list_cameras
            , const gwViewPort viewport ) const
        {
            int shader_use[_GW_SHADERS_PIPELINE_LENGTH] = { 0 };
            update_camera_matrix( list_cameras, shader_use
                , viewport.width, viewport.height );

            /* Loop through the shaders pipeline */
            for (int ish = 0; ish < _GW_SHADERS_PIPELINE_LENGTH; ish++){
                if (shader_use[ish] > 0){
                    ShaderGeneric* shader = shader_pipeline[ish];
                    if (shader == nullptr) continue;

                    /* Check if the shader requires to be reloaded */
                    if (shader->update != 0){
                        shader->load( shader->path
                            , shader->vertex_filename
                            , shader->fragment_filename
                            , shader->geometric_filename );

                        /* Load multipass shaders */
                        ShaderGeneric* next = (ShaderGeneric*)shader->multipass;
                        while (next != nullptr){
                            next->load( next->path
                                , next->vertex_filename
                                , next->fragment_filename
                                , next->geometric_filename );

                            next = (ShaderGeneric*)next->multipass;
                        }
                        shader->update = 0;
                    }
                    render_shader( list_cameras, shader, viewport );
                }
            }
        }

        /* Loop through the frames */
        void render_frames
            ( const DynamicStack< Frame >* list_frames
            , const gwViewPort parent_viewport ) const
        {
            const DynamicStack< Frame >* token_frame = list_frames;
            while (token_frame != nullptr){
                Frame* frame = token_frame->obj;
                token_frame = token_frame->next;
                if (frame == nullptr) continue;

                gwViewPort viewport = frame->clear( parent_viewport );
                render_pipeline( &frame->render_list, viewport );
            }
        }

        void render() const
        {
            /* Loop through the frame buffers */
            const DynamicStack< RenderTarget >* token_buffer = &list_buffers;
            RenderTarget* target = token_buffer->obj;
            while (target != nullptr){
                gwViewPort viewport;
                viewport.x0 = 0;
                viewport.y0 = 0;
                viewport.width = target->render_buffer.buffer_width;
                viewport.height = target->render_buffer.buffer_height;
                viewport.fbo = target->render_buffer.fbo;

                target->clear();
                render_frames( &target->list_frames, viewport );

                token_buffer = token_buffer->next;
                target = token_buffer != nullptr ? token_buffer->obj : nullptr;
           }

            /* Render the main screen */
            gwViewPort viewport;
            viewport.x0 = 0;
            viewport.y0 = 0;
            viewport.width = screen_width;
            viewport.height = screen_height;
            viewport.fbo = 0;

            this->clear();

            render_frames( &list_frames, viewport );
        }
    };
}

#endif _HGW_WINDOWCONTEXT_HPP
/**/
