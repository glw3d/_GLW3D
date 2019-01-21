/*
* Author: Mario J. Martin <dominonurbs$gmail.com>
*
* Some methods to resize images.
*/

#ifndef _MG_IMAGE_EDIT_H
#define _MG_IMAGE_EDIT_H

#include <stdint.h>

#include "common/definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

    /* Resizes a 1D image using linear interpolation. */
    uint8_t* gw_image_resize_linear
        ( const uint8_t* orig_data
        , const int orig_width
        , const int target_width
        , const int bytes_per_pixel
        );

    /* Resizes 2D image using bilinear interpolation. */
    uint8_t* gw_image_resize_bilinear
        ( const uint8_t* orig_data
        , const int orig_width
        , const int orig_height
        , const int target_width
        , const int target_height
        , const int bytes_per_pixel
        );

    /* Calculates the closest power of 2. The result is always iqual or below the original size */
    int gw_image_getPowerof2( const int size );

#ifdef	__cplusplus
}
#endif

#endif /* _MG_IMAGE_EDIT_H */

