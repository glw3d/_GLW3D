/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Constructors and destructors.

*******************************************************************************/

#include <stdlib.h>

#include "common/definitions.h"
#include "common/log.h"
#include "common/check_malloc.h"

#include "dataset_arrays.h"

extern "C"
owDoubleStream* owDoubleStream_create( const size_t len )
{
    owDoubleStream* obj = new owDoubleStream;

    obj->stream = nullptr;
    obj->length = 0;
    
    if (len > 0){
        _check_( obj->stream = (double*)_malloc_( sizeof( double ) * len ) );
        if (obj->stream != nullptr){
            obj->length = len;
        }
    }

    return obj;
}

extern "C"
void owDoubleStream_free( owDoubleStream* obj )
{
    if (obj != nullptr){
        free( obj->stream );
        obj->stream = nullptr;
        obj->length = 0;
        delete(obj);
    }
}

extern "C"
owIntStream* owIntStream_create( const size_t len )
{
    owIntStream* obj = new owIntStream;

    obj->stream = nullptr;
    obj->length = 0;
    
    if (len > 0){
        _check_( obj->stream = (int*)_malloc_( sizeof( int ) * len ) );
        if (obj->stream != nullptr){
            obj->length = len;
        }
    }

    return obj;
}

extern "C"
void owIntStream_free( owIntStream* obj )
{
    if (obj != nullptr){
        free( obj->stream );
        obj->stream = nullptr;
        obj->length = 0;
        delete(obj);
    }
}

extern "C"
owVector3dStream* owVector3dStream_create( const size_t len )
{
    owVector3dStream* obj = new owVector3dStream;
    obj->stream = nullptr;
    obj->length = 0;

    if (len > 0){
        _check_( obj->stream = (owVector3d*)_malloc_
            ( sizeof( owVector3d ) * len ) );
        if (obj->stream != nullptr){
            obj->length = len;
        }
    }

    return obj;
}

extern "C"
void owVector3dStream_free( owVector3dStream* obj )
{
    if (obj != nullptr){
        free( obj->stream );
        obj->stream = nullptr;
        obj->length = 0;
        delete(obj);
    }
}


