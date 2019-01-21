/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Basic data structures.
adpStreams are used to make an easy integration with Python. 
Python wrap with SWIG does not support well C-like arrays like double*.

*******************************************************************************/

#ifndef DATASET_ARRAYS_H
#define DATASET_ARRAYS_H

#include "common/definitions.h"

#define owDouble double

/* Basic three dimensional vector */
typedef struct
{
    owDouble x, y, z;
} owVector3d;

/* Array of integers */
typedef struct
{
    int* stream;
    size_t length;
} owIntStream;

/* Array of doubles */
typedef struct
{
    owDouble* stream;
    size_t length;
} owDoubleStream;

/* Array of vectors */
typedef struct
{
    owVector3d* stream;
    size_t length;
} owVector3dStream;


#ifdef  __cplusplus
extern "C" {
#endif

/* Creates a C-like array of type double */
owDoubleStream* owDoubleStream_create( const size_t len );

/* Deletes a C-like array of type double */
void owDoubleStream_free( owDoubleStream* a );

/* Creates a C-like array of type integer */
owIntStream* owIntStream_create( const size_t len );

/* Deletes a C-like array of type integer */
void owIntStream_free( owIntStream* a );

/* Creates a C-like array of three-dimensional vectors */
owVector3dStream* owVector3dStream_create( const size_t len );

/* Deletes a C-like array of three-dimensional vectors */
void owVector3dStream_free( owVector3dStream* a );

#ifdef  __cplusplus
}
#endif

#endif /* DATASET_ARRAYS_H */
