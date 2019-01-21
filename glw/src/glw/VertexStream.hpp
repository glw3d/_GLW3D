/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Data structure for working with Vertex Buffer Objects
These contains the information of each vertex, position, normal, and texcoord

*******************************************************************************/

#ifndef _HGW_VERTEX_STREAM_H
#define _HGW_VERTEX_STREAM_H

#include <stddef.h>
#include <stdlib.h>

#include "common/log.h"
#include "common/check_malloc.h"

#include "gwgl.h"
#include "gwdata.h"

#include "signatures.h"


/* A convenient macro to offset buffers */
#define BUFFER_OFFSET(bytes) ((GLubyte*)NULL + (bytes))

namespace gw
{
/* Holds the vertex data that is sent to the GPU.
* A mesh is attached to a vertex stream and an index stream.
* The primary vertex stream holds the coord, normals, and texture coord */
template< typename VStreamT, typename VType >
class BaseVertexStream : public VStreamT
{
public:

    /* Vertex Buffer Object.
    * This is given by OpenGL when the buffer is created in the GPU. */
    GLuint vbo;

    /* @usage indicates how the buffer is used. Possible values are:
    * GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY,
    * GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY,
    * GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, or GL_DYNAMIC_COPY.
    * Usually for vertex streams the most common is GL_DYNAMIC_DRAW */
    GLenum usage;

    /* Size of the buffer in the GPU. */
    GLsizei buffer_count;

    /* signature for safer upcasting */
    int signature;

    /* Used to indicate that the stream comes from an eternal source */
    int free_data;

    BaseVertexStream()
    {
        VStreamT::stream = nullptr;
        VStreamT::length = 0;
        vbo = 0;
        usage = GL_DYNAMIC_DRAW;
        buffer_count = 0;
        signature = _GW_SIGNATURE_VERTEX;
        update = 0;
        free_data = 0;
    }

    ~BaseVertexStream()
    {
        if (free_data){
            free( VStreamT::stream );
        }
        VStreamT::stream = nullptr;
        VStreamT::length = 0;
        if (vbo != 0){
            _warning_( "Vertex stream buffer %i has not been released," 
                "before OpenGL exist.", vbo )
        }
        vbo = 0;
        signature = 0;
    }

    /* Creates a OpenGL vertex object buffer in the GPU.
     * Returns 1 if the buffer has been updated. */
    void updateBuffer(const int buffer_len)
    {
        if (VStreamT::stream == nullptr || buffer_len == 0){
            return;
        }

        if (buffer_count == buffer_len && vbo > 0){
            /* Updates current vertex buffer */
            /* Hook the buffer to the context */
            glBindBuffer( GL_ARRAY_BUFFER, vbo );

            /* Update the data to the buffer;
            * by calling glBufferData() will recreates the whole buffer;
            * glBufferSubData() avoids the cost of reallocating. */
            glBufferSubData
                ( GL_ARRAY_BUFFER, 0
                , sizeof( VType ) * buffer_count
                , VStreamT::stream
                );
        }
        else{
            /* Creates a new buffer */
            if (vbo > 0){
                /* Releases current VBO if neccesary */
                glDeleteBuffers( 1, &vbo );
            }

            /* Request a new buffer */
            glGenBuffers( 1, &vbo );

            if (vbo == 0){
                _handle_error_( "Failed to generate the vertex buffer object!" );
                buffer_count = 0;
                return;
            }

            /* Save the size of the current buffer */
            buffer_count = buffer_len;

            /* Hook the buffer to the context */
            glBindBuffer( GL_ARRAY_BUFFER, vbo );

            /* Copy data to the GPU buffer */
            glBufferData( GL_ARRAY_BUFFER
                , sizeof( VType ) * buffer_count, VStreamT::stream, usage );
        }
    }


    /* Clears the buffer in the GPU. */
    void deleteBuffer()
    {
        if (vbo != 0){
            /* Releases the GPU buffer */
            glDeleteBuffers( 1, &vbo );
            vbo = 0;
            buffer_count = 0;
        }
    }

    /* Creates stream with the especified size.
    * The current data is destroyed, but the GPU buffer is not touched. */
    void alloc( const size_t len )
    {
        if (len > 0){
            if (free_data != 0){
                /* delete previous data, if any */
                free( this->stream );
            }
            free_data = 1;
            _check_( this->stream = (VType*)_calloc_
                ( len, sizeof( VType ) ) );
        }
        if (VStreamT::stream != nullptr){
            VStreamT::length = len;
        }
        else{
            VStreamT::length = 0;
        }
    }

    /* Access the data using regular array operator */
    gwVertex& operator[]( const int i )
    {
        return VStreamT::stream[i];
    }

