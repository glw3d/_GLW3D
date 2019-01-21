/**
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#ifndef _HGW_STREAM_HPP
#define _HGW_STREAM_HPP

#include "signatures.h"
#include "defines.h"
#include "gwdata.h"
#include "gwmath.h"
#include "gwcamera.h"
#include "signatures.h"
#include "gwcolors.h"

namespace gw
{
    /* It is the equivalent to a C array */
    template< typename T >
    struct Stream
    {
        /* Data */
        T* stream;

        /* Total length of the stream */
        size_t length;

        Stream()
        {
            stream = nullptr;
            length = 0;
        }

        explicit Stream( const size_t len )
        {
            alloc( len );
        }

        ~Stream()
        {
            free( stream );
            stream = nullptr;
            length = 0;
        }

        /* Assign the stream with the same value pattern */
        void fill( const T value )
        {
            memset( stream, value, sizeof( T )*len );
            /*
            for (size_t i = 0; i < length; i++){
                stream[i] = value;
            }
            */
        }

        /* Reserves memory. If there is alredy data, it will realloc the size
        * and copy the original data. */
        void alloc( const size_t len )
        {
            if (len == 0){
                free( stream );
                length = 0;
            }
            else if (stream == nullptr){
                _check_( stream = (T*)_calloc_( len, sizeof( T ) ) );
                if (stream != nullptr){
                    length = len;
                }
            }
            else if (stream != nullptr && length != len){
                T* buffer = nullptr;
                _check_( buffer = (T*)_calloc_( len, sizeof( T ) ) );
                if (buffer != nullptr){
                    size_t s = len < length ? len : length;
                    memcpy( buffer, stream, s * sizeof( T ) );
                    free( stream );
                    stream = buffer;
                    length = len;
                }
            }
        }

        /* Reserves memory, deleting current data and assigning the value
        * If the value is 0, it is equivalent to a calloc. */
        void alloc( const size_t len, const T value )
        {
            if (len == 0){
                free( stream );
                length = 0;
            }
            else if (stream == nullptr){
                if (value == 0){
                    _check_( stream = (T*)_calloc_( len, sizeof( T ) ) );
                }
                else{
                    _check_( stream = (T*)_malloc_( sizeof( T )*len ) );
                }
                if (stream != nullptr){
                    length = len;
                    fill( value );
                }
            }
            else if (stream != nullptr && length != len){
                T* buffer = nullptr;
                if (value == 0){
                    _check_( buffer = (T*)_calloc_( len, sizeof( T ) ) );
                }
                else{
                    _check_( buffer = (T*)_malloc_( sizeof( T )*len ) );
                }
                if (buffer != nullptr){
                    free( stream );
                    stream = buffer;
                    length = len;
                    fill( value );
                }
            }
        }

        T& operator[]( const size_t i ) const
        {
            if (i < length){
                return stream[i];
            }
            else{
                _handle_error_( "Out of bounds!" );
                return *stream;
            }
        }

        T& operator[]( const int i ) const
        {
            if (i >= 0 && (size_t)i < length){
                return stream[i];
            }
            else{
                _handle_error_( "Out of bounds!" );
                return *stream;
            }
        }
    };
}

/******************************************************************************/

/* Dynamic Stack for storing pointer of objects with undefined size.
* They are very flexible for adding, removing and swapping,
* but it is not very efficient for index accesing.
* The basic architecture is a chain of containers */
template< typename T >
struct DynamicStack
{
public:
    T* obj;   /* Object pointer */
    struct DynamicStack* next; /* Pointer to the next element in the stack */
    struct DynamicStack* from; /* Pointer to the previous element */

    DynamicStack()
    {
        obj = nullptr;
        next = nullptr;
        from = nullptr;
    }

    ~DynamicStack()
    {
        next = nullptr;
        from = nullptr;
        obj = nullptr;
    }

    void add( T* p_obj )
    {
        if (this->obj == nullptr){
            /* Assined to the root */
            this->obj = p_obj;
        }
        else{
            /* Create a new container and attach to the last one */
            DynamicStack* new_token = new DynamicStack;
            new_token->obj = p_obj;

            /* Find the last */
            DynamicStack* last = this;
            while (last->next != nullptr){
                last = last->next;
            }
            last->next = new_token;
            new_token->from = last;
        }
    }

    /* This function is suppose to be called from the root token */
    void remove( const T* p_obj )
    {
        /* Find the container */
        DynamicStack* token = this;
        while (token != nullptr){
            if (token->obj == p_obj){
                /* Delete the container */
                DynamicStack* next = token->next;
                if (next != nullptr){
                    next->from = token->from;
                }
                if (token->from != nullptr){
                    token->from->next = next;
                }
                delete(token);
                token = next;
            }
            else{
                token = token->next;
            }
        }
    }

    /* Checks if the object is already in the list */
    bool seek( const T* p_obj )
    {
        /* Find the container */
        DynamicStack* token = this;
        while (token != nullptr){
            if (token->obj == p_obj){
                return true;
            }
            token = token->next;
        }

        return false;
    }


    /* Clears the whole stack. This is suppose to be called from the root */
    void clear()
    {
        DynamicStack* token = this->next;
        DynamicStack* next = nullptr;

        while (token != nullptr){
            next = token->next;
            delete(token);
            token = next;
        }
    }

    /* Access an object by its index position */
    T* operator[]( const size_t n )
    {
        DynamicStack* token = this;
        T* obj = token->obj;

        size_t i = 0;
        while (token != nullptr && i < n){
            token = token->next;
            obj = (token == nullptr) ? nullptr : token->obj;
            i++;
        }

        return obj;
    }

    /* Iterates through all objetcs stored and performs a lambda function */
    void iterate( void( *call( T* item ) ) )
    {
        DynamicStack< T >*token = this;
        while (token != nullptr){
            T* obj = token->obj;
            if (obj != nullptr){
                call( obj );
            }
            token = token->next;
        }
    }
};

#endif _HGW_STREAM_HPP
/**/
