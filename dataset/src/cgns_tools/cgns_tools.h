/***
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#ifndef CGNS_TOOLS_H
#define CGNS_TOOLS_H

#include "stdlib.h"

#include "dataset/dataset_arrays.h"
#include "cgnsGrid.h"

#define ADP_FILL_INT (-2147483647L)

#ifdef  __cplusplus
extern "C" {
#endif

/** Imports a surface grid in CGNS file format */
cgnsGrid* cgnsGrid_import( const char* filename, int* OUTPUT );


#ifdef  __cplusplus
}
#endif

#endif /* CGNS_TOOLS_H */
