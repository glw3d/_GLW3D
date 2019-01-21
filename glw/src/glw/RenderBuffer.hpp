/*
* Author: Mario J. Martin <dominonurbs$gmail.com>
*
* The render buffer is a very especialized frame buffer
* for offscreen rendering and postprocessing techniques.
*/

#ifndef _HGW_RederTarget_H
#define _HGW_RederTarget_H


#include <stdlib.h>
#include <stdint.h>

#include "common/definitions.h"
#include "common/log.h"
#include "common/check_malloc.h"

#include "gwgl.h"
#include "Texture.hpp"

namespace gw{

    class RenderBuffer
    {
    public:
        Texture color;

        /* Signature */
        int signature;

        /* Frame buffer location */
        GLuint fbo;

        /* Render buffer names for the depth */
        GLuint rbo_depth;
        GLuint tex_depth;

        /* Dimensions in pixels */
        int buffer_width, buffer_height;
                                                                                            
    public:

        /* Default constructor */
        RenderBuffer()
        {
            /* The signature may change if it is the auxiliary frame buffer */ 
            signature = _GW_SIGNATURE_RENDER_TARGET;

            /* Frame buffer object */
            fbo = 0;

            /* Color texture */
            color.tex_name = 0;

            /* Depth buffer */
            rbo_depth = 0;

            /* The depth is stored in a texture */
            tex_depth = 0;

            /* Buffer dimensions */
            buffer_width = 0;
            buffer_height = 0;
        }

        /* Releases resources */
        void dispose()
        {
            color.dispose();

            if (rbo_depth > 0){
                glDeleteRenderbuffers( 1, &rbo_depth );
                rbo_depth = 0;
            }
            if (fbo > 0){
                glDeleteFramebuffers( 1, &fbo );
                fbo = 0;
            }

            if (color.tex_name > 0){
                glDeleteTextures( 1, &color.tex_name );
                color.tex_name = 0;
            }
            if (tex_depth > 0){
                glDeleteTextures( 1, &tex_depth );
                tex_depth = 0;
            }

            buffer_width = 0;
            buffer_height = 0;
        }

        /* Destructor */
        ~RenderBuffer()
        {
            if (color.tex_name > 0){
                _handle_error_( "Warning! Texture buffer %i is not deleted before destructor call", color.tex_name );
                color.tex_name = 0;
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

        bool check_power_of_2( const int n )
        {
            if (n <= 0){
                return false;
            }

            int y = n;
            int u = 1;
            while (u != 0){
                u = ((y >> 1) & 1);
                y = (y >> 1);
            }

            if (y > 0){
                return false;
            }

            return true;
        }

        /* Generates a frame buffer texture; used for off-screen rendering.
        * The frame buffer is created with a render depth.
        * Make sure that you do not also need a stencil buffer. */
        void generateBuffers
            ( const int _width
            , const int _height
            )
        {_check_gl_error_
            /* Check if it is power of 2 to generate the mipmaps */
            bool mipmaps = false;
            if (check_power_of_2( _width ) && check_power_of_2( _height )){
                mipmaps = true;
            }

            if (color.tex_name > 0){
                glDeleteTextures( 1, &color.tex_name );
                color.tex_name = 0;
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
            color.image.width = _width;
            color.image.height = _height;
            buffer_width = _width;
            buffer_height =_height;

            /* Generate color textures */
            glGenTextures( 1, &color.tex_name ); _check_gl_error_
                glBindTexture( GL_TEXTURE_2D, color.tex_name ); _check_gl_error_
            glTexImage2D
                ( GL_TEXTURE_2D, 0, GL_RGBA8
                , buffer_width, buffer_height
                , 0, GL_BGRA, GL_UNSIGNED_BYTE, 0
                );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            if (mipmaps == true){
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

                /* Generate mipmaps */
                glGenerateMipmapEXT( GL_TEXTURE_2D );
            }
            else{
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            }
            //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

            /* Generate depth texture */
            glGenTextures( 1, &tex_depth ); _check_gl_error_
                glBindTexture( GL_TEXTURE_2D, tex_depth ); _check_gl_error_
            glTexImage2D
            ( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8
            , buffer_width, buffer_height
            , 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0
            ); _check_gl_error_

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

            glBindTexture( GL_TEXTURE_2D, 0 ); _check_gl_error_

            /* Generate depth buffer with stencil */
            glGenRenderbuffers( 1, &rbo_depth ); _check_gl_error_
            glBindRenderbuffer( GL_RENDERBUFFER, rbo_depth ); _check_gl_error_
            glRenderbufferStorage
            ( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, buffer_width, buffer_height ); _check_gl_error_

            glBindRenderbuffer( GL_RENDERBUFFER, 0 ); _check_gl_error_

            /* Generate frame buffer */
            glGenFramebuffers( 1, &fbo ); _check_gl_error_
            glBindFramebuffer( GL_FRAMEBUFFER, fbo ); _check_gl_error_

            glFramebufferRenderbuffer
                ( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT
                , GL_RENDERBUFFER, rbo_depth
                );

            glFramebufferTexture2D
                ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0
                , GL_TEXTURE_2D, color.tex_name, 0
                );

            glFramebufferTexture2D
                ( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT
                , GL_TEXTURE_2D, tex_depth, 0
                );

            /* Unbind */
            glBindTexture( GL_TEXTURE_2D, 0 ); _check_gl_error_
                glBindFramebuffer( GL_FRAMEBUFFER, 0 ); _check_gl_error_

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
            glViewport( 0, 0, buffer_width, buffer_height );
        }

        /* Reads all the pixels from the framebuffer to the CPU memory */
        GLubyte* readFrameBuffer()
        {
            GLubyte* data_color;
            _check_( data_color = (GLubyte*)_malloc_
                ( 4 * buffer_width * buffer_height * sizeof( GLubyte ) ) );

            // bind this frame buffer
            bindFrameBuffer();

            glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
            glReadPixels
                ( 0, 0, buffer_width, buffer_height
                , GL_BGRA, GL_UNSIGNED_BYTE, data_color );

            // restore default framebuffer
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            return data_color;
        }
    };
} // end namespace

#endif 

/**/
