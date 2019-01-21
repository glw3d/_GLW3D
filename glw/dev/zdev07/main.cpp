/**
Author: Mario J. Martin <dominonurbs$gmail.com>

This example shows how to render text.
For this, an external library is required to read the fonts in .ttf format.
The letters are converted in bitmaps and rendered as images.
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "glw/gwgl.h"

#include "context_glfw/context_glfw.h"
#include "glw/glw.h"

#include "font/font.h"

char text[256] = "Write something...\n>";
gwMesh* label = nullptr;
int ftid_regular = 0;

void text_input( gwWindowContext* context, const unsigned int key )
{
    int num_letters = strlen( text );
    if (num_letters >= 255){
        num_letters = 0;
    }
    if ((num_letters % 32) == 0){
        text[num_letters] = '\n';
        num_letters++;
    }
    label->position.x = 300;
    label->position.y = 300;
    label->rotation.z = (float)(rand() % 360);
    label->primary_color.r = rand() % 255;
    label->primary_color.g = rand() % 255;
    label->primary_color.b = rand() % 255;

    text[num_letters] = key;
    num_letters++;
    text[num_letters] = '\0';

    /* Deletes previous texture */
    gwTexture_delete( label->texture0 );
    gwLabel_update( label, ftid_regular, text, 32 );
    context->update = 1;
}

int main()
{
    const int gl_hint = 31;

    /* Create a the OpenGL context using GLFW.*/
    int wid = glfwWindow_open();
    if (wid == 0){
        /* Failed to create the context */
        return 1;
    }
    gwWindowContext* context = gw_getWindowContext( wid );
    context->screen_width = 800;
    context->screen_height = 640;
    context->f_on_keycharpress = text_input;

    /* Load a font */
    ftid_regular = font_load( "../res/fonts/oldstandard/OldStandard-Regular.ttf" );

    /* There is an especific shader for fonts,
    * which is loaded with all defaults with the flag GW_SHADER_FONT */
    int sh = gwShader_load( wid, "../res/shaders", "font.vsh", "font.fsh", nullptr, GW_POLYGON_FILL );

    /* A label is a quad with a texture of the characters;
    * but essentially works as a regular mesh */
    label = gwLabel( ftid_regular, text, 32 );
    label->primary_color = gwColorBlack;
    label->secondary_color = gwColorWhite;
    label->secondary_color.a = 64;
    label->shaders = sh;

    /* We can also create a texture as by calling: gwTexture* gwText(...) 
     * Then we need to create a quad to render the texture */

    /* We need a frame and a camera for rendering */
    gwFrame* frame = gwFrame_create( wid );
    frame->background_color = gwColorGray;
    gwCamera* camera = gwCamera_create();
    camera->type = GW_CAMERA_VIEW_BLIT;
    gwFrame_addCamera( frame, camera );
    gwCamera_addMesh( camera, label );

    glfw_render_thread();

    printf( "Press ENTER to exit\n" );
    getchar();
}

