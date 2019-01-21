/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Extension of methods for handling strings that usually are missed
in the standard library
*******************************************************************************/

#ifndef _STRinG_ext_H
#define _STRinG_ext_H

#include <stdlib.h>

#ifdef  __cplusplus
extern "C" {
#endif

    /** Eliminates spaces, tabs and line breaks at the beginning and the end
    * of the string. */
    char* str_trim( char *str );


    /** Finds the first non space caracter */
    char* str_tokspaces( char *str );


    /** None strict comparisson between two strings. Also ignores the uppercase. */
    int str_check( const char *check, const char *base );


    /** Compares if the beggining of the string match the second string. */
    int str_match( const char* str1, const char* str2 );


    /** Converts a character to uppercase. */
    char char_uppercase( const char c );


    /** Converts a character to lowercase. */
    char char_lowercase( const char c );


    /** Convers all characters in a string to upper case. */
    char* str_upcase( char *str );


    /** Convers all characters in a string to upper case. */
    char* str_downcase( char *str );


    /** Copies a string in safe mode. */
    void strcpy_safe
        ( char *dest
        , const char* orig
        , const int buffer_size
        );


    /** Copies or clones a string */
    char* str_clone( char* dest, const char* source );


    /** Cconcatenates a string in safe mode */
    void strcat_safe
        ( char *dest
        , const char* orig
        , const int buffer_size
        );

    /** Gets the file extension */
    const char* str_getfile_ext( const char* fullpathname );

    /** Gets a clear name, path and extension from a path filename*/
    void parse_filename_path
        ( char *filename      /** buffer with a clear filename "file" */
        , char *path          /** buffer with the path "c:/folder/" */
        , char *extension     /** buffer with the extension ".txt" */
        , const char* fullpathname  /** full path and filename "c:/folder/file.txt" */
        );

    /** Concatentes the filename with the path */
    void strcat_path_filename
        ( char* out_string
        , const char* path
        , const char* filename
        , const int max_len
        );

    /** Find the first character in a string. */
    const char* find_character
        ( const char* str
        , const char* characteres
        );

    /** Extracts the filename form the full path. */
    void extract_filename_from_fullpath
        ( char* buf, const char* filename, const int buf_len );


    /** Extracts the filename form the full path (wide char version). */
    wchar_t* extract_filename_from_fullpath_w
        ( wchar_t* name, const wchar_t* filename );


    /** Copies a string from char to a string in wide char. */
    wchar_t* wcscpy_from_char( wchar_t* dest, const char* source );


    /** Copies a string from wide char to a string in char.
    * Take care that converting unicode characters to 8 bit char may
    * obtain undefined behaviours. */
    char* wchar2char( char* dest, const wchar_t* source );


    /** Copies a string from char to a string in wide char.
    * The lower 128 characters of the unicode are the same as ASCII */
    wchar_t* char2wchar( wchar_t* dest, const char* source );


#ifdef  __cplusplus
}
#endif

#endif /* _STRinG_ext_H */

