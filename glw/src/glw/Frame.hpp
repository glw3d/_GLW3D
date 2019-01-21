/**
Author: Mario J. Martin <dominonurbs$gmail.com>

A frame is basically a square region inside the window with different
renders or views. When a window is created a default frame is created.
A screen can be composed by several frames that might overlap each other.
When a window is created, it returns the default frame associated.

A mesh or a geometry can be associated to several frames, but watch out that
all frames are created withing the same OpenGL context.

*******************************************************************************/

#ifndef _HGW_FRAME_H
#define _HGW_FRAME_H

#include "common/log.h"
#include "common/check_malloc.h"

#include "signatures.h"
#include "defines.h"
#include "gwdata.h"

#include "Camera.hpp"

namespace gw
{
    /* */
    class Frame : public gwFrame
    {
    public:
        int signature;

        /* List of cameras to be rendered */
        DynamicStack<Camera> render_list;

        Frame()
        {
            signature = _GW_SIGNATURE_FRAME;

            active = 1;
            size_in_pixels1_relative0 = 0;
            x1 = 1;
            y1 = 1;
            x0 = 0;
            y0 = 0;
            preference = 0;
            background_color.r = 0;
            background_color.g = 0;
            background_color.b = 0;
            background_color.a = 0;
        }

        gwViewPort calculate_viewport( const gwViewPort parent_viewport )
        {
            gwViewPort viewport;

            /* Set viewport */
            if (size_in_pixels1_relative0 != 0){
                /* The definition is directly in pixels */
                int xx0 = (int)x0;
                int yy0 = (int)y0;
                int width = (int)(x1 - x0);
                int height = (int)(y1 - y0);

                viewport.x0 = xx0;
                viewport.y0 = yy0;
                viewport.width = width;
                viewport.height = height;
            }
            else{
                /* The definition is relative to the size of the screen */
                int xx0 = (int)(parent_viewport.width * x0);
                int yy0 = (int)(parent_viewport.height * y0);
                int width = (int)(parent_viewport.width * (x1 - x0));
                int height = (int)(parent_viewport.height * (y1 - y0));

                viewport.x0 = xx0;
                viewport.y0 = yy0;
                viewport.width = width;
                viewport.height = height;
            }

            return viewport;
        }

        gwViewPort clear( gwViewPort parent_viewport )
        {
            /* It is neccesary to return the viewport for the camera */
            gwViewPort viewport;
            
            GLfloat bgc_r = (GLfloat)background_color.r / 255;
            GLfloat bgc_g = (GLfloat)background_color.g / 255;
            GLfloat bgc_b = (GLfloat)background_color.b / 255;
            GLfloat bgc_a = (GLfloat)background_color.a / 255;
            glClearColor( bgc_r, bgc_g, bgc_b, bgc_a );

            viewport = calculate_viewport( parent_viewport );
            viewport.fbo = parent_viewport.fbo;

            glViewport( viewport.x0, viewport.y0, viewport.width, viewport.height );
            glScissor( viewport.x0, viewport.y0, viewport.width, viewport.height );

            if (background_color.a > 0){
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            }
            else{
                glClear( GL_DEPTH_BUFFER_BIT );
            }

            return viewport;
        }
    };
} /* end namespace */

#endif /* _HGW_FRAME_H */

