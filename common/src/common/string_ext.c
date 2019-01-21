/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Extension of methods for handling strings that usually are missed
in the standard library
*******************************************************************************/


#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "definitions.h"

/* Eliminates spaces, tabs and line breaks at the beginning and the end
* of the string. In traditional ASCII there is no character above 127.
* If a character has a value like 0xFF it could means that is using
* some form of extended ASCII, which we should not care, because it
* is deprecated in favor of Unicode, but more probably is a BOM
* that appears a the begining of the text file, which may cause troubles.*/
char* str_trim( char *str )
{
    size_t length;
    char *p1;
    char *p0;

    if (str == nullptr)
        return str;

    length = strlen( str );

    /* Cut the right */
    for (p1 = &str[length - 1]
        ; (*p1 == ' ' || *p1 == '\t' || *p1 == '\n' || *p1 == '\r' || *p1 == 13)
        && p1 >= &str[0];)
        *p1-- = '\0';

    /* Cut the left */
    for (p1 = &str[0]
        ; (*p1 == ' ' || *p1 == '\t' || *p1 == '\n' || *p1 == '\r' || *p1 == 13 || *p1 > 127)
        && p1 <= &str[length]; p1++);

    /* Move the uncut content to the beginning of the string */
    for (p0 = str; *p1 != '\0'; p0++, p1++)
        *p0 = *p1;

    *p0 = '\0';

    return str;
}

/* Finds the first non space caracter */
char* str_tokspaces( char *str )
{
    char* p = str;

    while (p != '\0'
        && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == 13)
        && *p < 128)
    {
        p++;
    }

    return p;
}

/* Converts a character to uppercase. */
char char_uppercase( const char c )
{
    char r;

    if (c >= 'a' && c <= 'z'){
        r = c + 'A' - 'a';
    }
    else{
        r = c;
    }

    return r;
}


/* Converts a character to lowercase. */
char char_lowercase( const char c )
{
    char r;

    if (c >= 'A' && c <= 'Z'){
        r = c - 'A' + 'a';
    }
    else{
        r = c;
    }

    return r;
}


/* Less strict comparisson between two strings. Also ignores the uppercase. */
int str_check( const char *str1, const char *str2 )
{
    size_t i = 0;
    size_t j = 0;
    char c;
    char b;

    /* Trim blank characters and spaces */
    while (str1[i] == ' '
        || str1[i] == '\t'
        || str1[i] == '\n'
        || str1[i] == '\r'
        )
    {
        i++;
    }

    while (str2[j] == ' '
        || str2[j] == '\t'
        || str2[j] == '\n'
        || str2[j] == '\r'
        )
    {
        j++;
    }

    /* Compare */
    b = char_lowercase( str1[i] );
    c = char_lowercase( str2[j] );
    while (b != '\0'){
        if (c == '\0'){
            return 1;
        }
        else if (b != c){
            return 1;
        }
        else{
            i++;
            j++;
            b = char_lowercase( str1[i] );
            c = char_lowercase( str2[j] );
        }
    }

    return 0;
}


/* Compares if the beggining of the string match the second string. */
int str_match( const char* str1, const char* str2 )
{
    int i = 0;

    while (str1[i] != '\0' && str2[i] != '\0'){
        if (str1[i] != str2[i]){
            return 0;
        }
        i++;
    }

    return 1;
}


/* Convers all characters in a string to upper case. */
char* str_upcase( char *str )
{
    char *p = str;

    while (*p != 0)
    {
        if (*p >= 'a' && *p <= 'z')
            *p = *p + ('A' - 'a');
        p++;
    }

    return str;
}


/* Convers all characters in a string to upper case. */
char* str_downcase( char *str )
{
    char *p = str;

    while (*p != 0)
    {
        if (*p >= 'A' && *p <= 'a')
            *p = (*p - 'A') + 'a';
        p++;
    }

    return str;
}