    /* Access the data using regular array operator */
    const gwVertex& operator[]( const int i ) const
    {
        return this->stream[i];
    }
};


/* Especialization for the default vertex format
* with fields for position, normal and texture coordinates*/
class VertexStream : public BaseVertexStream < gwVertexStream, gwVertex >
{
public:
    /* Enables client vertex data format */
    static inline void enable
        ( const int attPosition, const int attNormal, const int attTexture )
    {
        glEnableVertexAttribArray( attPosition );

        if (attNormal >= 0){
            glEnableVertexAttribArray( attNormal );
        }

        if (attTexture >= 0){
            glEnableVertexAttribArray( attTexture );
        }
    }

    /* Sets the vertex data format */
    static inline void attribFormat
        ( const int attPosition, const int attNormal, const int attTexCoord )
    {
        glVertexAttribPointer( attPosition, 3, GL_FLOAT, GL_FALSE
            , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, position ) );

        if (attNormal >= 0){
            glVertexAttribPointer( attNormal, 3, GL_FLOAT, GL_TRUE
                , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, normal ) );
        }

        if (attTexCoord >= 0){
            glVertexAttribPointer( attTexCoord, 2, GL_FLOAT, GL_FALSE
                , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, texCoord ) );
        }
    }

    /* Sets the vertex data format */
    static inline void attribFormatArray
        ( const int attPosition, const int attNormal, const int attTexCoord )
    {
        glVertexAttribPointer( attPosition, 3, GL_FLOAT, GL_FALSE
            , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, position ) );
        glEnableVertexAttribArray( attPosition );
        if (attNormal >= 0){
            glVertexAttribPointer( attNormal, 3, GL_FLOAT, GL_TRUE
                , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, normal ) );
            glEnableVertexAttribArray( attNormal );
        }
        if (attTexCoord >= 0){
            glVertexAttribPointer( attTexCoord, 2, GL_FLOAT, GL_FALSE
                , sizeof( gwVertex ), (GLvoid*)offsetof( gwVertex, texCoord ) );
            glEnableVertexAttribArray( attTexCoord );
        }
    }

    /* Disables attribute client state */
    static inline void disable
        ( const int attPosition, const int attNormal, const int attTexture )
    {
        glDisableVertexAttribArray( attPosition );

        if (attNormal >= 0){
            glDisableVertexAttribArray( attNormal );
        }
        if (attTexture >= 0){
            glDisableVertexAttribArray( attTexture );
        }
    }
};


/* Especialization for a single scalar format */
class ScalarStream : public BaseVertexStream < gwScalarStream, gwFloat >
{
public:
    /* Enables client vertex data format */
    static inline void enable
        ( const int attScalar )
    {
        if (attScalar >= 0){
            glEnableVertexAttribArray( attScalar );
        }
    }

    /* Sets the vertex data format */
    static inline void attribFormat( const int attScalar )
    {
        if (attScalar >= 0){
            glVertexAttribPointer( attScalar, 1, GL_FLOAT, GL_FALSE
                , sizeof( GLfloat ), BUFFER_OFFSET( 0 ) );
        }
    }

    /* Sets the vertex data format */
    static inline void attribFormatArray( const int attScalar )
    {
        if (attScalar >= 0){
            glVertexAttribPointer( attScalar, 1, GL_FLOAT, GL_FALSE
                , sizeof( GLfloat ), BUFFER_OFFSET( 0 ) );
            glEnableVertexAttribArray( attScalar );
        }
    }

    /* Disables attribute client state */
    static inline void disable( const int attScalar )
    {
        if (attScalar >= 0){
            glDisableVertexAttribArray( attScalar );
        }
    }
};

/* Especialization for three dimensional vector format */
class VectorStream : public BaseVertexStream < gwVectorStream, gwVector3f >
{
public:
    /* Enables client vertex data format */
    static inline void enable
        ( const int attVector )
    {
        if (attVector >= 0){
            glEnableVertexAttribArray( attVector );
        }
    }

    /* Sets the vertex data format */
    static inline void attribFormat( const int attVector )
    {
        if (attVector >= 0){
            glVertexAttribPointer( attVector, 3, GL_FLOAT, GL_FALSE
                , 3 * sizeof( GLfloat ), BUFFER_OFFSET( 0 ) );
        }
    }

    /* Sets the vertex data format */
    static inline void attribFormatArray( const int attVector )
    {
        if (attVector >= 0){
            glVertexAttribPointer( attVector, 3, GL_FLOAT, GL_FALSE
                , 3 * sizeof( GLfloat ), BUFFER_OFFSET( 0 ) );
            glEnableVertexAttribArray( attVector );
        }
    }

    /* Disables attribute client state */
    static inline void disable( const int attVector )
    {
        if (attVector >= 0){
            glDisableVertexAttribArray( attVector );
        }
    }
};

} // end namespace

#endif /* _HGW_VERTEX_STREAM_H */
