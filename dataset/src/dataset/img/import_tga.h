/* 
 * Author: Mario J. Martin <dominonurbs$gmail.com>
 *
 * Class to load and save images in TGA format.
 */

#ifndef _MG_TGA_LOADER_H
#define _MG_TGA_LOADER_H

#include <stdint.h>


#ifdef	__cplusplus
extern "C" {
#endif

#pragma pack(push) /* Store the current packing */ 
#pragma pack(1) /* Sets to 1 byte packing */ 
/* Header info for a 52 bits DIB/BMP file format (Windows V3) */
typedef struct 
{ 
    /* Number of characters in Identification field. */
	uint8_t idlength;			

    /* 0 or 1. 0 indicates if no color map is included. */
	uint8_t colourmaptype;      

    /* Number that indicates the format of the image. */
	uint8_t datatypecode;		

    /* Index of the first color map entry (ignored if colourmaptype = 1) */
	uint16_t colourmaporigin;	

    /* Color map length (ignored if colourmaptype = 1) */
	uint16_t colourmaplength;	

    /* Number of bits of each color entry (16, 24, 32) */
	uint8_t colourmapdepth;	    

    /* X coordinate of the lower left corner of the image */
	uint16_t x_origin;	       

    /* Y coordinate of the lower left corner of the image */
	uint16_t y_origin;	        

    /* Width in pixels of the image */
	uint16_t width;			    

    /* Height in pixels of the image */
	uint16_t height;			

    /* Number of bits per pixel (16, 24, 32) */
	uint8_t bitsperpixel;		

    /* Some flags */
	uint8_t imgedescriptor;	    

}MGtgaHeaderInfo; 

#pragma pack(pop) /* Resets to default packing */ 

typedef struct
{
	MGtgaHeaderInfo header;
	char id_string[256];
	uint8_t *data;
	unsigned int data_length;
	uint8_t *mapcolor;
    int bpp;
} gwTgaImage;

/* Initializes the tga data structure */
void gw_tga_init( gwTgaImage* image );

/* Releases memory resources */
void gw_tga_free( gwTgaImage* image );

/* Shows the header of a tga file */
void gw_tga_showHeader( const gwTgaImage* image, void (*f)(const char*, ...) );

/* Imports a .tga file */
gwTgaImage gw_tga_import( const char* filename, const int alphaChannel );

#ifdef	__cplusplus
}
#endif

#endif 

