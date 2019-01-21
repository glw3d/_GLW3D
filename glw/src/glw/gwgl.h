/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Sets the GL and GLEW headers in the right order

*******************************************************************************/

#ifndef _HGW_GL_H
#define _HGW_GL_H

#define _useGLEW
//#define _useQGL

#if defined(_useGLEW)
#ifndef __GLEW_H__
/*  Important!!!
For a static link with GLES, glew.h *must* be declared before gl.h
*/
#define GLEW_STATIC 

/* Qt do not mix well with GLES and GLEW */
#define QT_NO_OPENGL_ES_2

/* glew.h *must* be defined before gl.h */
#ifdef __APPLE__
#  include "glew.h"
#  include <OpenGL/gl.h>
#else
#  include "GL/glew.h"
#  include <GL/gl.h>
#endif/*__APPLE__*/

#endif
#elif defined(_useQGL)
#define GL_GLEXT_PROTOTYPES
#   include <QtOpenGL/QtOpenGL>

#else
#error Must define interface! _useGLEW or _useQGL
#endif

/* Macro used to check OpenGL error messages. */
#define _check_gl_error_ private_gw_check_gl_error( __LINE__, __FILE__ );

#if defined(__cplusplus)
extern "C" {
#endif

    /* Defined in internal.c */
    int private_gw_check_gl_error( const int line, const char* file );

#if defined(__cplusplus)
}
#endif

/* Check the OpenGL compativility */
#ifdef GLEW_VERSION_3_1
#define GW_GL3x
#endif
#ifdef GLEW_VERSION_2_1
#define GW_GL2x
#endif
#ifdef GLEW_VERSION_1_1
#define GW_GL1x
#endif


#endif

/**/
