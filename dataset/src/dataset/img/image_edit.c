/*
 * Author: Mario J. Martin <dominonurbs$gmail.com>
 *
 * Routines used for image resizing
 */

#include <stdlib.h>
#include <math.h>

#include "image_edit.h"

/* Resizes a 1D image using linear interpolation. */
uint8_t* gwImage_resize_linear
( const uint8_t* orig_data
, const int orig_width
, const int target_width
, const int bytes_per_pixel
){
    int x, c, x0, x1;
    size_t index = 0;
    double xx, xnt, value;
    uint8_t f0, f1;
    uint8_t* data = nullptr;

    if (orig_data == nullptr || orig_width <= 0 || target_width <= 0){
        return nullptr;
    }

    data = (uint8_t*)malloc( sizeof( uint8_t ) * bytes_per_pixel * target_width );
    if (data == nullptr){
        return nullptr;
    }

    for (x = 0; x < target_width; x++){
        for (c = 0; c < bytes_per_pixel; c++){
            xx = ((double)x * orig_width) / target_width;

            /* obtain the fractional and integer parts */
            xx = modf( xx, &xnt );

            x0 = (int)xnt;
            x1 = x0 + 1;
            if (x1 >= orig_width){
                x1 = orig_width - 1;
            }

            /* calculate the interpolated value */
            f0 = orig_data[x0 * bytes_per_pixel + c];
            f1 = orig_data[x1 * bytes_per_pixel + c];
            value = f0 * (1 - xx) + f1 * xx;

            /* get the fractional part to round the solution */
            xx = modf( value, &xnt );
            data[index++] = (xx < 0.5f) ? (uint8_t)(value) : (uint8_t)(value + 1);
        }
    }

    return data;
}

/** Resizes 2D image using bilinear interpolation. */
uint8_t* gwImage_resize_bilinear
( const uint8_t* orig_data
, const int orig_width
, const int orig_height
, const int target_width
, const int target_height
, const int bytes_per_pixel
){
    uint8_t* data = nullptr;
    uint8_t f00, f01, f10, f11;
    int x, y, c, x0, y0, x1, y1;
    size_t index = 0;
    double xx, yy, xnt, ynt, value;

    if (orig_data == nullptr || orig_width <= 0 || orig_height <= 0
        || target_width <= 0 || target_height <= 0){

        return nullptr;
    }

    data = (uint8_t*)malloc( sizeof( uint8_t ) * bytes_per_pixel * target_width * target_height );
    if (data == nullptr){
        return nullptr;
    }

    for (y = 0; y < target_height; y++){
        for (x = 0; x < target_width; x++){
            for (c = 0; c < bytes_per_pixel; c++){
                xx = ((double)x*orig_width) / target_width;
                yy = ((double)y*orig_height) / target_height;

                /* obtain the integer and fractional parts */
                xx = modf( xx, &xnt );
                yy = modf( yy, &ynt );

                x0 = (int)xnt;
                y0 = (int)ynt;
                x1 = x0 + 1;
                if (x1 >= orig_width){
                    x1 = orig_width - 1;
                }
                y1 = y0 + 1;
                if (y1 >= orig_height){
                    y1 = orig_height - 1;
                }

                /* calculate the interpolated value */
                f00 = orig_data[((x0)+(y0)*orig_width) * bytes_per_pixel + c];
                f10 = orig_data[((x1)+(y0)*orig_width) * bytes_per_pixel + c];
                f01 = orig_data[((x0)+(y1)*orig_width) * bytes_per_pixel + c];
                f11 = orig_data[((x1)+(y1)*orig_width) * bytes_per_pixel + c];
                value = f00 * (1 - xx) * (1 - yy) + f10 * xx * (1 - yy)
                    + f01 * (1 - xx) * yy + f11 * xx * yy;

                /* get the fractional part to round the solution */
                xx = modf( value, &xnt );
                data[index++] = (xx < 0.5f) ? (uint8_t)(value) : (uint8_t)(value + 1);
            }
        }
    }

    return data;
}

