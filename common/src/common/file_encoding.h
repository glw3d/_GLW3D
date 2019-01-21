/**
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#include <stdio.h>

#define ENCODING_UTF8       0 /* Traditional ASCII file */
#define ENCODING_UTF16_LE   1 /* wchat_t in the form 60 00 65 00 */ 
#define ENCODING_UTF16_BE   2 /* wchar_t in the form 00 60 00 65 */
#define ENCODING_UTF7       3 /* Ass odd as can be */

#ifdef  __cplusplus
extern "C" {
#endif

    /* Checks the text file encoding */
    int check_file_encoding( FILE* fh, int* bom_length );

#ifdef  __cplusplus
}
#endif

