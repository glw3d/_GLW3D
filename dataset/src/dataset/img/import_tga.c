/*
 * Author: Mario J. Martin <dominonurbs$gmail.com>
 *
 * Class to load and save images in TGA format.
 */

#include <stdio.h>
#include <stdlib.h>

#include "common/definitions.h"
#include "common/check_malloc.h"
#include "common/log.h"

#include "import_tga.h"

typedef struct
{
	uint8_t b, g, r, a;
}BGRA;

typedef struct
{
	uint8_t b, g, r;
}BGR;

/* Initializes the tga data structure */
void gw_tga_init( gwTgaImage* image )
{
	image->data = nullptr;
	image->data_length = 0;
    image->mapcolor = nullptr;
    image->id_string[0] = '\0';
    image->bpp = 0;
}

/* Releases memory resources */
void gw_tga_free( gwTgaImage* image )
{
    if (image->data != nullptr){
        free( image->data );
    }
    if (image->mapcolor != nullptr){
        free( image->mapcolor );
    }

    image->data = nullptr;
    image->mapcolor = nullptr;
	image->data_length = 0;
	image->id_string[0] = '\0';
    image->bpp = 0;
}


static void readrgb24( gwTgaImage* image, const void* raw )
{
	BGR *p_data;
	uint8_t *p_raw;
    int x, y;
    uint8_t b, g, r;
    size_t pixel_length = (size_t)image->header.height * (size_t)image->header.width;

	if (image->data == nullptr || image->data_length < pixel_length * sizeof(BGR)){
		image->data = (uint8_t*)malloc(pixel_length * sizeof(BGR));
	}
    image->data_length = pixel_length * sizeof(BGR);

    p_data = (BGR*)image->data;
	p_raw = (uint8_t*)raw;
	for (y = 0; y < image->header.height; y++){
		for (x = 0; x < image->header.width; x++){
			b = *p_raw++;
			g = *p_raw++;
			r = *p_raw++;

			p_data->b = b;
			p_data->g = g;
			p_data->r = r;
			p_data++;
		}
	}
    image->bpp = 3;
}

static void readrgb24w( gwTgaImage* image, const void* raw )
{
	BGRA *p_data;
	uint8_t *p_raw;
    int x, y;
    uint8_t b, g, r;
    size_t pixel_length = (size_t)image->header.height * (size_t)image->header.width;

    if (image->data == nullptr || image->data_length < pixel_length * sizeof(BGRA)){
		image->data = (uint8_t*)malloc(pixel_length * sizeof(BGRA));
	}

    image->data_length = pixel_length * sizeof(BGRA);

    p_data = (BGRA*)image->data;
	p_raw = (uint8_t*)raw;
	for (y = 0; y < image->header.height; y++){
		for (x = 0; x < image->header.width; x++){
			b = *p_raw++;
			g = *p_raw++;
			r = *p_raw++;

			p_data->b = b;
			p_data->g = g;
			p_data->r = r;
			p_data->a = 0xFF;
			p_data++;
		}
	}
    image->bpp = 4;
}

static void readrgb32( gwTgaImage* image, const void* raw )
{
	BGRA *p_data;
	uint8_t *p_raw;
    int x, y;
    uint8_t b, g, r, a;
    size_t pixel_length = (size_t)image->header.height * (size_t)image->header.width;

    if (image->data == nullptr || image->data_length < pixel_length * sizeof(BGRA)){
		image->data = (uint8_t*)malloc(pixel_length * sizeof(BGRA));
	}

	image->data_length = pixel_length * sizeof(BGRA);

    p_data = (BGRA*)image->data;
	p_raw = (uint8_t*)raw;
	for (y = 0; y < image->header.height; y++){
		for (x = 0; x < image->header.width; x++){
			b = *p_raw++;
			g = *p_raw++;
			r = *p_raw++;
			a = *p_raw++;

			p_data->b = b;
			p_data->g = g;
			p_data->r = r;
			p_data->a = a;
			p_data++;
		}
	}
    image->bpp = 4;
}