/* Copies a string in safe mode. */
void strcpy_safe
( char *dest
, const char* orig
, const int buffer_size /* maximun number of characters including the '\0' */
)
{
    int i = 0, max_length = buffer_size - 1;
    const char* p_orig = orig;
    char* p_dest = dest;

    if (dest == orig){
        return;
    }
    if (orig == nullptr){
        *p_dest = '\0';
        return;
    }

    while (i < max_length && *p_orig != '\0'){
        /* According to Bejarne Stroutstrup, it is not safe to write *p++ = *b++;
        because it is not clear the order of the operands '=' and '++'
        for the right term. */
        *p_dest = *p_orig;
        p_dest++;
        p_orig++;
        i++;
    }
    *p_dest = '\0';
}


/* Copies or clones a string */
char* str_clone( char* dest, const char* source )
{
    const char* p = source;
    char* out = nullptr;
    int length = 0;

    if (source == nullptr){
        return nullptr;
    }

    if (dest == nullptr){
        p = source;
        while (*p != '\0'){
            length++;
            p++;
        }
        length++;

        out = (char*)malloc( length * sizeof( char ) );
        if (out == nullptr){
            /* Out of memory */
            return nullptr;
        }
    }
    else{
        out = dest;
    }

    memcpy( out, source, length * sizeof( char ) );

    return out;
}


/* Concatenates a string in safe mode */
void strcat_safe
( char *dest
, const char* orig
, const int buffer_size
)
{
    int i = 0;
    int max_length = buffer_size - 1;
    const char* p_orig = orig;
    char* p_dest = dest;

    for (; *p_dest != '\0' && i < max_length - 1; p_dest++, i++);

    while (i++ < max_length - 1 && *p_orig != '\0'){
        *p_dest = *p_orig;
        p_dest++;
        p_orig++;
    }
    *p_dest = '\0';
}

/* Gets the file extension */
const char* str_getfile_ext( const char* fullpathname )
{
    const char* p = fullpathname;
    const char* ext = fullpathname;
    while (*p != '\0'){
        if (*p == '.'){
            p++;
            ext = p;
        }
        p++;
    }

    return ext;
}

/* Gets a clear name, path and extension from a path filename*/
void parse_filename_path
( char *filename      /** buffer with a clear filename, eg "file" */
, char *path          /** buffer with the path, eg "c:/folder/" */
, char *extension     /** buffer with the extension, eg ".txt" */
, const char* fullpathname  /** full path and filename, eg "c:/folder/file.txt" */
)
{
    const char* p_slash = nullptr;
    const char* p_dot = nullptr;
    const char* p = fullpathname;
    const char* p_end;
    char* p_dest;

    if (path != nullptr){
        *path = '\0';
    }
    if (extension != nullptr){
        *extension = '\0';
    }
    if (filename != nullptr){
        *filename = '\0';
    }

    if (fullpathname == nullptr)
        return;

    while (*p != '\0'){
        if (*p == '/' || *p == '\\'){
            p_slash = p;
        }
        if (*p == '.'){
            p_dot = p;
        }
        p++;
    }
    p_end = p;

    /* Copy the extension */
    if (extension != nullptr && p_dot != nullptr){
        p_dest = extension;
        p = p_dot + 1;
        while (*p != '\0'){
            *p_dest = *p;
            p_dest++;
            p++;
        }
        *p_dest = '\0';
    }

    /* Copy the path */
    if (path != nullptr && p_slash != nullptr){
        p_dest = path;
        p = fullpathname;
        while (*p != '\0' && p <= p_slash){
            *p_dest = *p;
            p_dest++;
            p++;
        }
        *p_dest = '\0';
    }

    /* Copy the name */
    if (filename != nullptr){
        if (p_dot == nullptr){
            p_dot = p_end;
        }
        if (p_slash == nullptr){
            p_slash = fullpathname;
        }
        p = p_slash + 1;
        p_dest = filename;
        while (p < p_dot){
            *p_dest = *p;
            p_dest++;
            p++;
        }
        *p_dest = '\0';
    }
}


