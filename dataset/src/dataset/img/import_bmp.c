/*
 * Author: Mario J. Martin <dominonurbs$gmail.com>
 *
 * Load and save images in BMP/DIB format.
 */

#include <stdio.h>
#include <stdlib.h>

#include "common/definitions.h"
#include "common/check_malloc.h"
#include "common/log.h"

#include "import_bmp.h"

/** Method used to load BMP/DIB file formats */
typedef struct
{
	uint8_t b, g, r, a;   /* For bitmap images that are stored in BGRA format */
}BGRA;

typedef struct
{
	uint8_t b, g, r;      /* For bitmap images that are stored in BGR format */
}BGR;

void gw_bmp_init( gwBmpImage* image )
{
    image->data = nullptr;
    image->data_length = 0;
    image->bpp = 0;
}

void gw_bmp_free( gwBmpImage* image )
{
    if (image->data != nullptr){
	    free(image->data);
    }
    image->data = nullptr;
    image->data_length = 0;
    image->bpp = 0;
}

static void readbit1( gwBmpImage* image, const void* raw )
{
	_handle_error_("Loading 1 bit .bmp images is not supported!");
}

static void readbit1w( gwBmpImage* image, const void* raw )
{
	_handle_error_("Loading 1 bit .bmp images is not supported!");
}

static void readbit4( gwBmpImage* image, const void* raw )
{
	_handle_error_("Loading 4 bit .bmp images is not supported!");
}

static void readbit4w( gwBmpImage* image, const void* raw )
{
	_handle_error_("Loading 4 bit .bmp images is not supported!");
}

/* Reads 256 colors. The output is RGB */
static void readbit8( gwBmpImage* image, const uint8_t *palette, const void* raw )
{
    int i, x, y;
    unsigned int index;
    uint8_t b, g, r;
    int padding;
	BGR* data = (BGR*)image->data;
	unsigned int ipixel = 0;
	uint8_t *p_raw = (uint8_t*)raw;

	for (y = 0; y < image->header.height; y++){
		padding = 0;
		for (x = 0; x < image->header.width; x++){
			index = *p_raw++;
			b = palette[index*4+2];
			g = palette[index*4+1];
			r = palette[index*4];

			data[ipixel].b = b;
			data[ipixel].g = g;
			data[ipixel].r = r;
			ipixel++;
			padding++;
		}
		/* correct data alignment */
		for (i = 0; i < 4 - padding % 4 && padding % 4 != 0 ; i++)
			p_raw++;
	}
    image->bpp = 3;
}

/* Reads 256 colors. The output is RGBA */
static void readbit8w( gwBmpImage* image, const uint8_t *palette, const void* raw )
{
    int i, x, y;
    unsigned int index;
    uint8_t b, g, r;
    int padding;
	BGRA* data = (BGRA*)image->data;
	unsigned int ipixel = 0;
	uint8_t *p_raw = (uint8_t*)raw;

	for (y = 0; y < image->header.height; y++){
		padding = 0;
		for (x = 0; x < image->header.width; x++){
			index = *p_raw++;
			b = palette[index*4+2];
			g = palette[index*4+1];
			r = palette[index*4];

			data[ipixel].b = b;
			data[ipixel].g = g;
			data[ipixel].r = r;
			data[ipixel].a = 0xFF;
			ipixel++;
			padding++;
		}
		/* correct data alignment */
		for (i = 0; i < 4 - padding % 4 && padding % 4 != 0 ; i++)
			p_raw++;
	}
    image->bpp = 4;
}

