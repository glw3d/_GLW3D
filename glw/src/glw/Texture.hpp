
#ifndef _HGW_TEXTURE_H
#define _HGW_TEXTURE_H

#include <stdlib.h>

#include "common/definitions.h"
#include "common/log.h"
#include "common/check_malloc.h"

#include "defines.h"
#include "gwgl.h"
#include "signatures.h"
#include "gwdata.h"

namespace gw{

    class Texture
    {
        /* DATA */
    public:
        /* Raw data pointer should be BGRA (32 bits) image format */
        gwTexture image;

        /* Signature */
        int signature;

        /* Id assigned by the manager */
        int id;

        /* Id assigned by OpenGL */
        GLuint tex_name;

        /* Bytes per pixel */
        uint bpp;

        /* Most probably GL_BGRA. Other possible values are GL_RED, GL_RG, GL_RGB, and GL_RGBA.
        * For most GPUs GL_BGRA is prefered. Although OpenGL accepts RGB format, this is usually
        * converted by the driver into RGBA (32 bits). In other words, usually there is no real
        * benefit on using 24 bits formats (RGB or BGR). Also most GPUs are optimized with BGRA
        * instead of RGBA, although this last one is an old legacy */
        GLenum format;

        /* For performance, most probably some compressed format, (GL_COMPRESSED_RGBA).
        * Non compressed formats are preferable in some cases for quality. Compresion algorithms
        * are designed to be fast at expense of loosing image quality.
        * Some applications may use other formats, like GL_DEPTH_COMPONENT.
        * The sRGB format is an alternative to RGB that is gaining popularity, because it reproduces
        * better the human eye sensitivity to colors http://www.w3.org/Graphics/Color/sRGB.html ,
        * but it may not be supported by all graphic cards.
        * For other formats see http://www.opengl.org/sdk/docs/man4/
        * The internalFormat parameter is only used when glTexImage2D(), and you can change it
        * manually before generating the texture, but it must be coherent with 'format'
        * in the number of color channels. */
        GLint internalFormat;

        /* Usually is GL_TEXTURE_2D, if one of the dimensions is 1 then GL_TEXTURE_1D may be used.
        * OpenGL also supports many other formats, like 3D textures and cube textures,
        * but notice that this class is meant to work with 2D images. */
        GLenum target;

        /* Specifies the data type of the pixel. GL_UNSIGNED_BYTE is always employed */
        GLenum type;

        /* Specifies the level-of-detail number (LOD).
        * Level 0 is the base image an the only value that should be used. Since OpenGL 1.4 mipmaps
        * are always automatically generated; although requires power of 2 dimensions.
        * There are exceptional situations where the lod could be different of 0 */
        GLint level;

        /* Specifies if the texture is generated with mipmaps */
        bool _mipmap;

        /* PROPERTIES */
        /* Indicates if the texture is created with mipmapping */
        inline const bool mipmap() const { return this->_mipmap; }

        /* CONSTRUCTOR AND INITIALIZERS */
    public:
        /* Default constructor */
        Texture()
        {
            init();
        }

        /* Default initializer (using constructor notation) */
        Texture& operator()()
        {
            init();

            return *this;
        }

        /* Default initializer */
        void init()
        {
            // default values
            image.buffer = nullptr;
            image.width = 0;
            image.height = 0;
            image.update = 0;
            image.interpolation = 1;

            signature = _GW_SIGNATURE_TEXTURE;
            tex_name = 0;
            id = 0;
            bpp = 4;
            format = GL_BGRA;
            internalFormat = GL_COMPRESSED_RGBA;
            target = GL_TEXTURE_2D;
            type = GL_UNSIGNED_BYTE;
            level = 0;
            _mipmap = false;
        }

        /* Constructor */
        Texture& alloc( const GLsizei width, const GLsizei height )
        {
            _check_( image.buffer = (gwColor*)_malloc_
                ( sizeof( gwColor ) * width * height * 4 ) );
            image.width = width;
            image.height = height;

            return *this;
        }

        /* Copy constructor */
        Texture( const Texture& y )
        {
            this->image = y.image;
            this->tex_name = y.tex_name;
            this->bpp = y.bpp;
            this->format = y.format;
            this->internalFormat = y.internalFormat;
            this->target = y.target;
            this->type = y.type;
            this->level = y.level;
            this->_mipmap = y._mipmap;
        }

        /* Destructor */
        ~Texture()
        {
            signature = 0;

            if (tex_name > 0){
                _handle_error_( "Warning! texture buffer %i is not deleted before destructor", tex_name );
                tex_name = 0;
            }
            freeMemory();
        }

        /* Releases the stream raw data in the CPU */
        void freeMemory()
        {
            if (image.buffer != nullptr){
                free( image.buffer );
                image.buffer = nullptr;
                image.width = 0;
                image.height = 0;
            }
        }

