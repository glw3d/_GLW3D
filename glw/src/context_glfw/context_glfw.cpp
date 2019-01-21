/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Creates the context to render with windows created with GLFW.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "GLFW/glfw3.h"

#include "common/definitions.h"
#include "common/log.h"

#include "glw/Stream.hpp"
#include "glw/defines.h"
#include "glw/gwdata.h"
#include "glw/gwcamera.h"
#include "glw/glw.h"
#include "glw/gwthreads.h"

static GLFWwindow* window_slots[_GW_MAX_WINDOWS] = { nullptr };
static int mouse_left_pressed = 0;
static int mouse_right_pressed = 0;
static int mouse_middle_pressed = 0;

/* Used to calculate the displacement of the mouse, when is a button or key is pressed */
static int mouse_pressed_coord_x = 0;
static int mouse_pressed_coord_y = 0;

/* Current mouse coordinates */
static int mouse_coord_x = 0;
static int mouse_coord_y = 0;

static int frame_rate = _GW_DEFAULT_FRAMERATE;

/* Used to identify when a key is clicked first time, while is still pressed */
static int key_is_pressed = 0;

gwFrame* hilighted_frame = nullptr;
gwMesh* hilighted_mesh = nullptr;

/******************************************************************************/

/* Gets the window id from the GLFW window data structure.
* GLFW has a pointer, allowing to insert user data in window. */
static int getWindowSlot( GLFWwindow* window )
{
    if (window == nullptr){
        return -1;
    }

    int* pslot = (int*)glfwGetWindowUserPointer( window );
    if (pslot != nullptr){
        return *pslot;
    }
    else{
        return -1;
    }
}

/* Called at keyboard inputs */
static void key_callback
( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    int gwkey = scancode;
    int gwaction = 0;
    int wid = getWindowSlot( window );
    gwWindowContext* context = gw_getWindowContext( wid );

    if (action == GLFW_PRESS){
        gwaction = GW_KEY_PRESSED;
    }
    else if (action == GLFW_RELEASE){
        gwaction = GW_KEY_RELEASED;
    }
    else if (action == GLFW_REPEAT){
        gwaction = GW_KEY_REPEAT;
    }

    if (key == GLFW_KEY_ESCAPE){
        gwkey = GW_KEY_ESCAPE;
    }
    else if (key == GLFW_KEY_DELETE){
        gwkey = GW_KEY_DELETE;
    }
    else if (key == GLFW_KEY_LEFT_CONTROL){
        gwkey = GW_KEY_LEFT_CONTROL;
    }
	else if ( key == GLFW_KEY_RIGHT_CONTROL ){
		gwkey = GW_KEY_RIGHT_CONTROL;
	}
	else if ( key == GLFW_KEY_RIGHT_SHIFT ){
		gwkey = GW_KEY_RIGHT_SHIFT;
	}
	else if ( key == GLFW_KEY_LEFT_SHIFT ){
		gwkey = GW_KEY_LEFT_SHIFT;
	}

	double mx, my;
	glfwGetCursorPos( window, &mx, &my );
	int mouse_x = (int)mx;
	int mouse_y = (int)my;

	if ( action == GLFW_PRESS && key_is_pressed == 0){
		if ( key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT ){
			mouse_pressed_coord_x = mouse_x;
			mouse_pressed_coord_y = mouse_y;
		}
	}

	if ( action == GLFW_PRESS && key_is_pressed == 0 ){
		key_is_pressed = 1;
	}
	if ( action == GLFW_RELEASE && key_is_pressed == 1 ){
		key_is_pressed = 0;
	}

    /* Detect the frame */
	gwFrame* frame = gw_frame_from_mouse_coord( wid, mouse_x, mouse_y );

    /* Iterate through linked cameras */   
    DynamicStack<gwCamera>* token = (DynamicStack<gwCamera>*)gw_window_getCameraList( wid );
    while (token != nullptr){
        gwCamera* camera = token->obj;
        if (camera != nullptr){
            if (camera->boundary_frame == nullptr 
                || camera->boundary_frame == hilighted_frame)
            {
				if ( action == GLFW_PRESS){
					camera->on_keyboard( camera, wid, 0, gwkey, gwaction
						, mouse_x, mouse_y
						, mouse_pressed_coord_x, mouse_pressed_coord_y );
				}
				else if ( action == GLFW_RELEASE ){
					camera->on_keyboard( camera, wid, 0, gwkey, gwaction
						, mouse_x, mouse_y
						, mouse_pressed_coord_x, mouse_pressed_coord_y );
				}
			}
        }
        token = token->next;
    }
}

