/**
Author: Mario J. Martin <dominonurbs$gmail.com>

A frame buffer is basically used for off-screen rendering.

*******************************************************************************/

#ifndef _HGW_FRAMEBUFFER_H
#define _HGW_FRAMEBUFFER_H

#include "common/log.h"
#include "common/check_malloc.h"

#include "signatures.h"
#include "defines.h"
#include "gwdata.h"

#include "Stream.hpp"

#include "RenderBuffer.hpp"
#include "Frame.hpp"

namespace gw
{
    /* */
    class RenderTarget : public gwRenderTarget
    {
    public:
        int signature;
        RenderBuffer render_buffer;

        /* List of cameras to be rendered */
        DynamicStack<Frame> list_frames;

        RenderTarget()
        {
            width = 0;
            height = 0;
            background_color = gwColorBlack;
            color_texture = &(render_buffer.color.image);
            signature = _GW_SIGNATURE_RENDER_TARGET;

            /* Ovewrite the signature of the texture in the render buffer */
            render_buffer.color.signature = _GW_SIGNATURE_RENDER_BUFFER;
        }
        
        /* Checks and updates the buffers if the dimensions have changed */
        void update( const int screen_width, const int screen_height )
        {
            if (width == 0 || height == 0){
                if (screen_width != render_buffer.buffer_width
                    || screen_height != render_buffer.buffer_height)
                {
                    render_buffer.generateBuffers( screen_width, screen_height );
                }
            }
            else{
                if (width != render_buffer.buffer_width
                    || height != render_buffer.buffer_height)
                {
                    render_buffer.generateBuffers( width, height );
                }
            }
        }

        gwViewPort clear()
        {
            GLfloat bgc_r = (GLfloat)background_color.r / 255;
            GLfloat bgc_g = (GLfloat)background_color.g / 255;
            GLfloat bgc_b = (GLfloat)background_color.b / 255;
            GLfloat bgc_a = (GLfloat)background_color.a / 255;
            glClearColor( bgc_r, bgc_g, bgc_b, bgc_a );

            glBindFramebuffer( GL_FRAMEBUFFER, render_buffer.fbo );

            glViewport( 0, 0, render_buffer.buffer_width, render_buffer.buffer_height );
            glScissor( 0, 0, render_buffer.buffer_width, render_buffer.buffer_height );

            GLenum att[] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers( 1, att );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

            gwViewPort viewport;
            viewport.x0 = 0;
            viewport.y0 = 0;
            viewport.width = render_buffer.buffer_width;
            viewport.height = render_buffer.buffer_height;
            viewport.fbo = render_buffer.fbo;

            return viewport;
        }
    };
} /* end namespace */
#endif