static void readrgbrle24( gwTgaImage* image, FILE *file_handler, const char *filename )
{
	uint8_t header;
	BGR raw[128];
	BGR color;
	int number;
	BGR *p_data;
    int i, x, y;
    size_t pixel_length = (size_t)image->header.height * (size_t)image->header.width;

    if (image->data == nullptr || image->data_length < pixel_length * sizeof(BGR)){
		image->data = (uint8_t*)malloc(pixel_length * sizeof(BGR));
	}

    image->data_length = pixel_length * sizeof(BGR);

	p_data = (BGR*)image->data;
	for (y = 0; y < image->header.height; y++){
		for (x = 0; x < image->header.width;){
			if (fread(&header, sizeof(char), 1, file_handler) < 1){
				_handle_error_("Cannot read data! %s", filename);
				return;
			}
			number = (header & 0x7F);
			if ((header & 0x80) == 0x80){
				/* compressed RLE */
				if (fread(&color, sizeof(BGR), 1, file_handler) < 1){
					_handle_error_("Cannot read data! %s", filename);
					return;
				}
				for (i = 0; i < number + 1; i++){
					p_data->b = color.b;
					p_data->g = color.g;
					p_data->r = color.r;
					p_data++;
					x++;
				}
			}
			else{
				/* raw data */
				if (fread(raw, sizeof(BGR), number + 1, file_handler) < 1){
					_handle_error_("Cannot read data! %s", filename);
					return;
				}
				for (i = 0; i < number + 1; i++){
					p_data->b = raw[i].b;
					p_data->g = raw[i].g;
					p_data->r = raw[i].r;
					p_data++;
					x++;
				}
			}
		}
	}
    image->bpp = 3;
}

static void readrgbrle24w( gwTgaImage* image, FILE *file_handler, const char *filename )
{
	uint8_t header;
	BGR raw[128];
	BGR color;
	int number;
	BGRA *p_data;
    int i, x, y;
    size_t pixel_length = (size_t)image->header.height * (size_t)image->header.width;

    if (image->data == nullptr || image->data_length < pixel_length * sizeof(BGRA)){
		image->data = (uint8_t*)malloc(pixel_length * sizeof(BGRA));
	}

    image->data_length = pixel_length * sizeof(BGRA);

	p_data = (BGRA*)image->data;
	for (y = 0; y < image->header.height; y++){
		for (x = 0; x < image->header.width;){
			if (fread(&header, sizeof(char), 1, file_handler) < 1){
				_handle_error_("Cannot read data! %s", filename);
				return;
			}
			number = (header & 0x7F);
			if ((header & 0x80) == 0x80){
				/* compressed RLE */
				if (fread(&color, sizeof(BGR), 1, file_handler) < 1){
					_handle_error_("Cannot read data! %s", filename);
					return;
				}
				for (i = 0; i < number + 1; i++){
					p_data->b = color.b;
					p_data->g = color.g;
					p_data->r = color.r;
					p_data->a = 0xFF;
					p_data++;
					x++;
				}
			}
			else{
				/* raw data */
				if (fread(raw, sizeof(BGR), number + 1, file_handler) < 1){
					_handle_error_("Cannot read data! %s", filename);
					return;
				}
				for (i = 0; i < number + 1; i++){
					p_data->b = raw[i].b;
					p_data->g = raw[i].g;
					p_data->r = raw[i].r;
					p_data->a = 0xFF;
					p_data++;
					x++;
				}
			}
		}
	}
    image->bpp = 4;
}

static void readrgbrle32( gwTgaImage* image, FILE *file_handler, const char *filename )
{
	uint8_t header;
	BGRA raw[128];
	BGRA color;
	int number;
	BGRA *p_data;
    int i, x, y;
    size_t pixel_length = (size_t)image->header.height * (size_t)image->header.width;

	if (image->data == nullptr || image->data_length < pixel_length * sizeof(BGRA) ){
		image->data = (uint8_t*)malloc(pixel_length * sizeof(BGRA));
	}

    image->data_length = pixel_length * sizeof(BGRA);

	p_data = (BGRA*)image->data;
	for (y = 0; y < image->header.height; y++){
		for (x = 0; x < image->header.width;){
			if (fread(&header, sizeof(char), 1, file_handler) < 1){
				_handle_error_("Cannot read data! %s", filename);
				return;
			}
			number = (header & 0x7F);
			if ((header & 0x80) == 0x80){
				/* compressed RLE */
				if (fread(&color, sizeof(BGRA), 1, file_handler) < 1){
					_handle_error_("Cannot read data! %s", filename);
					return;
				}
				for (i = 0; i < number + 1; i++){
					p_data->b = color.b;
					p_data->g = color.g;
					p_data->r = color.r;
					p_data->a = color.a;
					p_data++;
					x++;
				}
			}
			else{
				/* raw data */
				if (fread(raw, sizeof(BGRA), number + 1, file_handler) < 1){
					_handle_error_("Cannot read data! %s", filename);
					return;
				}
				for (i = 0; i < number + 1; i++){
					p_data->b = raw[i].b;
					p_data->g = raw[i].g;
					p_data->r = raw[i].r;
					p_data->a = raw[i].a;
					p_data++;
					x++;
				}
			}
		}
	}
    image->bpp = 4;
}