/* Reads 24 bit colors. The output is RGB */
static void readbit24( gwBmpImage* image, const void* raw )
{
    int i, x, y;
    uint8_t b, g, r;
    int padding;
	BGR* data = (BGR*)image->data;
	unsigned int ipixel = 0;
	uint8_t *p_raw = (uint8_t*)raw;

    if (image->header.height < 0){
	    /* if height is negative, the image is stored from top to botton */
        _handle_error_("Warning! Negative height!");
    }
    for (y = 0; y < image->header.height; y++){
	    padding = 0;
	    for (x = 0; x < image->header.width; x++){
		    b = *p_raw++;
		    g = *p_raw++;
		    r = *p_raw++;

		    data[ipixel].b = b;
		    data[ipixel].g = g;
		    data[ipixel].r = r;
		    ipixel++;
		    padding++;
	    }
	    /* correct data alignment */
	    for (i = 0; i < padding % 4; i++){
		    p_raw++;
	    }
    }
    image->bpp = 3;
}

/* Reads 24 bit colors. The output is RGBA */
static void readbit24w( gwBmpImage* image, const void* raw )
{
    int i, x, y;
    uint8_t b, g, r;
    int padding;
	BGRA* data = (BGRA*)image->data;
	unsigned int ipixel = 0;
	uint8_t *p_raw = (uint8_t*)raw;

    if (image->header.height < 0){
	    /* if height is negative, the image is stored from top to botton */
        _handle_error_("Warning! Negative height!");
    }
    for (y = 0; y < image->header.height; y++){
	    padding = 0;
	    for (x = 0; x < image->header.width; x++){
		    b = *p_raw++;
		    g = *p_raw++;
		    r = *p_raw++;

		    data[ipixel].b = b;
		    data[ipixel].g = g;
		    data[ipixel].r = r;
		    data[ipixel].a = 0xFF;

		    ipixel++;
		    padding++;
	    }
	    /* correct data alignment */
	    for (i = 0; i < padding % 4; i++){
		    p_raw++;
	    }
    }
    image->bpp = 4;
}

/* Reads 32 bit colors. The output is RGBA */
static void readbit32( gwBmpImage* image, const void* raw )
{
    int x, y;
    uint8_t b, g, r, a;
    int height;
	BGRA* data = (BGRA*)image->data;
	unsigned int ipixel = 0;
	uint8_t *p_raw = (uint8_t*)raw;

    if (image->header.height > 0){
	    for (y = 0; y < image->header.height; y++){
		    for (x = 0; x < image->header.width; x++){
                /* Acording with Bergarne Stroutstrup it is unsafe to write
                b = *p_raw++;
                because the compiler does not know the right order for 
                the post ++ and the = operators
                */
			    b = *p_raw;
			    p_raw++;
			    g = *p_raw;
			    p_raw++;
			    r = *p_raw;
			    p_raw++;
			    a = *p_raw;
			    p_raw++;

			    data[ipixel].b = b;
			    data[ipixel].g = g;
			    data[ipixel].r = r;
			    data[ipixel].a = a;
			    ipixel++;
		    }
	    }
    }
    else{
	    /* if the height is negative, the image is stored from top to botton */
        height = -image->header.height;
	    for (y = 0; y < height; y++){
            p_raw = (uint8_t*)raw + height * image->header.width;
		    for (x = 0; x < image->header.width; x++){
			    b = *p_raw;
			    p_raw++;
			    g = *p_raw;
			    p_raw++;
			    r = *p_raw;
			    p_raw++;
			    a = *p_raw;
			    p_raw++;

			    data[ipixel].b = b;
			    data[ipixel].g = g;
			    data[ipixel].r = r;
			    data[ipixel].a = a;
			    ipixel++;
		    }
	    }
    }

    image->bpp = 4;
}


/* Shows the header of a bmp image */
void gw_bmp_showHeader( const gwBmpImage* image, void( *f )(const char*, ...) )
{
	f("\n type       : %c%c", image->header.type[0], image->header.type[1]);
	f("\n file size  : %i", image->header.fileSize);
	f("\n data offset: %i", image->header.dataOffset);
	f("\n header size: %i", image->header.headerSize);
	f("\n width      : %i", image->header.width);
	f("\n height     : %i", image->header.height);
	f("\n planes     : %i", image->header.planes);
	f("\n bits pixel : %i", image->header.bitsPerPixel);
	f("\n compression: %i", image->header.compression);
	f("\n image size : %i", image->header.dataSize);
	f("\n hresolution: %i", image->header.horizontalResolution);
	f("\n vresolution: %i", image->header.verticalResolution);
	f("\n num colors : %i", image->header.numberOfColors);
	f("\n importantcolors: %i", image->header.numberOfImportantColors);
}


