/*
* File:   Stream.hpp
* Author: Mario J. Martin <dominonurbs$gmail.com>
*
* This class is C++ version of C-like arrays,
* similar in many ways to to std::vector, but using the concept of stream
* It just make easier to work with arrays.
*/

#ifndef _Stream_HPP
#define _Stream_HPP

#ifndef __cplusplus
#error "This class requires a C++ compiler."
#endif

#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "definitions.h"
#include "log.h"
#include "check_malloc.h"

template< typename T >
class Stream {

public:
    /** DATA */
    T* stream;

    /** Total size of the stream in number of elements */
    size_t _length;

protected:
    bool free_memory;

public:

    /** Reserves memory */
    void alloc( const size_t size )
    {
        if (size != _length){
            /* Release previous data */
            free( stream );
            _length = 0;
            if (size > 0){
                _check_( stream = (T*)_malloc_( sizeof( T ) * size ) );
                if (stream == nullptr){
                    /* Out of memory */
                    return;
                }
                else{
                    _length = size;
                }
            }
        }
    }

    /** Copies data to the stream */
    void copy( const T* y, const size_t length ){
        alloc( length );
        if (stream != nullptr){
            memcpy( stream, y, sizeof( T )*length );
        }
    }

    /** Default constructor */
    explicit Stream(){
        _length = 0;
        stream = nullptr;
        free_memory = true;
    }


    /** Creates a stream of data with capacity length */
    /* @param length fixed length of the stream
    **/
    explicit Stream( const size_t length ){
        alloc( length );
        free_memory = true;
    }

    /** Copy constructor. Actually it creates a reference */
    Stream( const Stream& y ){
        this->stream = y.stream;
        this->_length = y._length;
        this->free_memory = false;
    }

    /** Default initializer. */
    const Stream& init(){
        _length = 0;
        stream = nullptr;
        free_memory = false;

        return (*this);
    }

    /** Destructor */
    ~Stream(){
        // release memory resources
        if (free_memory == true){
            free( stream );
            _length = 0;
            free_memory = false;
        }
    }


    /** Releases the memory. The notation dispose() is taken from C# */
    void dispose(){
        if (free_memory == true){
            free( stream );
        }
        stream = nullptr;
        _length = 0;
    }

    /* Access to the data using the x[i] notation */
    T& operator [] ( const size_t index ) const{
        if (this->stream == nullptr){
            throw("\nStream has not been initialized!");
        }
        if ((size_t)index >= _length){
            throw("\nIndex out of range!");
        }
        if (index < 0){
            throw("\nNegative index!");
        }

        return *(stream + index);
    }

    /** Assign an unique value to all elements of the array */
    const Stream< T >& fill( const T& value ){
        for (T* p = stream; p < stream + _length; p++){
            *p = value;
        }

        return *this;
    }
};

#endif    /* _STREAM_HPP */