        /* Deletes the texture buffer in the GPU */
        void deleteTexture()
        {
            if (tex_name > 0){
                glDeleteTextures( 1, &tex_name );
                tex_name = 0;
            }
        }

        /* Releses all resources */
        void dispose()
        {
            freeMemory();
            deleteTexture();
        }

        /* Sets the canvas dimensions. The effect would be to cut a portion of the image if
        * the dimension is lower that the original, or resizes the canvas filling the extra with 0.
        * Negative values are allowed to strech the canvas to the down-left corner. */
        void canvas( const int x0, const int y0, const int new_width, const int new_height )
        {
            int x, y, orig_index, dest_index;
            gwColor* dest = nullptr;

            _check_( dest = (gwColor*)_malloc_
                ( sizeof( gwColor ) * new_width * new_height * bpp ) );

            dest_index = 0;

            for (y = y0; y < y0 + new_height; y++){
                for (x = x0; x < x0 + new_width; x++){
                    orig_index = (x + y * image.width) * bpp;
                    for (uint c = 0; c < bpp; c++){
                        if (x < 0 || y < 0 || x >= image.width || y >= image.height){
                            dest[dest_index].b = 0;
                            dest[dest_index].g = 0;
                            dest[dest_index].r = 0;
                            dest[dest_index].a = 0;
                            dest_index++;
                        }
                        else{
                            dest[dest_index] = image.buffer[orig_index];
                            dest_index++;
                            orig_index++;
                        }
                    }
                }
            }

            free( image.buffer );
            image.buffer = dest;
            image.width = new_width;
            image.height = new_height;
        }

        /* Sets the transparency to 0 in the w channel by choosing one color as a mask.
        * Tolerance measures the distance of each pixel channel to the mask. A tolerance of 0 means
        * that only the pixels that matches the mask are affected */
        void transparencyMask( const gwColor transparent_color, const GLubyte tolerance = 0 )
        {
            for (int i = 0; i < image.width * image.height; i += 1){
                gwColor raw = image.buffer[i];
                GLubyte db = raw.b - transparent_color.b;
                GLubyte dg = raw.g - transparent_color.g;
                GLubyte dr = raw.r - transparent_color.r;
                db = (db >= 0) ? db : -db;
                dg = (dg >= 0) ? dg : -dg;
                dr = (dr >= 0) ? dr : -dr;
                if (db <= tolerance && dg <= tolerance && dr <= tolerance){
                    image.buffer[i].a = 0;
                }
            }
        }

        /* Updates the data texture */
        void updateTexture()
        {

            if (tex_name == 0){
                generateTexture( _mipmap, nullptr );
                return;
            }

            if (image.buffer == nullptr){
                return;
            }

            if (target == GL_TEXTURE_2D){

                glBindTexture( target, tex_name );
                if (bpp == 4){
                    glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
                }
                else{
                    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
                }

                /* Send the data to the GPU */
                /* A common mistake is to call glTexImage2D() instead glTexSubImage2D. */
                glTexSubImage2D( target, 0, 0, 0, image.width, image.height, format, type, image.buffer );

                /* deprecated... if (glewIsSupported("GL_ARB_framebuffer_object") == GL_TRUE && _mipmap == true){ */
                if (_mipmap == true){
                    /* Since opengl 3.0 mipmaps are generated with glGenerateMipmap(), but it seems that
                    * it is not always working properly with all graphic cards */
                    glEnable( target );      // Without this, some ATI cards has reported problems
                    glGenerateMipmap( target );
                }

                /* Unbind the texture */
                glBindTexture( target, 0 );
            }
            else if (target == GL_TEXTURE_1D){
                glBindTexture( target, tex_name );
                /* bpp == 4 */
                glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
                glTexSubImage1D( target, level, 0, image.width * image.height, format, type, image.buffer );

                /* Unbind the texture */
                glBindTexture( target, 0 );
            }

            GLenum er = glGetError();

            if (er == GL_OUT_OF_MEMORY){
                _handle_error_( "OpenGL - Out of memory!" );
            }
            else if (er != GL_NO_ERROR){
                _handle_error_( "OpenGL - error = %i", er );
            }
        }

        /* Activates the texture unit and binds the texture to be used as sampler */
        void bindTexture
            ( const int texture_location
            , const int tex_unit
            ) const
        {
            if (tex_name > 0){
                glActiveTexture( GL_TEXTURE0 + tex_unit );
                glUniform1i( texture_location, tex_unit );
                glBindTexture( target, tex_name );
            }
        }

        /* Unbinds the texture */
        void UnBindTexture() const
        {
            glBindTexture( target, 0 );
        }

        /* Calculates the lower closest power of 2. */
        int getPowerof2( const int size )
        {
            int lower_size = 1;
            int x = size / 2;

            if (size <= 0)
                return 0;

            while (x > 0){
                lower_size *= 2;
                x /= 2;
            }

            return lower_size;
        }

