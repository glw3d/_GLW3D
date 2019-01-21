/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Creates the context to manage the mouse and keyboard inputs for 3d views
*/

#include <stdlib.h>
#include <math.h>

#include "common/log.h"

#include "glw/glw.h"
#include "glw/gwcamera.h"

#define _abs_(a) ( a < 0 ? -a : a)

int ctrl_key_pressed = 0;
gwMesh* focused_mesh = nullptr;

/* A square box drawn for selecting an area of the screen */
gwMesh* selection_box[_GW_MAX_WINDOWS];

/* Rotate the cameras */
static void rotate_camera
( gwCamera* camera, const double delta_x, const double delta_y )
{
    camera->rotation_angles.x += gwFloat( delta_x );
    camera->rotation_angles.z += gwFloat( delta_y );
}

/* Move the cameras */
static void move_camera
( gwCamera* camera
, const int screen_width, const int screen_height
, const double mouse_x, const double mouse_y, const double zdepth )
{
    gwFloat dx = (gwFloat)mouse_x / screen_height;
    gwFloat dy = (gwFloat)mouse_y / screen_height;

    gwVector3f displ = gwCamera_calculate_cad_displacement
        ( camera, 2 * dx, 2 * dy, 2 * (gwFloat)zdepth );

    camera->eye.x += displ.x;
    camera->eye.y += displ.y;
    camera->eye.z += displ.z;
}

static void zoom_camera( gwCamera* camera, double delta_zoom )
{
    camera->zoom += camera->zoom * gwFloat( delta_zoom );
}

static void center_camera
( gwCamera* camera
, const int screen_width, const int screen_height
, const double mouse_x, const double mouse_y, const double zdepth
, const double delta_zoom )
{
    /* Loop through the cameras associated to this frame */
    gwFloat dx = (gwFloat)mouse_x / screen_height;
    gwFloat dy = (gwFloat)mouse_y / screen_height;

    gwVector3f displ = gwCamera_calculate_cad_displacement
        ( camera, 2 * dx, 2 * dy, 2 * (gwFloat)zdepth );

    camera->eye.x += displ.x;
    camera->eye.y += displ.y;
    camera->eye.z += displ.z;
    camera->zoom *= (gwFloat)delta_zoom;
}

static void get_frame_size
( const gwWindowContext* context, const gwFrame* frame
, int* frame_width, int* frame_height 
, int* frame_x0, int* frame_y0 )
{
    double width = context->screen_width;
    double height = context->screen_height;
    double x0 = 0.0f, y0 = 0.0f;
    double x1, y1;
    if (frame != nullptr){
        x0 = frame->x0;
        x1 = frame->x1;
        y0 = frame->y0;
        y1 = frame->y1;

        if (x0 > x1){
            const double temp = x0;
            x0 = x1;
            x1 = temp;
        }
        if (y0 > y1){
            const double temp = y0;
            y0 = y1;
            y1 = temp;
        }

        if (frame->size_in_pixels1_relative0 != 0){
            width = x1 - x0;
            height = y1 - y0;
        }
        else{
            width = context->screen_width * (x1 - x0);
            height = context->screen_height * (y1 - y0);
            x0 *= context->screen_width;
            y0 *= context->screen_height;
        }
    }

    *frame_width = (int)width;
    *frame_height = (int)height;
    *frame_x0 = (int)x0;
    *frame_y0 = (int)y0;
}