/* Loads a bmp file */
static void bmp_load( gwBmpImage* image, const char* filename, const int alphaChannel )
{
	FILE *file_handler;
	int row;
	int padding;
	int dataSize;
	void* raw_data;
    uint8_t *palette = nullptr; // palette (it is only used in the 8 bits format)
    size_t pixel_length;

    /* mgImport_bmp_init(image); */

    if (filename == nullptr){
        return;
    }

	file_handler = fopen(filename, "rb");

	if (file_handler == NULL){
		_handle_error_("Cannot open filename %s", filename);
		return;
	}

	/* read the header */
	if (fread(&image->header, sizeof(gwBmpHeaderInfo), 1, file_handler) < 1){
		_handle_error_("Cannot read header filename %s", filename);
		fclose(file_handler);
		return;
	}

	/* check if is a DIB/BMP file */
	if (!(	(image->header.type[0] == 'B' && image->header.type[1] == 'M')
		||	(image->header.type[0] == 'B' && image->header.type[1] == 'A')
		||	(image->header.type[0] == 'C' && image->header.type[1] == 'I')
		||	(image->header.type[0] == 'C' && image->header.type[1] == 'P')
		||	(image->header.type[0] == 'I' && image->header.type[1] == 'C')
		||	(image->header.type[0] == 'P' && image->header.type[1] == 'T'))){
		_handle_error_("Not a bitmap file %s", filename);
		fclose(file_handler);
		return;
	}

	/* check if the image is compressed (we do not support compression). */
	if (image->header.compression != 0){
		_handle_error_("Compressed BMP files are no supported! :( ... %s", filename);
		fclose(file_handler);
		return;
	}

	/* if the image is indexed read the palette */
	if (image->header.numberOfColors > 0){
		fseek(file_handler, image->header.fileSize - image->header.dataSize
            - image->header.numberOfColors * sizeof(BGRA), 0);
		palette = (uint8_t*)malloc(sizeof(BGRA) * image->header.numberOfColors);
		if (fread(palette, sizeof(gwBmpHeaderInfo)
            , image->header.numberOfColors, file_handler) < 1){
			_handle_error_("Cannot read palette from %s", filename);
			fclose(file_handler);
			return;
		}
	}
	else if (image->header.numberOfColors == 0 && image->header.bitsPerPixel == 8){
		palette = (uint8_t*)malloc(sizeof(BGRA) * 256);
		fseek(file_handler, image->header.fileSize - image->header.dataSize - 1024, 0);
		if (fread(palette, sizeof(BGRA), 256, file_handler) < 1){
			_handle_error_("Cannot read palette from %s", filename);
			fclose(file_handler);
			return;
		}
	}

    /* make sure that width and height are positive */
    if (image->header.width < 0){
        image->header.width = -image->header.width;
    }
    if (image->header.height < 0){
        /* Actually in specs a negative height means that the image is inverted */
        image->header.height = -image->header.height;
    }
    pixel_length = (size_t)image->header.width * (size_t)image->header.height;

	/* read the data. We cannot trust the dataSize in the header (some files have a value of 0) */
	row = image->header.width;
	padding = 4 - ((image->header.width * image->header.bitsPerPixel) / 8) % 4;
	padding = (padding > 3) ? 0 : padding;
	dataSize = ((image->header.height * row * image->header.bitsPerPixel) / 8)
        + padding * image->header.height;
	raw_data = malloc(dataSize);
	fseek(file_handler, image->header.dataOffset, 0);
	if (fread(raw_data, dataSize, 1, file_handler) < 1){
		_handle_error_("Cannot read image data from %s", filename);
		free(raw_data);
		fclose(file_handler);
		return;
	}

	/* reads raw data in BGR bpp */
	if(image->header.bitsPerPixel == 1){
        if (alphaChannel == 1){
		    readbit1w(image, raw_data);
        }
        else{
	        readbit1(image, raw_data);
        }
	}
	else if(image->header.bitsPerPixel == 4){
        if (alphaChannel == 1){
		    readbit4w(image, raw_data);
        }
        else{
		    readbit4(image, raw_data);
        }
	}
	else if(image->header.bitsPerPixel == 8 && alphaChannel != 0){
        image->data_length = pixel_length * sizeof(BGRA);
	    if (image->data == nullptr){
		    image->data = (uint8_t*)malloc(image->data_length);
	    }
	    else{
		    if (image->data_length < (size_t)image->header.height * (size_t)image->header.width * sizeof(BGRA)){
			    image->data = (uint8_t*)realloc(image->data, image->data_length);
		    }
	    }
	    readbit8w(image, palette, raw_data);
    }
	else if(image->header.bitsPerPixel == 8 && alphaChannel == 0){
        image->data_length = pixel_length * sizeof(BGR);
	    if (image->data == nullptr){
		    image->data = (uint8_t*)malloc(image->data_length);
	    }
	    else{
		    if (image->data_length < pixel_length * sizeof(BGR)){
			    image->data = (uint8_t*)realloc(image->data, image->data_length);
		    }
	    }
	    readbit8(image, palette, raw_data);
    }
	else if(image->header.bitsPerPixel == 24 && alphaChannel != 0){
        image->data_length = pixel_length * sizeof(BGRA);
	    if (image->data == nullptr){
		    image->data = (uint8_t*)malloc(image->data_length);
	    }
	    else{
		    if (image->data_length < pixel_length * sizeof(BGRA)){
			    image->data = (uint8_t*)realloc(image->data, image->data_length);
		    }
	    }
	    readbit24w(image, raw_data);
    }
	else if(image->header.bitsPerPixel == 24 && alphaChannel == 0){
        image->data_length = pixel_length * sizeof(BGR);
	    if (image->data == nullptr){
		    image->data = (uint8_t*)malloc(image->data_length);
	    }
	    else{
		    if (image->data_length < pixel_length * sizeof(BGR)){
			    image->data = (uint8_t*)realloc(image->data, image->data_length);
		    }
	    }
	    readbit24(image, raw_data);
    }
	else if(image->header.bitsPerPixel == 32){
	    image->data_length = pixel_length * sizeof(BGR);
		if (image->data == nullptr){
			image->data = (uint8_t*)malloc(image->data_length);
		}
		else{
			if (image->data_length < pixel_length * sizeof(BGR)){
				image->data = (uint8_t*)realloc(image->data, image->data_length);
			}
		}
		readbit32(image, raw_data);
	}

    if (raw_data != nullptr){
	    free(raw_data);
    }
    if (palette != nullptr){
        free(palette);
    }

	fclose(file_handler);
}