/* Called at keyboard inputs */
static void keychar_callback
( GLFWwindow* window, unsigned int codepoint )
{
    int wid = getWindowSlot( window );
    gwWindowContext* context = gw_getWindowContext( wid );

    if (context->f_on_keycharpress != nullptr){
        context->f_on_keycharpress( context, codepoint );
    }
}

/* Called at mouse button input */
static void mouse_button_callback
( GLFWwindow* window, int glfwbutton, int action, int mods )
{
    glfwMakeContextCurrent( window );

    int wid = getWindowSlot( window );

	double mx, my;
	glfwGetCursorPos( window, &mx, &my );
	int mouse_x = (int)mx;
	int mouse_y = (int)my;

    gwWindowContext* context = gw_getWindowContext( wid );
    if (context == nullptr) return;
    context->update = 1;

	hilighted_frame = gw_frame_from_mouse_coord( wid, mouse_x, mouse_y );

    /* Picking */
    int vertex_id;
    float zdepth;
    gwMesh* picked_mesh = gwWindowContext_picking
		( wid, &vertex_id, &zdepth, mouse_x, mouse_y );

    int double_click = 0;
    if (action == GLFW_PRESS){
        if (mouse_left_pressed == 0 && mouse_right_pressed == 0){
        }

        static clock_t click_event_time = -1;
        clock_t time = clock();
        float click_interval = (float)(time - click_event_time);
        click_event_time = time;
        click_interval /= CLOCKS_PER_SEC;
        double_click = (click_interval < 0.25) ? 1 : 0;

		mouse_pressed_coord_x = (int)mouse_x;
		mouse_pressed_coord_y = (int)mouse_y;

        int button = 0;
        if (glfwbutton == GLFW_MOUSE_BUTTON_LEFT){
            mouse_left_pressed = 1;
            button = GW_MOUSE_BUTTON_LEFT;
        }
        else if (glfwbutton == GLFW_MOUSE_BUTTON_RIGHT){
            mouse_right_pressed = 1;
            button = GW_MOUSE_BUTTON_RIGHT;
        }
        else if( glfwbutton == GLFW_MOUSE_BUTTON_MIDDLE ){
            mouse_middle_pressed = 1;
            button = GW_MOUSE_BUTTON_MIDDLE;
        }

        /* Detect the frame */
		gwFrame* frame = gw_frame_from_mouse_coord( wid, mouse_x, mouse_y );

        /* Trigger the mesh event */
        if (picked_mesh != nullptr && picked_mesh->f_on_mouse_click != nullptr){
            picked_mesh->f_on_mouse_click( picked_mesh, vertex_id, button );
        }

        /* Iterate through linked cameras */
        DynamicStack<gwCamera>* token 
            = (DynamicStack<gwCamera>*)gw_window_getCameraList( wid );

        while (token != nullptr){
            gwCamera* camera = token->obj;
            if (camera != nullptr){
                if (camera->boundary_frame == nullptr
                    || camera->boundary_frame == hilighted_frame)
                {
                    if (camera->on_mouse_click != nullptr){
                        camera->on_mouse_click
                            ( camera, wid, frame, button, double_click
							, (int)mouse_x, (int)mouse_y, zdepth );
                    }
                }
            }
            token = token->next;
        }

    }
    else if (action == GLFW_RELEASE){
        int button = 0;
        if (glfwbutton == GLFW_MOUSE_BUTTON_LEFT){
            mouse_left_pressed = 0;
            button = GW_MOUSE_BUTTON_LEFT;
        }
        else if (glfwbutton == GLFW_MOUSE_BUTTON_RIGHT){
            mouse_right_pressed = 0;
            button = GW_MOUSE_BUTTON_RIGHT;
        }
        else if (glfwbutton == GLFW_MOUSE_BUTTON_MIDDLE){
            mouse_middle_pressed = 0;
            button = GW_MOUSE_BUTTON_MIDDLE;
        }

        /* Detect the frame */
        gwFrame* frame = gw_frame_from_mouse_coord( wid, mouse_x, mouse_y );

        /* Trigger the mesh event */
        if (picked_mesh != nullptr && picked_mesh->f_on_mouse_release != nullptr){
            picked_mesh->f_on_mouse_release( picked_mesh, vertex_id, button );
        }

        /* Iterate through linked cameras */
        DynamicStack<gwCamera>* token = (DynamicStack<gwCamera>*)gw_window_getCameraList( wid );
        while (token != nullptr){
            gwCamera* camera = token->obj;
            if (camera != nullptr){
                if (camera->boundary_frame == nullptr
                    || camera->boundary_frame == hilighted_frame)
                {
                    if (camera->on_mouse_release != nullptr){
                        camera->on_mouse_release
							( camera, wid, hilighted_frame, button, mouse_x, mouse_y, zdepth );
                    }
                }
            }
            token = token->next;
        }
    }
}

