/**
Author: Mario J. Martin <dominonurbs$gmail.com>

This function checks the encoding of a text file

*******************************************************************************/

#include <stdio.h>

#include "definitions.h"
#include "file_encoding.h"

/* Checks the text file encoding */
int check_file_encoding( FILE* fh, int* bom_length )
{
    unsigned char bom[8] = { 0 };

    fread( bom, sizeof( unsigned char ), 2, fh );
    if (bom[0] == 0xEF && bom[1] == 0xBB){
        /* Traditional ANSI encoding */

        /* There is one additional byte in the file */
        fread( bom, sizeof( unsigned char ), 1, fh );
        if (bom_length != nullptr){
            *bom_length = 3;
        }
        return ENCODING_UTF8;
    }
    else if (bom[0] == 0xFE && bom[1] == 0xFF){
        /* Unicode big endian encoding */
        if (bom_length != nullptr){
            *bom_length = 2;
        }
        return ENCODING_UTF16_BE;
    }
    else if (bom[0] == 0xFF && bom[1] == 0xFE){
        /* Unicode little endian encoding */
        if (bom_length != nullptr){
            *bom_length = 2;
        }
        return ENCODING_UTF16_LE;
    }
    else if (bom[0] == 0 && bom[1] == 0){
        /* Unicode big endian encoding with no BOM */
        if (bom_length != nullptr){
            *bom_length = 0;
        }
        rewind( fh );
        return ENCODING_UTF16_BE;
    }
    else if (bom[0] == 0x2B && bom[1] == 0x2F){
        /* UTF-7 encoding (very weird) */
        if (bom_length != nullptr){
            /* Actually, the fourth and fith bytes are not part of the BOM
            * and the fith byte may be present or not. */
            *bom_length = 4;
        }
        return ENCODING_UTF7;
    }
    else{
        /* There is no BOM bytes, so assuming its a traditional ASCII */
        if (bom_length != nullptr){
            *bom_length = 0;
        }

        return ENCODING_UTF8;
    }
}

