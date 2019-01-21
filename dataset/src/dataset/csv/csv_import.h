/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Imports CSV (Comma Sepparated Variables) text file
*******************************************************************************/

#ifndef HCSV_IMPORT_H
#define HCSV_IMPORT_H

#define FILL_DOUBLE	(9.9692099683868690e+36)
#define FILL_FLOAT	(9.9692099683868690e+36f)

#include <stdlib.h>

#include "dataset/dataset_arrays.h"

/* Array of strings */
typedef struct
{
    char buffer[1024];
    char* var_names[128];
    size_t num_vars;
} csvVarNames;

typedef struct
{
    /** Variable values */
    double* stream;

    /** Number of varables */
    size_t num_vars;

    /** Length of the variables */
    size_t var_len;

}csvDataSet;

#ifdef  __cplusplus
extern "C" {
#endif

    /* Reads the first line with the names of the variables,
    * and count the total length of the variables */
    EXTERNDL
        csvVarNames csv_inquiry( const char* filename, size_t* var_len );

    /* Reads the variables in a CSV file.
    * Several variable names are separated with commas.
    * 'len' returns the total size of the array.
    * A fill value FILL_DOUBLE is set, if a variable name does not exist or
    * there is a problem reading it. */
    EXTERNDL
        csvDataSet* csv_import
        ( const char* filename, const char* var_names );

#ifdef  __cplusplus
}
#endif

#endif /* HCSV_IMPORT_H */
