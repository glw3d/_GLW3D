/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Some useful macros that wrap malloc, calloc, and realloc

*******************************************************************************/


/* For debugging memory leaks call _CrtDumpMemoryLeaks() in MSVC.
* Redefining malloc and calloc might make nuts some compilers. */
#ifdef  _MSC_VER 
    #ifdef _DEBUG  
        #include <crtdbg.h>
        #define _malloc_(size) \
            _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__)

        #define _calloc_(count, size) \
            _calloc_dbg(count, size, _NORMAL_BLOCK, __FILE__, __LINE__)

        #define _realloc_(target, size) \
            _realloc_dbg(target, size, _NORMAL_BLOCK, __FILE__, __LINE__)

        #ifndef DBG_NEW
            #define DBG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
            #define new DBG_NEW
        #endif
	#else
		#define _malloc_(size) malloc(size)
		#define _calloc_(count, size) calloc(count, size)
		#define _realloc_(target, size) realloc(target, size)
    #endif  /* _DEBUG */
#else
    #define _malloc_(size) malloc(size)
    #define _calloc_(count, size) calloc(count, size)
    #define _realloc_(target, size) realloc(target, size)
#endif  /* _MSC_VER */