/* Loads a bmp file */
gwBmpImage gw_bmp_import( const char* filename, const int alphaChannel )
{
    gwBmpImage image;
    gw_bmp_init( &image );
    bmp_load( &image, filename, alphaChannel );
    return image;
}

/* Saves the image in bmp format. Returns 0 if everything has gone ok */
int gw_bmp_save
    ( const char* filename
    , uint8_t* data
    , const unsigned int width
    , const unsigned int height
    , const int bpp             /* bits per pixel of the input image */
    , const int alpha_channel   /*1-RGBA (the output includes the alpha channel) 0-Only RGB */
    )
{
    gwBmpHeaderInfo header; 
    int padding;
	uint8_t* p = data;
    uint8_t blank[4] = {0, 0, 0, 0};
	FILE *file_handler;
    unsigned int i, j;
    unsigned int out_bpp = (alpha_channel == 1) ? 32 : 24;

    if (data == nullptr){
        _handle_error_("Cannot save nullptr data! %s", filename);
        return 2;
    }

    file_handler = fopen(filename, "wb");
    if (file_handler == nullptr){
        _handle_error_("Cannot open %s !", filename);
        return 1;
    }

    if (alpha_channel == 1){
        padding = 4 - (width * 4) % 4;
	    if (padding == 4){
		    padding = 0;
        }
    }
    else{
        padding = 4 - (width * 3) % 4;
	    if (padding == 4){
		    padding = 0;
        }
    }

	header.dataSize = (out_bpp * (width + padding)) * height;
	header.bitsPerPixel = out_bpp;
	header.compression = 0;
	header.dataOffset = 54;
	header.fileSize = 54 + header.dataSize;
	header.headerSize = 40;
	header.numberOfColors = 0;
	header.numberOfImportantColors = 0;
	header.horizontalResolution = 0;
	header.verticalResolution = 0;
	header.planes = 1;
	header.type[0] = 'B';
	header.type[1] = 'M';
	header.width = width;
	header.height = height;

	fwrite(&header, sizeof(gwBmpHeaderInfo), 1, file_handler);

    if (bpp == 24 && alpha_channel == 1){
        /* brg to rgba */
	    for (j = 0; j < height; j++){
	        for (i = 0; i < width; i++){
                fputc(*(p), file_handler);
                fputc(*(p+1), file_handler);
                fputc(*(p+2), file_handler);
                fputc(255, file_handler);
                p += 3;
            }
            fwrite(blank, sizeof(int8_t), padding, file_handler);
        }
    }
    else if (bpp == 24 && alpha_channel == 0){
        /* brg to brg case */
	    for (j = 0; j < height; j++){
	        for (i = 0; i < width; i++){
                fputc(*(p), file_handler);
                fputc(*(p+1), file_handler);
                fputc(*(p+2), file_handler);
                p += 3;
            }
            fwrite(blank, sizeof(int8_t), padding, file_handler);
        }
    }
    else if (bpp == 32 && alpha_channel == 1){
        /* bgra to rgba case */
	    for (j = 0; j < height; j++){
	        for (i = 0; i < width; i++){
                fputc(*(p), file_handler);
                fputc(*(p+1), file_handler);
                fputc(*(p+2), file_handler);
                fputc(*(p+3), file_handler);
                p += 4;
            }
            fwrite(blank, sizeof(int8_t), padding, file_handler);
        }
    }
    else if (bpp == 32 && alpha_channel == 0){
        /* bgra to rgb case */
	    for (j = 0; j < height; j++){
	        for (i = 0; i < width; i++){
                fputc(*(p), file_handler);
                fputc(*(p+1), file_handler);
                fputc(*(p+2), file_handler);
                p += 4;
            }
            fwrite(blank, sizeof(int8_t), padding, file_handler);
        }
    }
    else if (bpp == 8 && alpha_channel == 0){
        /* greyscale */
 	    for (j = 0; j < height; j++){
	        for (i = 0; i < width; i++){
                fputc(*p, file_handler);
                fputc(*p, file_handler);
                fputc(*p, file_handler);
                p++;
            }
            fwrite(blank, sizeof(int8_t), padding, file_handler);
        }
    }
    else if (bpp == 8 && alpha_channel == 1){
        /* greyscale */
 	    for (j = 0; j < height; j++){
	        for (i = 0; i < width; i++){
                fputc(*p, file_handler);
                fputc(*p, file_handler);
                fputc(*p, file_handler);
                fputc(255, file_handler);
                p++;
            }
            fwrite(blank, sizeof(int8_t), padding, file_handler);
        }
    }
    else{
        /* odd case. Fill with 0 */
 	    for (j = 0; j < height; j++){
	        for (i = 0; i < width; i++){
                fputc(0, file_handler);
                fputc(0, file_handler);
                fputc(0, file_handler);
                if (alpha_channel == 1){
                    fputc(0, file_handler);
                }
            }
            fwrite(blank, sizeof(int8_t), padding, file_handler);
        }
    }

    fclose(file_handler);

    return 0;
}