static void on_mouse_move
( gwCamera* camera, const int context_id, const gwFrame* frame
, const int button, const int mouse_x, const int mouse_y, const float zdepth
, const int delta_x, const int delta_y )
{
    //int panel_id, vertex_id;
    //float zdepth;
    //gw_picking( context_id, &panel_id, &vertex_id, &zdepth, mx, my );
    //zdepth = gw_get_depth( context_id, mx, my );
    //_trace_( "[%i] %i %i %f\n", context_id, panel_id, vertex_id, zdepth );
    //_trace_( "[%i] %f\n", context_id, zdepth );

    gwWindowContext* context = gw_getWindowContext( context_id );
    if (context == nullptr) return;
    int width, height, fx0, fy0;
    get_frame_size( context, frame, &width, &height, &fx0, &fy0 );

    if (button & GW_MOUSE_BUTTON_RIGHT){
        rotate_camera( camera, -delta_x, delta_y );
    }
    else if (button & GW_MOUSE_BUTTON_MIDDLE){
        move_camera
            ( camera
            , width, height
            , delta_x, delta_y, 0
            );
    }
    else if (ctrl_key_pressed == 1){
        move_camera
            ( camera
            , width, height
            , 0, 0, (double)delta_y / 500
            );
    }

    if (button & GW_MOUSE_BUTTON_LEFT){
    }
}

static void on_mouse_release
( gwCamera* camera, const int context_id, const gwFrame* frame
, const int button, const int mx, const int my, const float zdepth )
{
    gwWindowContext* context = gw_getWindowContext( context_id );
    if (context == nullptr) return;

    int width, height, fx0, fy0;
    get_frame_size( context, frame, &width, &height, &fx0, &fy0 );

    if (button & GW_MOUSE_BUTTON_MIDDLE){
    }

    if (button & GW_MOUSE_BUTTON_LEFT){
    }

    if (button & GW_MOUSE_BUTTON_RIGHT){
    }
}

static void on_mouse_scroll
( gwCamera* camera, const int context_id, const gwFrame* frame
, const double delta_x, const double delta_y )
{
    zoom_camera( camera, 0.05 * delta_y );
}

static void square_zoom
( gwCamera* camera, const int context_id, const gwFrame* frame
, const int mx, const int my
, const int mouse_pressed_coord_x, const int mouse_pressed_coord_y )
{
	gwWindowContext* context = gw_getWindowContext( context_id );
	if ( context == nullptr ) return;

	int width, height, fx0, fy0;
	get_frame_size( context, frame, &width, &height, &fx0, &fy0 );

	int x0 = mouse_pressed_coord_x;
	int y0 = mouse_pressed_coord_y;
	int x1 = mx;
	int y1 = my;

	/* Check a minimun size */
	if ( abs( y1 - y0 ) > 7 && abs( x1 - x0 ) > 7 ){
		/* Get the selection box */
		size_t buffer_len = 0;
		gwFloat* zdepth = gwWindowContext_picking_box_depth
			( context_id, x0, y0, x1, y1, &buffer_len );

		double fx0 = 0;
		double fy0 = 0;
		double fx1 = width;
		double fy1 = height;
		if ( fx0 > fx1 ){
			const double temp = fx0;
			fx0 = fx1;
			fx1 = temp;
		}
		if ( fy0 > fy1 ){
			const double temp = fy0;
			fy0 = fy1;
			fy1 = temp;
		}

		const double fw = fx1 - fx0;
		const double fh = fy1 - fy0;

		/* Adjust the camera position */
		const float xw = (float)( x0 + x1 ) / 2;
		const float yw = (float)( y0 + y1 ) / 2;
		const gwFloat dx = gwFloat( ( xw - fx0 ) - fw / 2 );
		const gwFloat dy = gwFloat( ( yw - fy0 ) - fh / 2 );

		/* Calculate the depth */
		double zm = 0;
		double dz = 0.5;
		double min_diff = 10;
		if ( zdepth != nullptr ){
			size_t n = 0;
			for ( size_t i = 0; i < buffer_len; i++ ){
				float z = zdepth[i];
				if ( z > 0 && z < 1 ){
					zm += z;
					n++;
				}
			}
			if ( n > 0 ){
				zm /= n;
				dz = zm;
				/* Find the closest to the median */
				for ( size_t i = 0; i < buffer_len; i++ ){
					float z = zdepth[i];
					if ( z >0 && z < 1 && _abs_(zm - z) < min_diff ){
						dz = z;
						min_diff = _abs_(zm - z);
					}
				}
			}
		}

		/* Deletes the picking box */
		free( zdepth );

		_trace_( ">>>%f\n", dz );
		/* Calculate the zoom */
		double delta_zoom = 1;
		if ( fx1 != fx0 && fy1 != fy0 ){
			double zx = fw / abs( x1 - x0 );
			double zy = fh / abs( y1 - y0 );
			if ( zx < 0 ) zx = -zx;
			if ( zy < 0 ) zy = -zy;
			if ( zx < zy ){
				delta_zoom = zx;
			}
			else{
				delta_zoom = zy;
			}
		}

		/* Adjust the camera view */
		center_camera( camera
			, context->screen_width, context->screen_height
			, dx, dy, dz - 0.5, delta_zoom );
	}
}