/* Concatentes the filename with the path */
void strcat_path_filename
( char* out_string, const char* path, const char* filename, const int max_len )
{
    int n;

    if (path != nullptr){
        n = strlen( path );
        out_string[0] = '\0';
        strcpy_safe( out_string, path, max_len );

        if (n > 0 && path[n - 1] != '\\' && path[n - 1] != '/'){
            strcat_safe( out_string, "/", max_len );
        }
        strcat_safe( out_string, filename, max_len );
    }
    else{
        strcpy_safe( out_string, filename, max_len );
    }
}


/* Find the first character in a string. */
const char* find_character
( const char* str, const char* characteres )
{
    const char* p = str;
    const char* c = characteres;

    while (*p != '\0'){
        c = characteres;
        while (*c != '\0'){
            if (*p == *c){
                return p;
            }
            c++;
        }
        p++;
    }

    return p;
}


/* Extracts the filename form the full path. */
void extract_filename_from_fullpath
( char* buf, const char* filename, const int buf_len )
{
    size_t index = 0;
    size_t slash = 0;
    size_t count = 0;
    const char* p = filename;

    /* Find the last slash or ':' */
    while (*p != '\0'){
        if (*p == ':' || *p == '\\' || *p == '/'){
            slash = index;
        }

        count++;
        index++;
        p++;
    }

    strcpy_safe( buf, &filename[slash + 1], buf_len );
}


/* Extracts the filename form the full path (wide char version). */
wchar_t* extract_filename_from_fullpath_w
( wchar_t* name, const wchar_t* filename )
{
    size_t index = 0;
    size_t slash = 0;
    size_t count = 0;
    const wchar_t* p = filename;

    // Find the last slash or :
    while (*p != L'\0'){
        if (*p == L':' || *p == L'\\' || *p == L'/'){
            slash = index;
        }

        count++;
        index++;
        p++;
    }

    wcscpy( name, &filename[slash + 1] );

    return name;
}


/* Copies a string from char to a string in wide char. */
wchar_t* wcscpy_from_char( wchar_t* dest, const char* source )
{
    const char* p = source;
    wchar_t* d = dest;

    while (*p != '\0'){
        *d = (wchar_t)*p;
        p++;
        d++;
    }
    *d = L'\0';

    return dest;
}


/* Copies a string from wide char to a string in char.
* Take care that converting unicode characters to 8 bit may
* obtain 'undefined behaviours'. */
char* wchar2char( char* dest, const wchar_t* source )
{
    const wchar_t* p = source;
    char* d;
    char* out = dest;
    int length = 0;

    if (source == nullptr){
        return nullptr;
    }

    if (dest == nullptr){
        while (*p != '\0'){
            length++;
            p++;
        }
        length++;

        out = (char*)calloc( length, sizeof( char ) );
        if (out == nullptr){
            /* Out of memory */
            return nullptr;
        }
    }

    p = source;
    d = out;
    while (*p != '\0'){
        *d = (char)*p;
        p++;
        d++;
    }
    *d = '\0';

    return dest;
}


/* Copies a string from char to a string in wide char.
* The lower 128 characters of the unicode are the same as ASCII */
wchar_t* char2wchar( wchar_t* dest, const char* source )
{
    const char* p = source;
    wchar_t* d;
    wchar_t* out = dest;
    int length = 0;

    if (source == nullptr){
        return nullptr;
    }

    if (dest == nullptr){
        length = strlen( source );
        length++;

        out = (wchar_t*)calloc( length, sizeof( wchar_t ) );
        if (out == nullptr){
            /* Out of memory */
            return nullptr;
        }
    }
    d = out;
    while (*p != '\0'){
        if (*p >= 0x20 && *p <= 0x7E){
            *d = (wchar_t)(*p);
        }
        else{
            *d = '?';
        }
        p++;
        d++;
    }

    *d = '\0';

    return out;
}