        /* Generates a texture. Only the color texture is generated. */
        void generateTexture( bool mipmap, void( *funcTexParam )(Texture&) )
        {
            if (image.buffer == nullptr || image.width <= 0 || image.height <= 0){
                return;
            }
            _mipmap = mipmap;

            /* Generates a new texture name */
            if (tex_name > 0){
                glDeleteTextures( 1, &tex_name );
            }
            glGenTextures( 1, &tex_name );

            /* check if one of the dimensions is 1 */
            if (image.width == 1 || image.height == 1){
                target = GL_TEXTURE_1D;
            }
            else{
                target = GL_TEXTURE_2D;
            }

            if (target == GL_TEXTURE_2D){
                if (_mipmap == true){
                    /* Check if the texture dimensions are power of 2 */
                    GLsizei width_p2 = getPowerof2( image.width );
                    GLsizei height_p2 = getPowerof2( image.height );

                    if (width_p2 != image.width || height_p2 != image.height){
                        /* Warning! Image sizes are not power of 2.
                        * Mipmapping is disabled"); */
                        _mipmap = false;
                    }
                }

                glBindTexture( target, tex_name );

                /* bpp == 4 */
                glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

                /* deprecated... if (glewIsSupported("GL_ARB_framebuffer_object") == GL_FALSE &&_mipmap == true){ */
                if (_mipmap == true){
                    /* Since OpenGL 1.4 mipmaps are automatically generated,
                    * so GL_GENERATE_MIPMAP is deprecated and since 3.1 is removed. */
                    //glTexParameteri( target, GL_GENERATE_MIPMAP, GL_TRUE );
                }

                if (funcTexParam == nullptr){
                    // some default parameters
                    glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_REPEAT );
                    glTexParameteri( target, GL_TEXTURE_WRAP_T, GL_REPEAT );
                    if (image.interpolation != 0){
                        glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                        if (_mipmap == true){
                            glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
                        }
                        else{
                            glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                        }
                    }
                    else{
                        glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                        if (_mipmap == true){
                            glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
                        }
                        else{
                            glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                        }
                    }
                }
                else{
                    funcTexParam( *this );
                }

                /* Send the data to the GPU */
                glTexImage2D( target, level, internalFormat
                    , image.width, image.height, 0, format, type, image.buffer );

                /* deprecated... if (glewIsSupported("GL_ARB_framebuffer_object") == GL_TRUE && _mipmap == true){ */
                if (_mipmap == true){
                    /* Since opengl 3.0 mipmaps should be generated with glGenerateMipmap() */
                    //glEnable(target);_check_gl_error_    // Without this, some ATI cards has reported problems
                    glGenerateMipmap( target );
                }

                /* Unbind to make sure that there will not be accidental modifications */
                glBindTexture( target, 0 );
            }
            else if (target == GL_TEXTURE_1D){

                glBindTexture( target, tex_name );

                /* bpp == 4 */
                glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

                if (funcTexParam == nullptr){
                    /* Set some default texture properties values */
                    glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_REPEAT );
                    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                }
                else{
                    funcTexParam( *this );
                }
                /* Mipmapping usually is not desired with 1D textures */

                glTexImage1D( target, level, internalFormat
                    , image.width * image.height, 0, format, type, image.buffer );

                /* Unbind to make sure that there will not be accidental modifications */
                glBindTexture( target, 0 );
            }

            GLenum er = glGetError();
            if (er == GL_OUT_OF_MEMORY){
                _handle_error_( "OpenGL - Out of memory!" );
            }
            else if (er != GL_NO_ERROR){
                _handle_error_( "OpenGL - error = %i", er );
            }
        }

        /* Generates a texture */
        void generateTexture()
        {
            generateTexture( true, nullptr );
        }

        /* Generates a texture with some parameters optimized for blitting. */
        void generateSprite( bool mipmap = false )
        {
            /* Set a non-compressed format */
            if (internalFormat == GL_COMPRESSED_RGB){
                internalFormat = GL_RGB8;
            }
            else if (internalFormat == GL_COMPRESSED_RGBA){
                internalFormat = GL_RGBA8;
            }
            else if (internalFormat == GL_COMPRESSED_SRGB){
                internalFormat = GL_SRGB8;
            }
            else if (internalFormat == GL_COMPRESSED_SRGB_ALPHA){
                internalFormat = GL_SRGB8_ALPHA8;
            }

            generateTexture( mipmap, &sprite_parameters );
        }

    private:
        /* Parameters used for sprites */
        static void sprite_parameters( Texture& img )
        {
            if (img.mipmap() == true){
                glTexParameteri( img.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
                glTexParameteri( img.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
                glTexParameteri( img.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri( img.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
            }
            else{
                glTexParameteri( img.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
                glTexParameteri( img.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
                glTexParameteri( img.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexParameteri( img.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            }
        }
    };

} /* end namespace */

#endif /* _HGW_TEXTURE_H */
