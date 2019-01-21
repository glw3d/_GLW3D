/***
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/


#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "common/definitions.h"
#include "common/check_malloc.h"
#include "common/log.h"

#include "font.h"
#include "richtext.h"

/* Globals that store the fonts read from Free Type */
FT_Library ft_library[MAX_FONTS] = { nullptr };
FT_Face ft_face[MAX_FONTS] = { nullptr };


static void draw_glyph
( uint8_t* bitmap
, const FT_Face face
, const FT_UInt unicode
, const int bitmap_index
, const int glyph_size
, const int texture_size
)
{
    FT_UInt glyph_index = FT_Get_Char_Index( face, unicode );
    FT_GlyphSlot  slot;
    int  i, j;
    const unsigned char* pbuffer = nullptr;
    const int row = (int)(bitmap_index / 16);
    const int col = (int)(bitmap_index % 16);
    uint8_t* ptexture = nullptr;
    int ipixel;

    FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
    ipixel = texture_size
        * (glyph_size*row + glyph_size - face->glyph->bitmap_top - 1)
        + col*glyph_size;

    if (face == nullptr || face->glyph->bitmap.buffer == nullptr){
        return;
    }

    slot = face->glyph;
    pbuffer = slot->bitmap.buffer;

    ptexture = &(bitmap[ipixel]);
    for (j = 0; j < (int)face->glyph->bitmap.rows && j < glyph_size; j++){
        uint8_t* p = ptexture;
        for (i = 0; i < (int)face->glyph->bitmap.width; i++){
            if (p < bitmap + texture_size * texture_size){
                *p = (uint8_t)(*pbuffer);
                p++;
            }
            pbuffer++;
        }
        ptexture += texture_size;
    }
}

/* Creates an image of characteres in a 16x16 grid glyphs */
BitmapTag font_draw_bitmap_glyphs
( const char* font_filename
, const int font_size
, const int* unicode
)
{
    BitmapTag bitmap;
    FT_Library    library;
    FT_Face       face;
    FT_Error      error;
    int glyph_size = 0;
    int size = font_size;
    if (size < 0){
        size = 1;
    }
    else if (size > 64){
        size = 64;
    }

    if (size <= 16){
        _check_( bitmap.buffer
            = (uint8_t*)_calloc_( 16 * 16 * 256, sizeof( uint8_t ) ) );
        bitmap.width = 16 * 16;
        bitmap.height = 16 * 16;
        glyph_size = 16;
    }
    else if (size <= 32){
        _check_( bitmap.buffer 
            = (uint8_t*)_calloc_( 32 * 32 * 256, sizeof( uint8_t ) ) );
        bitmap.width = 16 * 32;
        bitmap.height = 16 * 32;
        glyph_size = 32;
    }
    else{ // (size <= 64)
        _check_( bitmap.buffer
            = (uint8_t*)calloc( 64 * 64 * 256, sizeof( uint8_t ) ) );
        bitmap.width = 16 * 64;
        bitmap.height = 16 * 64;
        glyph_size = 64;
    }

    /* initialize library */
    error = FT_Init_FreeType( &library );
    if (error){
        _handle_error_( "Cannot initialize FreeType!" );
        return bitmap;
    }

    error = FT_New_Face( library, font_filename, 0, &face );
    if (error == FT_Err_Unknown_File_Format){
        _handle_error_( "Font file type is not supported! %s", font_filename );
        return bitmap;
    }
    else if (error){
        _handle_error_( "Cannot open font file! %s", font_filename );
        return bitmap;
    }

    error = FT_Set_Char_Size( face, 0, size * 16, 300, 300 );
    if (error){
        _handle_error_( "Font size %i is not supported! %s", size, font_filename );
    }

    if (unicode == nullptr){
        int i;
        for (i = 0; i < 256; i++){
            draw_glyph( bitmap.buffer, face, i, i, glyph_size, bitmap.width );
        }
    }
    else{
        int i;
        for (i = 0; i < 256 && unicode[i] != 0; i++){
            draw_glyph( bitmap.buffer, face, unicode[i], i, glyph_size, bitmap.width );
        }
    }

    FT_Done_Face( face );
    FT_Done_FreeType( library );

    return bitmap;
}

static int find_available_slot()
{
    int i;
    for (i = 1; i < MAX_FONTS; i++){
        if (ft_library[i] == nullptr){
            return i;
        }
    }

    /* The 0 slot is reserved to null */
    return 0;
}

