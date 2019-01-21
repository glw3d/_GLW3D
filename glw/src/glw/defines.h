#ifndef GW_DEFINES
#define GW_DEFINES

#include "common/definitions.h"

/* Versions of OpenGL*/
#define GW_VERSION_GL4x    41
#define GW_VERSION_GL3x    31
#define GW_VERSION_GL2x    21
#define GW_VERSION_GL1x    11

/* Indicates the primitive of the index stream. */
#define GW_INDEX_POINTS          0x0000
#define GW_INDEX_LINES           0x0001
#define GW_INDEX_LINE_LOOP       0x0002
#define GW_INDEX_LINE_STRIP      0x0003
#define GW_INDEX_TRIANGLES       0x0004
#define GW_INDEX_TRIANGLE_STRIP  0x0005

/* Indicates the type of geometry the shader is especialized */
#define GW_POLYGON_NONE     0   /* there is no especialization */
#define GW_POLYGON_POINT    1   /* dots */
#define GW_POLYGON_LINE     2   /* lines */
#define GW_POLYGON_FILL     4   /* surfaces */

/* Uses no transformations at all.
 * In this mode, the camera only considers the screen (x,y) coordinates */
#define GW_CAMERA_VIEW_SCREEN  0   

/* Uses only to deactivate the screen ratio correction. 
 * Usually should be always activated */
#define GW_CAMERA_VIEW_FLAG_NO_SCREEN_RATIO    1   

/* Used for CAD and LOOK AT to add perspective (field angle) */
#define GW_CAMERA_VIEW_FLAG_PERSPECTIVE   2

/* Similar to the identity matrix, but uses the relation [0, size_in_pixels] 
 * instead [-1, 1], which is more convenient for rendering sprites and icons */
#define GW_CAMERA_VIEW_BLIT      4

/* Used for CAD view. The camera is always looking at the center of the scene */
#define GW_CAMERA_VIEW_CAD       8 

/* Used to show guis that are affected by rotations, but not translation */
#define GW_CAMERA_VIEW_AXIS      16 

/* The camera uses the lookat view. 
Similar to CAD view, but uses a classic set of variables: eye, target, up */
#define GW_CAMERA_VIEW_LOOKAT    32 


#define GW_SHADER_SOLID     1
#define GW_SHADER_WIREFRAME 2
#define GW_SHADER_DOTCLOUD  4
#define GW_SHADER_BLIT      8
#define GW_SHADER_TEXTURE   16
#define GW_SHADER_FONT      32
#define GW_SHADER_PICKING   64
#define GW_SHADER_ISOLINES  128
#define GW_SHADER_VECTOR    256

#define GW_TARGET_SCREEN            0
#define GW_TARGET_PICKING           1
#define GW_TARGET_OFFSCREEN_BUFFER  2

/* Messages */
#define GW_WINDOW_OPEN         1
#define GW_WINDOW_CLOSE        2

/******************************************************************************/

/* A resonable max number of windows. 
 * The only reason to have more than one window is for having multiple monitors
 * or virtual reality headset */
#define _GW_MAX_WINDOWS 8

/* A resonable max number of objects per window */
#define _GW_MAX_CAMERAS 1024
#define _GW_MAX_MESHES 1024

/* Max number of shaders in the pipeline */
#define _GW_SHADERS_PIPELINE_LENGTH 24
#define _GW_NUM_DEFAULT_SHADERS 12

/* After each render waits to adjust the framerate */
#define _GW_DEFAULT_FRAMERATE 24

/******************************************************************************/
/* Each context (GLFW, Qt, Wx) has its own defines.
 * These wrap in one to make applications independant on the context */ 
#define GW_MOUSE_BUTTON_LEFT            1
#define GW_MOUSE_BUTTON_RIGHT           2
#define GW_MOUSE_BUTTON_MIDDLE          4

/* Keyboard pressets */
#define GW_KEY_PRESSED  1
#define GW_KEY_RELEASED  2
#define GW_KEY_REPEAT  3

/* Especial keys */
#define GW_KEY_ESCAPE             256
#define GW_KEY_ENTER              257
#define GW_KEY_TAB                258
#define GW_KEY_BACKSPACE          259
#define GW_KEY_INSERT             260
#define GW_KEY_DELETE             261
#define GW_KEY_LEFT_SHIFT         340
#define GW_KEY_LEFT_CONTROL       341
#define GW_KEY_LEFT_ALT           342
#define GW_KEY_LEFT_SUPER         343
#define GW_KEY_RIGHT_SHIFT        344
#define GW_KEY_RIGHT_CONTROL      345
#define GW_KEY_RIGHT_ALT          346
#endif
