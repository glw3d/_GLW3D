/* 
 * Author: Mario J. Martin <dominonurbs$gmail.com>
 *
 * Methods to load and save images in BMP/DIB format.
 */

#ifndef _MG_BMP__H
#define _MG_BMP__H

#include <stdint.h>

#pragma pack(push)  /* Push the current packing */
#pragma pack(2)     /* Sets to 2 byte packing. This is to ensure the correct alignment of the header */


/* Header info for a 52 bits DIB/BMP file format (Windows V3) */
typedef struct 
{ 
    /* Magic number. Posible values are BM, BA, CI, CP, IC and PT. */
	uint8_t type[2];			

    /* Size of the file in bytes. */
	uint32_t fileSize;	

	uint32_t reserved;	

    /* Offset byte from the beginning where the bitmap data can be found.  */
	uint32_t dataOffset;	

    /* the size of this header (from here) */
	uint32_t headerSize;		

    /* width and height in pixels. A negative height indicate a top left rasterization. */
	int width, height;

    /** always 1 */
	uint16_t planes;

    /* Number of bits of the data. Typical values are 1, 4, 8, 24 and 32
     * Warning! This is usually taken from the header of the original file, 
     * but the loader always converts the image into 32 bits,
     * because the standard in MG is BGRA */
	uint16_t bitsPerPixel;

    /* Compression method being used. Posible values are 0 (no compresion), 1, 2, 3, 4 nd, 5. */
	uint32_t compression;	

    /* The size of the raw bitmap data. */
	uint32_t dataSize;	

    /* Pixel per meter (used only for the printer). */
	uint32_t horizontalResolution;

    /* Pixel per meter (used only for the printer). */
	uint32_t verticalResolution;

    /* Number of colors of the palette. A value of 0 is for BGRA modes (not using palette) */
	uint32_t numberOfColors;

    /* Number of important colors of the palette. */
	uint32_t numberOfImportantColors; 

}gwBmpHeaderInfo; 

#pragma pack(pop) /* Resets to default packing */ 

/* BMP data structure */
typedef struct 
{
	gwBmpHeaderInfo header; /* Header information */
	uint8_t *data;          /* Raw data */
	size_t data_length;     /* Total length of the data */
    int bpp;                /* Bytes per pixel */
} gwBmpImage;


#ifdef	__cplusplus
extern "C" {
#endif

/* Initializes bmp image data structure */
void gw_bmp_init( gwBmpImage* image );

/* Releases memory for a bmp image data structure */
void gw_bmp_free( gwBmpImage* image );

/* Shows the header of a bmp image */
void gw_bmp_showHeader
    ( const gwBmpImage* image
    , void (*f)(const char*, ...)
    );

/* Loads a bmp file. Set show header true if you want to display the header */
gwBmpImage gw_bmp_import
    ( const char* filename
    , const int alphaChannel
    );

/* Saves a image in a .bmp file */
int gw_bmp_save
    ( const char* filename
    , uint8_t* data
    , const unsigned int width
    , const unsigned int height
    , const int bpp             /* bits per pixel of the input image */
    , const int alpha_channel   /*1-RGBA (the output includes the alpha channel) 0-Only RGB */
    );

#ifdef	__cplusplus
}
#endif

#endif 