/* Loads a font. Returns a font id and 0 if fails */
int font_load( const char* font_path )
{
    FT_Library    library;
    FT_Face       face;
    FT_Error      error;

    /* Find an available slot */
    int slot = find_available_slot();
    if (slot <= 0){
        _handle_error_( "There is a maximun of %i font types available!"
            , MAX_FONTS );
        return 0;
    }

    /* initialize library */
    error = FT_Init_FreeType( &library );
    if (error){
        _handle_error_( "Cannot initialize FreeType!" );
        return 0;
    }

    error = FT_New_Face( library, font_path, 0, &face );
    if (error == FT_Err_Unknown_File_Format){
        _handle_error_( "Font file type is not supported! %s", font_path );
        return 0;
    }
    else if (error){
        _handle_error_( "Cannot open font file! %s", font_path );
        return 0;
    }

    ft_library[slot] = library;
    ft_face[slot] = face;

    return slot;
}

/* Release memory resources */
void font_unload( const int slot )
{
    if (slot > 0 && slot < MAX_FONTS){
        if (ft_face[slot] != nullptr){
            FT_Done_Face( ft_face[slot] );
            ft_face[slot] = nullptr;
        }
        if (ft_library[slot] != nullptr){
            FT_Done_FreeType( ft_library[slot] );
            ft_library[slot] = nullptr;
        }
    }
}

static void write_glyph
    ( uint8_t* bitmap
    , const FT_Face face
    , const int bitmap_width
    )
{
    FT_GlyphSlot glyph = face->glyph;
    uint8_t* pbitmap = bitmap;
    int i, j;
    if (glyph->bitmap.buffer == nullptr){
        return;
    }

    uint8_t* pbuffer = glyph->bitmap.buffer;
    for (j = 0; j < (int)face->glyph->bitmap.rows; j++){
        uint8_t* p = pbitmap;
        for (i = 0; i < (int)face->glyph->bitmap.width; i++){
            *p = (uint8_t)(*pbuffer);
            p++;
            pbuffer++;
        }
        pbitmap += bitmap_width;
    }
}

/* Creates a bitmap with the text */
BitmapTag font_write_char( const int ftid, const char* text, const int font_size )
{
    BitmapTag bitmap;
    int size = font_size < 0 ? -font_size : font_size;
    int invert = font_size < 0 ? 1 : 0;
    int x = 0;
    int pen_x = 0;
    int pen_y = size + size / 2;
    const char* pt = text;
    int ichar = 0;
    const double line_space = 1.25;
    FT_Face face;
    FT_Error error;

    bitmap.buffer = nullptr;
    bitmap.width = 0;
    bitmap.height = 0;

    if (text == nullptr){
        return bitmap;
    }

    if (ftid <= 0 || ftid >= MAX_FONTS || ft_library[ftid] == nullptr){
        return bitmap;
    }

    /* Set the font size */
    face = ft_face[ftid];
    error = FT_Set_Char_Size( face, 0, size * 16, 300, 300 );
    if (error){
        _handle_error_( "Font size %i is not supported!", size );
        return bitmap;
    }

    /* Calculates the required dimensions */
    while (*pt != '\0')
    {
        ichar++;
        if (*pt == '\n'){
            pen_y += (int)(size * line_space);
            if (pen_x < x){
                pen_x = x;
            }
            x = 0;
            pt++;
        }
        else if (*pt == '\t'){
            FT_UInt glyph_index = FT_Get_Char_Index( face, ' ' );
            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
            int space_size = face->glyph->advance.x >> 6;
            int tab_advance = x % (4 * space_size);
            if (tab_advance == 0) tab_advance = 4 * space_size;
            x += tab_advance;
            pt++;
        }
        else if ((unsigned char)(*pt) < ' '){
            /* Ignore this character */
            pt++;
        }
        else{
            FT_UInt glyph_index = FT_Get_Char_Index( face, (unsigned char)(*pt) );
            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
            x += (face->glyph->advance.x >> 6);
            pt++;
        }
    }

    if (pen_x < x){
        pen_x = x;
    }

    if (pen_x == 0){
        return bitmap;
    }

    /* Reserve memory */
    bitmap.buffer = (uint8_t*)calloc( pen_x * pen_y, sizeof( uint8_t ) );
    bitmap.width = pen_x;
    bitmap.height = pen_y;

    /* Draw the bitmap */
    pen_x = 0;
    pen_y = 0;
    pt = text;
    while (*pt != '\0')
    {
        ichar++;
        if (*pt == '\n'){
            pen_y += (int)(size * line_space);
            pen_x = 0;
            pt++;
        }
        else if (*pt == '\t'){
            FT_UInt glyph_index = FT_Get_Char_Index( face, ' ' );
            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
            int space_size = face->glyph->advance.x >> 6;
            int tab_advance = pen_x % (4 * space_size);
            if (tab_advance == 0) tab_advance = 4 * space_size;
            pen_x += tab_advance;
            pt++;
        }
        else if ((unsigned char)(*pt) < ' '){
            /* Ignore this character */
                pt++;
            }
        else{
            FT_UInt glyph_index = FT_Get_Char_Index( face, (unsigned char)(*pt) );
            int offset = pen_y * bitmap.width + pen_x;

            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );

            /* Align the vertical row */
            offset += size - face->glyph->bitmap_top * bitmap.width;
            write_glyph( &(bitmap.buffer[offset]), face, bitmap.width );

            pen_x += (face->glyph->advance.x >> 6);
            pt++;
        }
    }

    return bitmap;
}


