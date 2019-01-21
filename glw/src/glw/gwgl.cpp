/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Routines to handle traces, infos, warnings, and error messages.
It provides function pointers to allow to capture them from applications.
*/

#include "common/log.h"
#include "gwgl.h"

/* Check the error messages from OpenGL */
int private_gw_check_gl_error( const int line, const char* file )
{
    GLenum er = glGetError();

    if (er == GL_OUT_OF_MEMORY){
        log_handle_error( line, file, "OpenGL - Out of memory!" );
    }
    else if (er != GL_NO_ERROR){
        log_handle_error( line, file, "OpenGL - error = %i", er );
    }
    return er;
}
