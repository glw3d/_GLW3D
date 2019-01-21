/**
Author: Mario J. Martin <dominonurbs$gmail.com>

This is the basic test to check the OpenGL availability and
compilation problems. Requires GLFW to create the context, while
GLEW is a wrap for to make OpenGL available.
*/

#include <stdio.h>
#include <stdlib.h>

#include "GL/glew.h"
#include <GL/gl.h>
#include "GLFW/glfw3.h"

#include "common/definitions.h"

#include "glw/gwthreads.h"

int rendering = 0;

/* Check the error messages from OpenGL */
GLenum check_gl_error()
{
    GLenum er = glGetError();

    if (er == GL_OUT_OF_MEMORY){
        printf( "OpenGL - Out of memory!\n" );
    }
    else if (er != GL_NO_ERROR){
        printf( "OpenGL - error = %i\n", er );
    }
    return er;
}

/* GLFW error callback, when an error is reported */
static void error_callback( int error, const char* description )
{
    printf( description );
}

GLFWwindow* create_window( const int gl_version )
{
    /* GLFW library must be initialized.
    * This function may only be called from the main thread.
    * Additional calls to this function will do nothing. */
    if (glfwInit() == GL_FALSE){
        printf( "Cannot initialize GLFW!\n" );
        return nullptr;
    }

    glfwMakeContextCurrent( NULL );
    glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_API );

    /* Create the render context */
    GLFWwindow* window = NULL;
    if (gl_version >= 32){
        /* #if __APPLE__ Macs only support core versions of OpenGL */
        printf( "Trying OpenGL 3.2 core...\n" );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        window = glfwCreateWindow( 800, 600, "OpenGL 3.2", NULL, NULL );
    }

    if (window == NULL && gl_version >= 31){
        printf( "Trying OpenGL 3.1 ...\n" );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );
        window = glfwCreateWindow( 800, 600, "OpenGL 3.1", NULL, NULL );
    }

    if (window == NULL && gl_version >= 21){
        /* Trying to create a older version of OpenGL */
        printf( "Trying OpenGL 2.1 ...\n" );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );
        window = glfwCreateWindow( 800, 600, "OpenGL 2.1", NULL, NULL );
    }

    if (window == NULL && gl_version >= 11){
        /* Trying to create a legacy version of OpenGL */
        printf( "Trying OpenGL 1.1 ...\n" );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 1 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );
        window = glfwCreateWindow( 800, 600, "OpenGL 1.1", NULL, NULL );
    }

    if (window == NULL){
        printf( "Cannot create GLFW render window\n" );
        return nullptr;
    }

    glfwMakeContextCurrent( window );

    /* Initialize GLEW */
    glewExperimental = false;
    GLenum err = glewInit();
    if (err != GLEW_OK){
        printf( "GLEW is unable to initialize: %s\n"
            , glewGetErrorString( err ) );
        return nullptr;
    }

    check_gl_error();

    return window;
}

static void key_callback
( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose( window, GL_TRUE );
}

static void window_iconify_callback( GLFWwindow* window, int iconified )
{
    if (iconified){
        // The window was iconified
        rendering = false;
    }
    else{
        // The window was restored
        rendering = true;
    }
}

static void mouse_button_callback
( GLFWwindow* window, int button, int action, int mods )
{
    static double seconds = 0;
    double time_between_clicks = glfwGetTime() - seconds;
    seconds = glfwGetTime();

    if (time_between_clicks < 0.5){
        /* double click */
    }

    if (action == GLFW_PRESS){
        if (button == GLFW_MOUSE_BUTTON_LEFT){
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT){
        }
        if (button == GLFW_MOUSE_BUTTON_MIDDLE){
        }
    }
    else if (action == GLFW_RELEASE){
        if (button == GLFW_MOUSE_BUTTON_LEFT){
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT){
        }
        if (button == GLFW_MOUSE_BUTTON_MIDDLE){
        }
    }
}

static void scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
}

/* Called everytime the window resizes */
static void resize_callback( GLFWwindow* window, int width, int height )
{
}

/* Called everytime the pointer moves */
static void mouse_move_callback( GLFWwindow* window, double xpos, double ypos )
{
}

int open_window( const int gl_version_hint )
{
    int width, height;
    int screen_width = 0, screen_height = 0;

    glfwSetErrorCallback( error_callback );

    GLFWwindow* window = create_window( gl_version_hint );
    if (window == nullptr){
        glfwTerminate();
        getchar();
        return 1;
    }

    glfwGetFramebufferSize( window, &screen_width, &screen_height );

    glViewport( 0, 0, screen_width, screen_height );
    glfwSetInputMode( window, GLFW_STICKY_MOUSE_BUTTONS, 1 );
    glfwSetKeyCallback( window, key_callback );
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetScrollCallback( window, scroll_callback );
    glfwSetWindowSizeCallback( window, resize_callback );
    glfwSetCursorPosCallback( window, mouse_move_callback );

    glfwSetWindowUserPointer( window, malloc( sizeof( int ) ) );
    int* pslot = (int*)glfwGetWindowUserPointer( window );
    if (pslot != nullptr){
        *pslot = 1001;
    }
    /* Make the window's context current */
    glfwMakeContextCurrent( window );

    glClearColor( 0.0f, 0.0f, 1.0f, 0.0f );
    glClearDepth( 1.0f );

    /* Loop until the user closes the window */
    rendering = 1;
    while (!glfwWindowShouldClose( window ))
    {
        glfwSetTime( 0.0 );

        int visible = glfwGetWindowAttrib( window, GLFW_VISIBLE );
        if (rendering != 0 && visible != 0){
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glfwGetFramebufferSize( window, &width, &height );
            if (width != screen_width || height != screen_height){
                /* resize */
                screen_width = width;
                screen_height = height;
            }
            /* Render here */

            /* Swap front and back buffers */
            glfwSwapBuffers( window );
            int time_rendering = int( glfwGetTime() * 1000 );
            if (time_rendering < 50){
                gw_sleep( 50 - time_rendering );
            }
            printf( "." );
            /* Poll for all process events */
            glfwPollEvents();
        }
    }

    glfwDestroyWindow( window );
    glfwTerminate();

    printf( "Window is closed\n" );

    return 0;
}

int main()
{
    open_window( 11 );
    open_window( 21 );
    open_window( 31 );
    open_window( 32 );

    return 0;
}
