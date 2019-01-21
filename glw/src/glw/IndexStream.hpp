/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Data structure for working with Index Buffer Objects
These indicates the connectivities of the vertex and the primitive
(points, lines, or surfaces)
It might be empty, indicating that connectivities are implicit
in the vertex stream.

*******************************************************************************/

#ifndef _GWH_INDEX_STREAM_H
#define _GWH_INDEX_STREAM_H

#include <stdlib.h>

#include "common/log.h"
#include "common/check_malloc.h"

#include "defines.h"
#include "gwgl.h"
#include "gwdata.h"

#include "signatures.h"

/* A convenient macro to offset buffers */
#define BUFFER_OFFSET(bytes) ((GLubyte*)NULL + (bytes))

namespace gw
{

/* Holds the index data that is sent to the GPU.
* A mesh is attached to a vertex stream and an index stream.
* The index vertex stream holds the connectivities of the elements */
class IndexStream : public gwIndexStream
{

public:
    /* Used to check that the casting returns a legitime class */
    int signature;

    /* Index Buffer Object.
    * This is given by OpenGL when the buffer is created in the GPU. */
    GLuint ibo;

    /* @usage indicates how the buffer is used.Possible values are
    * GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY,
    * GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY,
    * GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, or GL_DYNAMIC_COPY.
    * Usually for index streams the most common usage is GL_STATIC_DRAW */
    GLenum usage;

    /* Length of indexes stored in the buffer */
    GLsizei buffer_count;

    IndexStream()
    {
        stream = nullptr;
        length = 0;
        primitive = GW_INDEX_POINTS;
        ibo = 0;
        usage = GL_STATIC_DRAW;
        buffer_count = 0;
        signature = _GW_SIGNATURE_INDEX;
        update = 0;
    }

    ~IndexStream()
    {
        free( stream );
        stream = nullptr;
        this->length = 0;
        if (ibo != 0){
            _warning_( "Index stream buffer %i has not been released, before OpenGL exist.", ibo )
        }
        ibo = 0;
        signature = 0;
    }

    /* Clears the buffer in the GPU. */
    void deleteBuffer()
    {
        if (ibo != 0){
            /* Releases the GPU buffer */
            glDeleteBuffers( 1, &ibo );
            ibo = 0;
            buffer_count = 0;
        }
    }

    /* Creates stream with the especified size.
    * The current data is destroyed, but the GPU buffer is not touched. */
    void alloc( const size_t len )
    {
        if (len > 0){
            _check_( stream = (gwIndex*)_calloc_( len, sizeof( gwIndex ) ) );
            if (stream != nullptr){
                length = len;
            }
        }
    }

    /* Clears the memory in the CPU. The GPU buffer remains active. */
    void freeMemory()
    {
        free( stream );
        stream = nullptr;
        length = 0;
    }

    /* Gets the index type */
    inline const GLenum indexType() const
    {
        if (sizeof( gwIndex ) == sizeof( GLuint )){
            return GL_UNSIGNED_INT;
        }
        else if (sizeof( gwIndex ) == sizeof( GLushort )){
            return GL_UNSIGNED_SHORT;
        }
        else if (sizeof( gwIndex ) == sizeof( GLubyte )){
            return GL_UNSIGNED_BYTE;
        }
        else{
            _warning_( "Unsupported index size" );
            return GL_UNSIGNED_INT;
        }
    }

    /* Creates or updates an index buffer object */
    void updateBuffer()
    {
        if (stream == nullptr || length == 0){
            /* The stream is empty. It is not neccesary an error. 
             * The geometry can be defined with the primitive and the vertex stream */
            return;
        }

        if (buffer_count == length && ibo > 0){
            /* Updates current vertex buffer */
            /* Hook the buffer to the context */
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );

            /* Update the data to the buffer;
            * by calling glBufferData() will recreates the whole buffer;
            * glBufferSubData() avoids the cost of reallocating. */
            glBufferSubData
                ( GL_ELEMENT_ARRAY_BUFFER, 0
                , sizeof( gwIndex ) * buffer_count
                , stream
                );
        }
        else{
            /* Creates a new buffer object */
            if (ibo > 0){
                /* Releases current IBO if neccesary */
                glDeleteBuffers( 1, &ibo );
            }

            /* Request a new buffer */
            glGenBuffers( 1, &ibo );

            if (ibo == 0){
                _handle_error_( "Failed to create the index buffer object!" );
                buffer_count = 0;
            }
            buffer_count = length;

            /* Hook the buffer to the context */
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );

            /* Copy data to the buffer */
            glBufferData( GL_ELEMENT_ARRAY_BUFFER
                , sizeof( gwIndex ) * buffer_count, stream, usage );
        }
    }

    /* @primitive indicates the type of the elements.
    * Posible values are(using OpenGL 4.1 reference)
    * GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES,
    * GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY,
    * GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES,
    * GL_TRIANGLE_STRIP_ADJACENCY, GL_TRIANGLES_ADJACENCY and GL_PATCHES
    * Notice that GL_QUADS are deprecated since OpenGL 3.x
    *
    * The primitive is defined in the handler as GW_xxx
    * with the supported ones. */
    inline const GLenum gl_primitive() const
    {
        switch (primitive){
        case GW_INDEX_POINTS:          return GL_POINTS;
        case GW_INDEX_LINES:           return GL_LINES;
        case GW_INDEX_LINE_LOOP:       return GL_LINE_LOOP;
        case GW_INDEX_LINE_STRIP:      return GL_LINE_STRIP;
        case GW_INDEX_TRIANGLES:       return GL_TRIANGLES;
        case GW_INDEX_TRIANGLE_STRIP:  return GL_TRIANGLE_STRIP;
        default:                        return GL_POINTS;
        }
    }
};

} // end namespace

#endif
