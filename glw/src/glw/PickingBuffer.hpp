/*
* Author: Mario J. Martin <dominonurbs$gmail.com>
*
* The picking buffer is an off-screen buffer for identifying the entities
* under mouse coordinates in the scene on render time.
*
*/


#ifndef _HGW_PICKING_BUFFER_H
#define _HGW_PICKING_BUFFER_H

#include <stdlib.h>

#include "common/definitions.h"
#include "common/log.h"
#include "common/check_malloc.h"

#include "gwgl.h"

namespace gw{

    class PickingBuffer
    {
        /* DATA */
    public:
        /* Frame buffer location */
        GLuint fbo;

        /* Render buffer names */
        GLuint rbo_depth;
        GLuint rbo_entity_id;
        GLuint rbo_vertex_id;

        /* Size of the target. Usually the dimensions of the frame
        * are the same as the dimensions of the screen in pixels. */
        GLuint width, height;

    protected:
        /* Specifies if it supports OpenGL ARB (core) or the former EXT */
        enum FrameBufferSupport : int
        {
            FRAME_BUFFER_NO_SUPPORT = 0,
            FRAME_BUFFER_ARB = 1,
            FRAME_BUFFER_EXT = 2
        } support;

    public:

        /* Default constructor */
        PickingBuffer()
        {
            /* Frame buffer object */
            fbo = 0;

            /* Render buffers */
            rbo_depth = 0;
            rbo_entity_id = 0;
            rbo_vertex_id = 0;

            width = 0;
            height = 0;

            support = FRAME_BUFFER_NO_SUPPORT;
        }

        /* Release resources */
        void dispose()
        {
            if (rbo_depth > 0){
                glDeleteRenderbuffers( 1, &rbo_depth );
                rbo_depth = 0;
            }
            if (rbo_entity_id > 0){
                glDeleteRenderbuffers( 1, &rbo_entity_id );
                rbo_entity_id = 0;
            }
            if (rbo_vertex_id > 0){
                glDeleteRenderbuffers( 1, &rbo_vertex_id );
                rbo_vertex_id = 0;
            }
            if (fbo > 0){
                glDeleteFramebuffers( 1, &fbo );
                fbo = 0;
            }

            width = 0;
            height = 0;
        }

        /* Destructor */
        ~PickingBuffer()
        {
            if (rbo_depth > 0){
                _handle_error_( "Warning! Render buffer %i is not deleted before destructor call"
                    , rbo_depth );
                rbo_depth = 0;
            }
            if (rbo_entity_id > 0){
                _handle_error_( "Warning! Render buffer %i is not deleted before destructor call"
                    , rbo_entity_id );
                rbo_entity_id = 0;
            }
            if (rbo_vertex_id > 0){
                _handle_error_( "Warning! Render buffer %i is not deleted before destructor call"
                    , rbo_vertex_id );
                rbo_vertex_id = 0;
            }
            if (fbo > 0){
                _handle_error_( "Warning! Frame buffer texture %i is not deleted before destructor call"
                    , fbo );
                fbo = 0;
            }

            dispose();
        }

        /* Generates a frame buffer texture. This are used for off-screen rendering.
        * The frame buffer is created with a render depth. Make sure that you do not also need
        * a stencil buffer, if that is the case attach it manually. */
        void generateBuffers
            ( const int frame_width
            , const int frame_height
            )
        {
            if (frame_width <= 0 || frame_height <= 0){
                return;
            }

            /* All buffers are regenerated */
            if (rbo_depth > 0){
                glDeleteRenderbuffers( 1, &rbo_depth );
                rbo_depth = 0;
            }
            if (rbo_entity_id > 0){
                glDeleteRenderbuffers( 1, &rbo_entity_id );
                rbo_entity_id = 0;
            }
            if (rbo_vertex_id > 0){
                glDeleteRenderbuffers( 1, &rbo_vertex_id );
                rbo_vertex_id = 0;
            }
            if (fbo > 0){
                glDeleteFramebuffers( 1, &fbo );
                fbo = 0;
            }

            width = frame_width;
            height = frame_height;

            /* Generate depth buffer */
            glGenRenderbuffers( 1, &rbo_depth );
            glBindRenderbuffer( GL_RENDERBUFFER, rbo_depth );
            glRenderbufferStorage
                ( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, frame_width, frame_height );

            glGenRenderbuffers( 1, &rbo_entity_id );
            glBindRenderbuffer( GL_RENDERBUFFER, rbo_entity_id );
            glRenderbufferStorage
                ( GL_RENDERBUFFER, GL_R32UI, frame_width, frame_height );

            glGenRenderbuffers( 1, &rbo_vertex_id );
            glBindRenderbuffer( GL_RENDERBUFFER, rbo_vertex_id );
            glRenderbufferStorage
                ( GL_RENDERBUFFER, GL_R32UI, frame_width, frame_height );

            glBindRenderbuffer( GL_RENDERBUFFER, 0 );

            /* Generate frame buffer */
            glGenFramebuffers( 1, &fbo );
            glBindFramebuffer( GL_FRAMEBUFFER, fbo );

            glFramebufferRenderbuffer
                ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT
                , GL_RENDERBUFFER, rbo_depth
                );

