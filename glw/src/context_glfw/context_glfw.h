/**
Author: Mario J. Martin <dominonurbs$gmail.com>

This encapsulates the GLFW context.
GLFW creates a window with the OpenGL context and
handles mouse and keyboard inputs

*******************************************************************************/

#ifndef CONTEXT_GLFW_H
#define CONTEXT_GLFW_H

#include "glw/gwdata.h"

#if defined(__cplusplus)
extern "C" {
#endif

    /* Opens a GLFW window context.
    * Returns nullptr if the context was not able to be created.
    * gl_version_hint is a suggestion of the opengl version; but it might create
    * the context with another version. The behaviour depends on the drivers.
    * if the context is not available, try to create with a lower GL version.
    * Valid numbers are for example 11, 21, 31, 32... */
    int glfwWindow_create_gl_context( const int gl_version_hint );

    /* Request a new GLFW window and GL context.
     * These are not inmediatly created, but when the render begins */
    int glfwWindow_open();

    /* Must be called before rendering. Returns 0 if the window is closed */
    int glfwWindow_render_begin( const int context_id );

    /* Must be called at the end of the rendering to switch the buffers */
    void glfwWindow_render_end();

    /* After calling glfw_render() it waits to get the desired framerate */
    void glfw_setFrameRate( const int framerate );

    /* Calls the render loop. Returns the number of active windows */
    int glfw_render();

    /* Launches the rendering loop in a different thread. */
    void glfw_render_thread();

#if defined(__cplusplus)
}
#endif

#endif
/**/