/* Called when the mouse wheel input */
static void scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
    glfwMakeContextCurrent( window );

    int wid = getWindowSlot( window );
    gwWindowContext* context = gw_getWindowContext( wid );
    if (context == nullptr) return;

    context->update = 1;

    /* Detect the frame */
	double mx, my;
	glfwGetCursorPos( window, &mx, &my );
	int mouse_x = (int)mx;
	int mouse_y = (int)my;

	gwFrame* frame = gw_frame_from_mouse_coord( wid, mouse_x, mouse_y );

    /* Iterate through linked cameras */
    DynamicStack<gwCamera>* token = (DynamicStack<gwCamera>*)gw_window_getCameraList( wid );
    while (token != nullptr){
        gwCamera* camera = token->obj;
        if (camera != nullptr){
            if (camera->boundary_frame == nullptr
                || camera->boundary_frame == hilighted_frame)
            {
                camera->on_mouse_scroll( camera, wid, 0, xoffset, yoffset );
            }
        }
        token = token->next;
    }
}

/* Called when the window resizes */
static void resize_callback( GLFWwindow* window, int winwidth, int winheight )
{
    glfwMakeContextCurrent( window );

    int wid = getWindowSlot( window );
    gwWindowContext* context = gw_getWindowContext( wid );
    int fbwidth = winwidth, fbheight = winheight;
    glfwGetFramebufferSize( window, &fbwidth, &fbheight );
    if (context != nullptr){
        context->screen_width = fbwidth;
        context->screen_height = fbheight;
        gwWindowContext_regenerate_framebuffers( context );
        context->update = 1;
    }
}

/* Called when the pointer moves */
static void mouse_move_callback( GLFWwindow* window, double xpos, double ypos )
{
    int mouse_x = (int)xpos;
    int mouse_y = (int)ypos;

    glfwMakeContextCurrent( window );

    int screen_width = 0, screen_height = 0;
    int wid = getWindowSlot( window );

    glfwGetWindowSize( window, &screen_width, &screen_height );

    gwWindowContext* context = gw_getWindowContext( wid );
    context->update = 1;

    int pressed_buttons = mouse_left_pressed ? GW_MOUSE_BUTTON_LEFT : 0;
    pressed_buttons += mouse_right_pressed ? GW_MOUSE_BUTTON_RIGHT : 0;
    pressed_buttons += mouse_middle_pressed ? GW_MOUSE_BUTTON_MIDDLE : 0;

    const int delta_x = mouse_coord_x - mouse_x;
    const int delta_y = mouse_coord_y - mouse_y;
    mouse_coord_x = mouse_x;
    mouse_coord_y = mouse_y;

    /* Detect the frame */
    gwFrame* frame = gw_frame_from_mouse_coord( wid, mouse_x, mouse_y );

    /* Picking */
    int vertex_id;
    float zdepth;
    gwMesh* picked_mesh = gwWindowContext_picking
        ( wid, &vertex_id, &zdepth, mouse_coord_x, mouse_coord_y );

    if (picked_mesh != hilighted_mesh){
        /* Trigger the mesh event */
        if (picked_mesh != nullptr){
            if (picked_mesh->f_on_mouse_enter != nullptr){
                picked_mesh->f_on_mouse_enter( picked_mesh, vertex_id, pressed_buttons );
            }
        }
        if (hilighted_mesh != nullptr && hilighted_mesh->f_on_mouse_leave != nullptr){
            hilighted_mesh->f_on_mouse_leave( hilighted_mesh, vertex_id, pressed_buttons );
        }
        hilighted_mesh = picked_mesh;
    }

    /* Iterate through linked cameras */
    DynamicStack<gwCamera>* token = (DynamicStack<gwCamera>*)gw_window_getCameraList( wid );
    while (token != nullptr){
        gwCamera* camera = token->obj;
        if (camera != nullptr){
            if (camera->boundary_frame == nullptr
                || camera->boundary_frame == hilighted_frame)
            {
                if (camera->on_mouse_move != nullptr){
                    camera->on_mouse_move
                        ( camera, wid, hilighted_frame, pressed_buttons
                        , mouse_x, mouse_y, zdepth, delta_x, delta_y );
                }
            }
        }
        token = token->next;
    }
}