            glFramebufferRenderbuffer
                ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0
                , GL_RENDERBUFFER, rbo_entity_id
                );

            glFramebufferRenderbuffer
                ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1
                , GL_RENDERBUFFER, rbo_vertex_id
                );

            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            if (glCheckFramebufferStatus( GL_FRAMEBUFFER )
                != GL_FRAMEBUFFER_COMPLETE)
            {
                _handle_error_( "Cannot create frame buffer!" );
            }
        }

        /* Restores default frame buffer. */
        void unbindFrameBuffer()
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            /* Remember to restore the correct viewport */
            /* glViewport( 0, 0, screen_width, screen_height ); */
        }

        /* Bind the frame buffer associated to this image */
        void bindFrameBuffer() const
        {
            glBindFramebuffer( GL_FRAMEBUFFER, fbo );

            /* Set the correct target size for this buffer */
            glViewport( 0, 0, width, height );
        }

        /* Gets the data from the framebuffer to CPU memory.
        * Notice that if we target the frame buffer that
        * is already used for rendering will create a lag.
        * One possible improvement is to use a double buffering. */
        void readPixelValue
            ( GLuint* panelID, GLuint* vertexID, GLfloat* zdepth
            , const int mouse_x, const int mouse_y
            )
        {
            int x = mouse_x;
            int y = (height - mouse_y) - 1;

            if (fbo > 0){
                glBindFramebuffer( GL_FRAMEBUFFER, fbo );
                glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

                if (panelID != nullptr){
                    glReadBuffer( GL_COLOR_ATTACHMENT0 );
                    glReadPixels
                        ( x, y, 1, 1
                        , GL_RED_INTEGER, GL_UNSIGNED_INT, panelID
                        );
                }

                if (vertexID != nullptr){
                    glReadBuffer( GL_COLOR_ATTACHMENT1 );
                    glReadPixels
                        ( x, y, 1, 1
                        , GL_RED_INTEGER, GL_UNSIGNED_INT, vertexID
                        );
                }

                if (zdepth != nullptr){
                    glReadPixels
                        ( x, y, 1, 1
                        , GL_DEPTH_COMPONENT, GL_FLOAT, zdepth
                        );
                }

                // restore default framebuffer
                glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            }
        }

        /* Reads the all buffers inside a selection box.
        * The pointers are external buffers of the appropiated size */
        void readSelectionBox
            ( GLuint* panelID_array
            , GLuint* vertexID_array
            , GLfloat* zdepth_array
            , const int mouse_x0
            , const int mouse_x1
            , const int mouse_y0
            , const int mouse_y1
            )
        {
            int x0 = mouse_x0;
            int y0 = (height - mouse_y0) - 1;
            int x1 = mouse_x1;
            int y1 = (height - mouse_y1) - 1;

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

            int xL = (x1 - x0) + 1;
            int yL = (y1 - y0) + 1;

            if (fbo > 0){
                glBindFramebuffer( GL_FRAMEBUFFER, fbo );
                glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

                if (panelID_array != nullptr){
                    glReadBuffer( GL_COLOR_ATTACHMENT0 );
                    glReadPixels
                        ( x0, y0, xL, yL
                        , GL_RED_INTEGER, GL_UNSIGNED_INT, panelID_array
                        );
                }

                if (vertexID_array != nullptr){
                    glReadBuffer( GL_COLOR_ATTACHMENT1 );
                    glReadPixels
                        ( x0, y0, xL, yL
                        , GL_RED_INTEGER, GL_UNSIGNED_INT, vertexID_array
                        );
                }

                if (zdepth_array != nullptr){
                    glReadPixels
                        ( x0, y0, xL, yL
                        , GL_DEPTH_COMPONENT, GL_FLOAT, zdepth_array
                        );
                }

                /* Restore default framebuffer */
                glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            }
        }

        /* Reads the zdepth only inside a selection box.
        * The pointers are external buffers of the appropiated size */
        GLfloat* readSelectionBox_zdepth
            ( const int mouse_x0
            , const int mouse_x1
            , const int mouse_y0
            , const int mouse_y1
            , size_t* buffer_len
            )
        {
            GLfloat* zdepth_buffer = nullptr;
            *buffer_len = 0;

            int x0 = mouse_x0;
            int x1 = mouse_x1;
            int y0 = height - mouse_y0 - 1;
            int y1 = height - mouse_y1 - 1;

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

            if (zdepth_buffer != nullptr && fbo > 0){
                *buffer_len = w*h;
                glBindFramebuffer( GL_FRAMEBUFFER, fbo );
                glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
                glReadPixels
                    ( x0, y0, w, h
                    , GL_DEPTH_COMPONENT, GL_FLOAT, zdepth_buffer
                    );

                /* Restore default framebuffer */
                glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            }

            return zdepth_buffer;
        }
    };


    /*******************************************************************************
    Old alternative using a conventional render target, where the ids are stored
    as RGB values
    *******************************************************************************/

    class PickingBuffer21
    {
    public:
        /* Frame buffer location */
        GLuint fbo;

        /* Render buffer names for the depth */
        GLuint tex0; /* Color texture */
        GLuint rbo_depth; /* Depth buffer */
        GLuint tex_depth; /* The depth is stored in a texture */

        /* Buffer dimensions in pixels */
        int width, height;

    public:

        /* Default constructor */
        PickingBuffer21()
        {
            fbo = 0;
            tex0 = 0;
            rbo_depth = 0;
            tex_depth = 0;
            width = 0;
            height = 0;
        }

        /* Releases resources */
        void dispose()
        {
            if (rbo_depth > 0){
                glDeleteRenderbuffers( 1, &rbo_depth );
                rbo_depth = 0;
            }
            if (fbo > 0){
                glDeleteFramebuffers( 1, &fbo );
                fbo = 0;
            }

            if (tex0 > 0){
                glDeleteTextures( 1, &tex0 );
                tex0 = 0;
            }
            if (tex_depth > 0){
                glDeleteTextures( 1, &tex_depth );
                tex_depth = 0;
            }

            width = 0;
            height = 0;
        }

        /* Destructor */
        ~PickingBuffer21()
        {
            if (tex0 > 0){
                _handle_error_( "Warning! Texture buffer %i is not deleted before destructor call", tex0 );
                tex0 = 0;
            }
            if (tex_depth > 0){
                _handle_error_( "Warning! Texture depth buffer %i is not deleted before destructor call", tex_depth );
                tex_depth = 0;
            }
            if (rbo_depth > 0){
                _handle_error_( "Warning! Render buffer %i is not deleted before destructor call", rbo_depth );
                rbo_depth = 0;
            }
            if (fbo > 0){
                _handle_error_( "Warning! Frame buffer texture %i is not deleted before destructor call", fbo );
                fbo = 0;
            }
        }

        /* Generates a frame buffer texture; used for off-screen rendering.
        * The frame buffer is created with a render depth.
        * Make sure that you do not also need a stencil buffer. */
        void generateBuffers
            ( const int buffer_width
            , const int buffer_height
            )
        {
            if (tex0 > 0){
                glDeleteTextures( 1, &tex0 );
                tex0 = 0;
            }
            if (rbo_depth > 0){
                glDeleteRenderbuffers( 1, &rbo_depth );
                rbo_depth = 0;
            }
            if (tex_depth > 0){
                glDeleteTextures( 1, &tex_depth );
                tex_depth = 0;
            }
            if (fbo > 0){
                glDeleteFramebuffers( 1, &fbo );
                fbo = 0;
            }

            /* To create a cheap antialiasing effect, a bigger buffer can be used.
            * Then it is resized by the GPU to the screen size */
            width = buffer_width;
            height = buffer_height;

            /* Generate color textures */
            glGenTextures( 1, &tex0 );
            glBindTexture( GL_TEXTURE_2D, tex0 );
            glTexImage2D
                ( GL_TEXTURE_2D, 0, GL_RGBA8
                , width, height
                , 0, GL_BGRA, GL_UNSIGNED_BYTE, 0
                );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

            /* Generate depth texture */
            glGenTextures( 1, &tex_depth );
            glBindTexture( GL_TEXTURE_2D, tex_depth ); _check_gl_error_
                glTexImage2D
                ( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8
                , width, height
                , 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0
                ); _check_gl_error_

                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

            glBindTexture( GL_TEXTURE_2D, 0 );

            /* Generate depth buffer with stencil */
            glGenRenderbuffers( 1, &rbo_depth );
            glBindRenderbuffer( GL_RENDERBUFFER, rbo_depth );
            glRenderbufferStorage
                ( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height );

            glBindRenderbuffer( GL_RENDERBUFFER, 0 );

            /* Generate frame buffer */
            glGenFramebuffers( 1, &fbo );
            glBindFramebuffer( GL_FRAMEBUFFER, fbo );

            glFramebufferRenderbuffer
                ( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT
                , GL_RENDERBUFFER, rbo_depth
                );

            glFramebufferTexture2D
                ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0
                , GL_TEXTURE_2D, tex0, 0
                );

            glFramebufferTexture2D
                ( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT
                , GL_TEXTURE_2D, tex_depth, 0
                );

            /* Unbind */
            glBindTexture( GL_TEXTURE_2D, 0 );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            if (glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE){
                _handle_error_( "Cannot create frame buffer" );
            }
        }

        /* Restores default frame buffer. */
        void unbindFrameBuffer()
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            /* Remember to restore the correct viewport */
            /* glViewport( 0, 0, screen_width, screen_height ); */
        }

        /* Bind the frame buffer associated to this image */
        void bindFrameBuffer()
        {
            glBindFramebuffer( GL_FRAMEBUFFER, fbo );

            // Set the correct target size. (Just in case you forget it)
            glViewport( 0, 0, width, height );
        }

        /* Reads all the pixels from the framebuffer to the CPU memory */
        GLubyte* readFrameBuffer()
        {
            GLubyte* data_color;
            _check_( data_color = (GLubyte*)_malloc_
                ( 4 * width * height * sizeof( GLubyte ) ) );

            // bind this frame buffer
            bindFrameBuffer();

            glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
            glReadPixels
                ( 0, 0, width, height
                , GL_BGRA, GL_UNSIGNED_BYTE, data_color );

            // restore default framebuffer
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            return data_color;
        }

        /* Reads all the pixels from the framebuffer to the CPU memory */
        GLubyte* readPixels
            ( const int mouse_x0, const int mouse_y0
            , const int mouse_x1, const int mouse_y1 )
        {
            int x0 = mouse_x0;
            int y0 = (height - mouse_y0) - 1;
            int x1 = mouse_x1;
            int y1 = (height - mouse_y1) - 1;

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

            size_t xL = (x1 - x0) + 1;
            size_t yL = (y1 - y0) + 1;

            GLubyte* data_color;
            _check_( data_color = (GLubyte*)_malloc_( 4 * xL * yL * sizeof( GLubyte ) ) );

            // bind this frame buffer
            bindFrameBuffer();

            glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
            glReadPixels
                ( x0, y0, xL, yL
                , GL_BGRA, GL_UNSIGNED_BYTE, data_color );

            // restore default framebuffer
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            return data_color;
        }


        /* Reads the pixels from the framebuffer to the CPU memory */
        int32_t readPixel( const int mouse_x, const int mouse_y )
        {
            int x = mouse_x;
            int y = (height - mouse_y) - 1;

            int32_t color = 0;

            // bind this frame buffer
            bindFrameBuffer();

            glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
            glReadPixels
                ( x, y, 1, 1
                , GL_BGRA, GL_UNSIGNED_BYTE, &color );

            // restore default framebuffer
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            return color;
        }

        /* Reads all the pixels from the framebuffer to the CPU memory */
        GLfloat* readZdepth
            ( const int mouse_x0, const int mouse_x1
            , const int mouse_y0, const int mouse_y1
            , size_t* buffer_len )
        {
            GLfloat* zdepth_buffer = 0;
            *buffer_len = 0;

            int x0 = mouse_x0;
            int x1 = mouse_x1;
            int y0 = height - mouse_y0 - 1;
            int y1 = height - mouse_y1 - 1;

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

            size_t w = (x1 - x0) + 1;
            size_t h = (y1 - y0) + 1;

            _check_( zdepth_buffer
                = (GLfloat*)_calloc_( w*h, sizeof( GLfloat ) ) );

            if (zdepth_buffer != nullptr){
                *buffer_len = w*h;

                // bind this frame buffer
                bindFrameBuffer();

                glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
                glReadPixels
                    ( x0, y0, w, h
                    , GL_DEPTH_COMPONENT, GL_FLOAT, zdepth_buffer );
            }

            // restore default framebuffer
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            return zdepth_buffer;
        }

        /* Reads z coordinate from the framebuffer to the CPU memory */
        float readZdepth( const int mouse_x, const int mouse_y )
        {
            int x = mouse_x;
            int y = (height - mouse_y) - 1;
            GLfloat zdepth = 0;

            // bind this frame buffer
            bindFrameBuffer();

            glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
            glReadPixels
                ( x, y, 1, 1
                , GL_DEPTH_COMPONENT, GL_FLOAT, &zdepth );

            // restore default framebuffer
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            return (float)zdepth;
        }
    };

} // end namespace

#endif 

/**/
