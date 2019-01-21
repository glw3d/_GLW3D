/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Functions to handle fonts using freefont library
All calls and data structures are defined here

*******************************************************************************/


#ifndef _HMG_IMPORT_TTF_H
#define _HMG_IMPORT_TTF_H

#include <stdint.h>

#define MAX_FONTS 128

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct
    {
        uint8_t* buffer;
        int width;
        int height;
    }BitmapTag;

    BitmapTag font_draw_bitmap_glyphs
        ( const char* font_filename
        , const int font_size
        , const int* unicode
        );

    /* Loads a font */
    int font_load( const char* font_filename );

    /* Releases memory resources */
    void font_unload( const int slot );

    /* Writes a bitmap with only ascii characters and maybe extended ascii */
    BitmapTag font_write_char
        ( const int ftid, const char* text, const int size );

    /* Writes a bitmap.
     * Unicode characters can be defined as %U+03B1 or by name %{alpha} */
    BitmapTag font_write_richtext
        ( const int ftid, const char* text, const int size );

    /* Release memory resources */
    void BitmapTag_dispose( BitmapTag* bitmap );

#ifdef	__cplusplus
}
#endif

#endif

/**/