static void refresh_callback( GLFWwindow* window )
{
    glfwMakeContextCurrent( window );

    int wid = getWindowSlot( window );
    gwWindowContext* context = gw_getWindowContext( wid );
    context->update = 1;
}

/* Called when the window has been minimized */
static void window_iconify_callback( GLFWwindow* window, int iconified )
{
    glfwMakeContextCurrent( window );

    int wid = getWindowSlot( window );
    gwWindowContext* context = gw_getWindowContext( wid );
    if (context == nullptr) return;

    if (iconified){
        /* The window has been minimized */
        context->is_rendering = false;
    }
    else{
        /* The window has been restored */
        context->is_rendering = true;
        context->update = 1;
    }
}

static void setCallbacks( GLFWwindow* window )
{
    glfwSetInputMode( window, GLFW_STICKY_MOUSE_BUTTONS, 1 );
    glfwSetKeyCallback( window, key_callback );
    glfwSetCharCallback( window, keychar_callback );
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetScrollCallback( window, scroll_callback );
    glfwSetWindowSizeCallback( window, resize_callback );
    glfwSetCursorPosCallback( window, mouse_move_callback );
    glfwSetWindowRefreshCallback( window, refresh_callback );
    glfwSetWindowIconifyCallback( window, window_iconify_callback );
}

/* Opens a new window */
static GLFWwindow* create_window
( const int gl_version_hint, const int width, const int height )
{
    int gl_version = 0;
    int mayor = int( gl_version_hint / 10 );
    int minor = gl_version_hint % 10;

    /* GLFW library must be initialized.
    * This function may only be called from the main thread.
    * Additional calls to this function will do nothing. */

    glfwMakeContextCurrent( NULL );

    if (glfwInit() == GL_FALSE){
        _handle_error_( "Cannot initialize GLFW!" );
        return nullptr;
    }

    glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    //glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_API );

    /* Create the render context */
    GLFWwindow* window = NULL;
    if (gl_version_hint > 0){
        _trace_( "Trying OpenGL %i.%i ...\n", mayor, minor );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, mayor );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, minor );
        //glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        //glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        window = glfwCreateWindow( width, height, "Overview", NULL, NULL );
    }
    else{
        /* Don't care */
        window = glfwCreateWindow( width, height, "Overview", NULL, NULL );
    }
    if (window == NULL && gl_version_hint >= 32){
        /* #if __APPLE__ Macs only support core versions of OpenGL */
        _log_( "Trying OpenGL 3.2 core...\n" );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
        //glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        //glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        window = glfwCreateWindow( width, height, "Overview", NULL, NULL );
    }
    if (window == NULL && gl_version_hint >= 31){
        _log_( "Trying OpenGL 3.1 ...\n" );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        //glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        //glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );
        window = glfwCreateWindow( width, height, "Overview", NULL, NULL );
    }

    if (window == NULL && gl_version_hint >= 21){
        /* Trying to create a older version of OpenGL */
        _log_( "Trying OpenGL 2.1 legacy...\n" );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );
        window = glfwCreateWindow( width, height, "Overview", NULL, NULL );
    }

    if (window == NULL && gl_version_hint >= 11){
        /* Trying to create a legacy version of OpenGL */
        _log_( "Trying OpenGL 1.1 legacy...\n" );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 1 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );
        window = glfwCreateWindow( width, height, "Overview", NULL, NULL );
    }

    if (window == NULL){
        printf( "Cannot create GLFW render window\n" );
        return nullptr;
    }

    glfwMakeContextCurrent( window );

    return window;
}

/* Creates a new GLFW window and context */
static
int glfw_create_gl_context( int context_id, const int gl_version_hint )
{
    if (context_id <= 0){
        return 0;
    }

    gwWindowContext* context = gw_getWindowContext( context_id );
    GLFWwindow* window = create_window
        ( gl_version_hint, context->screen_width, context->screen_height );

    if (window == NULL){
        /* Failed to create the OpenGL context */
        return 1;
    }

    /* Initialize GLEW. */
    int gl_version = gw_initialize_glew( context_id );

    setCallbacks( window );
    window_slots[context_id] = window;
    int* pointer = new int;
    *pointer = context_id;
    glfwSetWindowUserPointer( window, pointer );

    int fbwidth, fbheight;
    glfwGetFramebufferSize( window, &fbwidth, &fbheight );
    context->is_rendering = 1;
    context->screen_width = fbwidth;
    context->screen_height = fbheight;

    return 0;
}