/* Creates a bitmap with the text */
BitmapTag font_write_richtext( const int ftid, const char* text, const int font_size )
{
    BitmapTag bitmap;
    int size = font_size < 0 ? -font_size : font_size;
    int x = 0;
    int pen_x = 0;
    int pen_y = size + size / 2;
    const char* pt = text;
    int ichar = 0;
    const double line_space = 1.25;
    FT_Face face;
    FT_Error error;

    bitmap.buffer = nullptr;
    bitmap.width = 0;
    bitmap.height = 0;

    if (text == nullptr){
        return bitmap;
    }

    if (ftid <= 0 || ftid >= MAX_FONTS || ft_library[ftid] == nullptr){
        return bitmap;
    }

    /* Set the font size */
    face = ft_face[ftid];
    error = FT_Set_Char_Size( face, 0, size * 16, 300, 300 );
    if (error){
        _handle_error_( "Font size %i is not supported!", size );
        return bitmap;
    }

    /* Calculates the required dimensions */
    while (*pt != '\0')
    {
        ichar++;
        if (*pt == '\n'){
            pen_y += (int)(size * line_space);
            if (pen_x < x){
                pen_x = x;
            }
            x = 0;
            pt++;
        }
        else if (*pt == '\t'){
            FT_UInt glyph_index = FT_Get_Char_Index( face, ' ' );
            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
            int space_size = face->glyph->advance.x >> 6;
            int tab_advance = x % (4 * space_size);
            if (tab_advance == 0) tab_advance = 4 * space_size;
            x += tab_advance;
            pt++;
        }
        else if ((unsigned char)(*pt) < ' '){
            /* Ignore this character */
            pt++;
        }
        else{
            unsigned long ucode = 0;
            pt = char2unicode( pt, &ucode );
            FT_UInt glyph_index = FT_Get_Char_Index( face, ucode );
            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
            x += (face->glyph->advance.x >> 6);
        }
    }

    if (pen_x < x){
        pen_x = x;
    }

    if (pen_x == 0){
        return bitmap;
    }

    /* Reserve memory */
    bitmap.buffer = (uint8_t*)calloc( pen_x * pen_y, sizeof( uint8_t ) );
    bitmap.width = pen_x;
    bitmap.height = pen_y;

    /* Draw the bitmap */
    pen_x = 0;
    pen_y = 0;
    pt = text;
    while (*pt != '\0')
    {
        ichar++;
        if (*pt == '\n'){
            pen_y += (int)(size * line_space);
            pen_x = 0;
            pt++;
        }
        else if (*pt == '\t'){
            FT_UInt glyph_index = FT_Get_Char_Index( face, ' ' );
            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
            int space_size = face->glyph->advance.x >> 6;
            int tab_advance = pen_x % (4 * space_size);
            if (tab_advance == 0) tab_advance = 4 * space_size;
            pen_x += tab_advance;
            pt++;
        }
        else if ((unsigned char)(*pt) < ' '){
            /* Ignore this character */
            pt++;
        }
        else{
            unsigned long ucode = 0;
            pt = char2unicode( pt, &ucode );
            FT_UInt glyph_index = FT_Get_Char_Index( face, ucode );
            int offset = pen_y * bitmap.width + pen_x;

            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );

            /* Align the vertical row */
            offset += (size - face->glyph->bitmap_top - 1) * bitmap.width;
            write_glyph( &(bitmap.buffer[offset]), face, bitmap.width );

            pen_x += (face->glyph->advance.x >> 6);
        }
    }

    return bitmap;
}

void BitmapTag_dispose( BitmapTag* bitmap )
{
    free( bitmap->buffer );
    bitmap->buffer = 0;
    bitmap->width = 0;
    bitmap->height = 0;
}