/* Shows the header of a tga file */
void gw_tga_showHeader( const gwTgaImage* image, void (*f)(const char*, ...) )
{
    if (image == nullptr){
        return;
    }

	f("\n idlength: %i", image->header.idlength);
	f("\n colourmaptype: %i", image->header.colourmaptype);
	f("\n datatypecode: %i", image->header.datatypecode);
	f("\n colourmaporigin: %i", image->header.colourmaporigin);
	f("\n colourmaplength: %i", image->header.colourmaplength);
	f("\n colourmapdepth: %i", image->header.colourmapdepth);
	f("\n x_origin: %i", image->header.x_origin);
	f("\n y_origin: %i", image->header.y_origin);
	f("\n width: %i", image->header.width);
	f("\n height: %i", image->header.height);
	f("\n bitsperpixel : %i", image->header.bitsperpixel);
	f("\n imgedescriptor: %X", image->header.imgedescriptor);
}

/* Loads a tga file */
static void tga_import( gwTgaImage* image, const char* filename, const int alphaChannel )
{
	FILE *file_handler;

    if (image == nullptr){
		return;
    }

	file_handler = fopen(filename, "rb+");

	if(file_handler == NULL){
		_handle_error_("Cannot open filename %s", filename);
		return;
	}

	/* read the header */
	if (fread(&image->header, sizeof(MGtgaHeaderInfo), 1, file_handler) < 1){
		_handle_error_("Cannot read header filename %s", filename);
		fclose(file_handler);
		return;
	}

	/* read id string */
	if (image->header.idlength > 0){
		if (fread(&image->id_string, sizeof(char), image->header.idlength, file_handler) < 1){
			_handle_error_("Cannot read id string! %s", filename);
			fclose(file_handler);
			return;
		}
	}

	/* read map color */
	if (image->header.colourmaptype == 1){
		_handle_error_("Color mapped images are not supported! %s", filename);
		fclose(file_handler);
		return;
	}

	/* check the data */
	if  (  image->header.datatypecode == 0 || image->header.bitsperpixel == 0 
        || image->header.width == 0 || image->header.height == 0 ){
		/* there is no data */
		_handle_error_("Image data is empty! %s", filename);
		fclose(file_handler);
		return;
	}

    /* make sure that width and length are positives */
    if (image->header.width < 0){
        image->header.width = -image->header.width;
    }
    if (image->header.height < 0){
        image->header.height = -image->header.height;
    }

	/* read the data */
	if (image->header.datatypecode == 1){
		/* uncompressed color mapped images */
		_handle_error_("Uncompressed TGA format is not supported! %s", filename);
	}
	else if (image->header.datatypecode == 2){
		/* uncompressed BGR images */
		if (image->header.bitsperpixel == 24){
            size_t raw_data_size = (image->header.bitsperpixel / 8) 
                                 * image->header.width * image->header.height;

		    void* raw_data = malloc(raw_data_size);
		    if (fread(raw_data, raw_data_size, 1, file_handler) < 1){
			    _handle_error_("Cannot read image data from %s", filename);
		    }
            else{
                if (alphaChannel == 1){
			        readrgb24w(image, raw_data);
                }
                else{
			        readrgb24(image, raw_data);
                }
            }
		    free(raw_data);
		}
		else if (image->header.bitsperpixel == 32){
            size_t raw_data_size = (image->header.bitsperpixel / 8) 
                                 * image->header.width * image->header.height;

		    void* raw_data = malloc(raw_data_size);
		    if (fread(raw_data, raw_data_size, 1, file_handler) < 1){
			    _handle_error_("Cannot read image data from %s", filename);
		    }
            else{
		        readrgb32(image, raw_data);
            }
		    free(raw_data);
		}
		else{
			_handle_error_("Format image not supported! %s", filename);
		}
	}
	else if (image->header.datatypecode == 3){
		/* uncompressed black and white images */
		_handle_error_("Format image not supported! %s", filename);
	}
	else if (image->header.datatypecode == 9){
		/* runlength encoded (RLE) color mapped images */
		_handle_error_("Format image not supported! %s", filename);
	}
	else if (image->header.datatypecode == 10){
		/* runlength (RLE) encoded BGR images */
		if (image->header.bitsperpixel == 24){
            if (alphaChannel == 1){
			    readrgbrle24w(image, file_handler, filename);
            }
            else{
			    readrgbrle24(image, file_handler, filename);
            }
		}
		else if (image->header.bitsperpixel == 32){
			readrgbrle32(image, file_handler, filename);
		}
		else{
			_handle_error_("Format image not supported! %s", filename);
		}
	}
	else if (image->header.datatypecode == 11){
		/* compressed black and white images */
		_handle_error_("Format image not supported! %s", filename);
	}
	else{
		/* compressed, using Huffman, Delta or whatever */
		_handle_error_("Format image not supported! %s", filename);
	}

	fclose(file_handler);
}

/* Loads a tga file */
gwTgaImage gw_tga_import( const char* filename, const int alphaChannel )
{
    gwTgaImage image;

    gw_tga_init( &image );
    tga_import( &image, filename, alphaChannel );

    return image;
}
