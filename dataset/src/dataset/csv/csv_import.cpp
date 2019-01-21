/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Imports CSV (Comma Sepparated Variables) text file
*******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/definitions.h"
#include "common/log.h"
#include "common/check_malloc.h"

#include "csv_import.h"

struct CcsvDataSet : public csvDataSet
{
    CcsvDataSet()
    {
        stream = nullptr;
        num_vars = 0;
        var_len = 0;
    }

    ~CcsvDataSet()
    {
        free( stream );
        num_vars = 0;
        var_len = 0;
    }
};

/* Eliminates the spaces at the beggining and the end */
static char* str_trim( char *str )
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

/* Parse a string separated by commas. Return the number of tokens */
static size_t parse_line( char* buffer, char** p_tokens )
{
    int num_var = 1;
    char* c = &(buffer[0]);
    p_tokens[0] = c;
    while (*c != '\0' && num_var < 128){
        if (*c == ','){
            p_tokens[num_var] = c + 1;
            num_var++;
            *c = '\0';
        }
        c++;
    }

    return num_var;
}

/* Reads the first line with the definitions of the variables */
csvVarNames csv_inquiry( const char* filename, size_t* pvar_len )
{
    char line_buf[1024];
    csvVarNames var_names;

    *pvar_len = 0;
    var_names.buffer[0] = '\0';
    var_names.var_names[0] = '\0';
    var_names.num_vars = 0;

    if (filename == nullptr){
        _handle_error_( "null filename" );
        return var_names;
    }

    FILE* fh = fopen( filename, "rt" );
    if (fh == NULL){
        _handle_error_( "Cannot open filename %s", filename );
        return var_names;
    }

    /* Read the first line */
    fgets( var_names.buffer, 1024, fh );

    /* Count the number of lines */
    size_t num_lines = 0;
    while (fgets( line_buf, 1024, fh ) != NULL){
        num_lines++;
    }
    *pvar_len = num_lines; /* The first line is the header */

    fclose( fh );

    /* Count the number of commas and identify the var names */
    size_t num_vars = parse_line( var_names.buffer, var_names.var_names );
    var_names.num_vars = num_vars;

    return var_names;
}

/* Reads the variables in a CSV file.
* Several variable names are separated with commas.
* 'len' returns the total size of the array.
* If a variable name does not exist or there is a problem reading it,
* a fill value FILL_FLOAT is set. */
csvDataSet* csv_import( const char* filename, const char* in_var_names )
{
    char buffer[1024];
    char* pvarname[128] = {};
    char line[1024];
    char* pvarvalue[128] = {};
    int pvarindex[128] = {};

    for (int i = 0; i < 128; i++){
        pvarindex[i] = -1;
    }

    size_t var_len = 0;
    csvVarNames names_in_file = csv_inquiry( filename, &var_len );
    csvDataSet* dataset = new csvDataSet;
    dataset->num_vars = 0;
    dataset->var_len = 0;
    dataset->stream = nullptr;

    if (names_in_file.num_vars == 0){
        _warning_( "Dataset is empty!" );
        return dataset;
    }
    if (in_var_names == nullptr || strlen( in_var_names ) == 0){
        _warning_( "There are no var names!" );
        return dataset;
    }

    size_t var_names_len = strlen( in_var_names );
    /* Get the var names */
    memcpy( buffer, in_var_names, sizeof( char )*(var_names_len + 1) );
    size_t num_input_var = parse_line( buffer, pvarname );

    /* Get the index of the variables */
    for (size_t i = 0; i < num_input_var; i++){
        pvarname[i] = str_trim( pvarname[i] );
        for (size_t j = 0; j < names_in_file.num_vars; j++){
            if (strcmp( pvarname[i], names_in_file.var_names[j] ) == 0){
                pvarindex[i] = j;
                break;
            }
        }
    }

    /* Allocate memory */
    _check_( dataset->stream = (double*)_malloc_
        ( sizeof( double )* num_input_var * var_len ) );

    if (dataset->stream == nullptr){
        _handle_error_( "Out of memory!" );
        return dataset;
    }

    dataset->num_vars = num_input_var;
    dataset->var_len = var_len;

    /* Read the data stream */
    FILE* fh = fopen( filename, "rt" );
    if (fh == NULL){
        _handle_error_( "Cannot open filename %s", filename );
        return dataset;
    }

    /* Read the header */
    fgets( line, 1024, fh );

    /* Read the values */
    int jline = 0;
    double* pvar = &(dataset->stream[0]);
    while (fgets( line, 1024, fh ) != NULL){
        parse_line( line, pvarvalue );
        for (size_t ivar = 0; ivar < num_input_var; ivar++){
            int icol = pvarindex[ivar];
            if (icol >= 0 && pvarvalue[icol] != nullptr){
                float value = FILL_FLOAT;
                sscanf( pvarvalue[icol], "%f", &value );
                pvar[ivar] = value;
            }
            else{
                pvar[ivar] = FILL_FLOAT;
            }
        }
        jline++;
        pvar += num_input_var;
    }

    fclose( fh );

    return dataset;
}