static void on_keyboard
( gwCamera* camera, const int context_id, const gwFrame* frame
, const int key, const int action
, const int mouse_x, const int mouse_y
, const int mouse_pressed_x, const int mouse_pressed_y )
{
    gwWindowContext* context = gw_getWindowContext( context_id );

    if (key == GW_KEY_ESCAPE){
		if ( action == GW_KEY_RELEASED ){
			/* Loop through the cameras associated to this frame */
			gwCamera_autofit( camera );
		}
    }
	else if ( key == GW_KEY_DELETE ){
		if ( action == GW_KEY_RELEASED && focused_mesh != nullptr ){
			focused_mesh->show = 0;
			context->update = 1;
		}
	}
	else if ( key == GW_KEY_RIGHT_SHIFT || key == GW_KEY_LEFT_SHIFT ){
		if ( action == GW_KEY_RELEASED ){
			square_zoom( camera, context_id, frame, mouse_x, mouse_y
				, mouse_pressed_x, mouse_pressed_y );
		}
	}
	if ( key == GW_KEY_LEFT_CONTROL || key == GW_KEY_RIGHT_CONTROL ){
		if ( action == GW_KEY_PRESSED ){
			ctrl_key_pressed = 1;
		}
		else if ( action == GW_KEY_RELEASED ){
			ctrl_key_pressed = 0;
		}
	}
}

static void on_mouse_click
( gwCamera* camera, const int context_id, const gwFrame* frame
, const int button, const int dbl_click, const int mx, const int my, const float zdepth )
{
    gwWindowContext* context = gw_getWindowContext( context_id );
    if (context == nullptr){
        return;
    }

    int frame_x0, frame_y0, frame_width, frame_height;
    int frame_x1 = context->screen_width;
    int frame_y1 = context->screen_height;

    get_frame_size( context, frame, &frame_width, &frame_height, &frame_x0, &frame_y0 );

    if (button & GW_MOUSE_BUTTON_LEFT){
        if (dbl_click == 1){
            /* Adjust the camera position */
            const double xw = (double)frame_x0 + (double)frame_width / 2;
            const double yw = frame_y0 + (double)frame_height / 2;

            /* Move the camera */
            gwFloat delta_x = (gwFloat)((mx - xw) / frame_height);
            gwFloat delta_y = -(gwFloat)((context->screen_height - my - yw) / frame_height);

            gwFloat delta_z = 0;
            if (zdepth < 1 && zdepth > 0){
                delta_z = zdepth - 0.5f;
            }

            gwVector3f displ = gwCamera_calculate_cad_displacement
                ( camera, 2 * delta_x, 2 * delta_y, 2 * delta_z );

            camera->eye.x += displ.x;
            camera->eye.y += displ.y;
            camera->eye.z += displ.z;
        }
    }
    if (button & GW_MOUSE_BUTTON_MIDDLE){
    }
}

extern "C"
void gwCamera_behaviour_cad( gwCamera* camera )
{
    camera->type = GW_CAMERA_VIEW_CAD;
    camera->on_mouse_click = on_mouse_click;
    camera->on_mouse_move = on_mouse_move;
    camera->on_mouse_release = on_mouse_release;
    camera->on_mouse_scroll = on_mouse_scroll;
    camera->on_keyboard = on_keyboard;
}