/* Closes the window and releases buffers associated to this GL context */
static
void glfw_destroy_gl_context( const int wid )
{
    GLFWwindow* window = window_slots[wid];
    gwWindowContext* context = gw_getWindowContext(wid);
    gwWindowContext_dispose( context );
    int* pslot = (int*)glfwGetWindowUserPointer( window );
    delete (pslot);
    glfwDestroyWindow( window );
    window_slots[wid] = nullptr;
}

/******************************************************************************/

/* Creates a new GLFW window and GL context */
extern "C"
int glfwWindow_create_gl_context( const int gl_version_hint )
{
    int context_id = gw_requestWindowContext();
    glfw_create_gl_context( context_id, gl_version_hint );

    gwWindowContext* context = gw_getWindowContext( context_id );
    gwWindowContext_regenerate_framebuffers( context );

    return context_id;
}

/* Request a new GLFW window and GL context, which are created at render time */
extern "C"
int glfwWindow_open()
{
    int context_id = gw_requestWindowContext();
    gwWindowContext* context = gw_getWindowContext( context_id );
    context->action = GW_WINDOW_OPEN;
    context->update = 1;
    
    return context_id;
}

/* Check the window status. Returns 1 if the window is active */
extern "C"
int glfwWindow_render_begin( const int context_id )
{
    /* Capture the events, such as mouse, keyboard, window resize, etc... */
    glfwPollEvents();

    gwWindowContext* context = gw_getWindowContext( context_id );
    if (context == nullptr){
        return 0;
    }

    /* Check if the context is required to be created */
    if (context->action == GW_WINDOW_OPEN){
        context->action = 0;
        if (window_slots[context_id] == nullptr){
            glfw_create_gl_context( context_id, context->gl_version_hint );
            gwWindowContext_regenerate_framebuffers( context );
        }
    }

    GLFWwindow* window = window_slots[context_id];
    if (window == nullptr){
        return 0;
    }

    glfwMakeContextCurrent( window );

    /* Check if the window has been closed */
    int status = glfwWindowShouldClose( window );
    if (status == 1 || context->action == GW_WINDOW_CLOSE){
        glfw_destroy_gl_context( context_id );
        return 0;
    }

    /* Check if the user has changed the screen size */
    int fbwidth, fbheight;
    glfwGetFramebufferSize( window, &fbwidth, &fbheight );
    if (fbwidth != context->screen_width || fbheight != context->screen_height){
        glfwSetWindowSize
            ( window, context->screen_width, context->screen_height );
        gwWindowContext_regenerate_framebuffers( context );
        context->update = 1;
    }

    /* Clear the screen */
    gwWindowContext_clear( context );

    return 1;
}

/* Updates the screen and waits to adjust the frame rate*/
extern "C"
void glfwWindow_render_end()
{
    for (int wid = 1; wid < _GW_MAX_WINDOWS; wid++){
        GLFWwindow* window = window_slots[wid];
        if (window != nullptr){
            /* Swap front and back buffers */
            glfwSwapBuffers( window );
        }
    }
}

/* Sets the desired frame rate */
extern "C"
void glfw_setFrameRate( const int fr )
{
    frame_rate = fr;
}

/* Calls the render loop */
extern "C"
int glfw_render()
{
    int num_windows = 0;
    double render_time_init = glfwGetTime();

    for (int wid = 1; wid < _GW_MAX_WINDOWS; wid++){
        num_windows += glfwWindow_render_begin( wid );
        gwWindowContext* context = gw_getWindowContext( wid );
        if (context != nullptr && context->update != 0){
            gwWindowContext_draw( wid );
            context->update = 0;

            GLFWwindow* window = window_slots[wid];
            if (window != nullptr){
                glfwSwapBuffers( window );
            }
        }
    }

    /* Sleeps to a maximun framerate (to avoid overheat the GPU) */
    const int miliseconds = 1000 / frame_rate;
    double time_end = glfwGetTime();
    int time_rendering = int( (time_end - render_time_init) * 1000 );
    if (time_rendering < miliseconds){
        gw_sleep( miliseconds - time_rendering );
    }

    return num_windows;
}

/* Launches the rendering loop in a different thread. */
extern "C"
void glfw_render_thread( const int frame_rate )
{
    gw_launch_thread( [](){ while (glfw_render() > 0); } );
}


/******************************************************************************/


