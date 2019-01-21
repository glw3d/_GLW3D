/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Some definitions required for compliance between different ANSI C and C++
standards and 'non-standard' compiler definitions. Current standards are
C11 and C++11; most of the features are slowly integrated in GCC and VSC
There are also widespread programming notations to deal with.
*******************************************************************************/

#ifndef _COMMON_DEFiniTIONS_H
#define _COMMON_DEFiniTIONS_H

/* Compatibility for alternative widely used C basic types short names. */
#ifndef __GNUC__
        typedef unsigned long ulong;
        typedef unsigned int uint;
        typedef unsigned short ushort;
        typedef unsigned char uchar;
    #else
    #ifndef _SYS_TYPES_H
        /* in gcc they are defined in sys/types.h  */
        #include <sys/types.h>
    #endif
#endif

/* MinGW does not define these types, event it uses gcc ?:( */
#ifdef __MINGW32__
    typedef unsigned long ulong;
    typedef unsigned int uint;
    typedef unsigned short ushort;
    typedef unsigned char uchar;
#endif

#ifndef __RPCNDR_H__
    typedef unsigned char byte;
#endif

/* nullptr is a standard keyword in C++11 and it is not the same as
* NULL (defined in stdio.h), although they are somehow compatibles.
* nullptr is recommended even in pure ANSI C sources, because in case of
* a cross compilation there is a potential for 'undefined behaviours'.
* Support for nullptr was added in GCC 4.6.0
* (GCC is no longer a pure C compiler). */
#ifdef  __cplusplus
#    undef _NULLPTR_CONST_

#    ifdef _MSC_VER
#        if (_MSC_VER >= 1600) 
#            define _NULLPTR_CONST_
#        endif
#    endif    /* _MSC_VER */

#    ifdef __GNUC__
#        if ( (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 6) )
#            define _NULLPTR_CONST_
#        endif
#    endif    /* __GNUC__ */

#    ifndef _NULLPTR_CONST_
#        define _NULLPTR_CONST_
#        define nullptr 0
#    endif

#else /* ANSI C */
#include <stdlib.h>
#    define nullptr NULL /* same as ((void*)0) defined in stdio.h or stdlib.h */
#endif

/* restrict is a keyword introduced for pointer declarations in C99
 * to limit the pointer aliasing and allows C codes to achieve 
 * the same performance a Fortran. However C++ does not have estandad support
 * for restrict, but most compilers that work with both C and C++ 
 * have their equivalent. Although MSVC messes everything as they use 'restrict' 
 * keyword for other purposes. For this reason we have to use the ugly macro */
#if __cplusplus
    #ifdef _MSC_VER      
        #define _restrict_ __restrict
    //#else
    //    #ifndef (__GNUC__) && ((__GNUC__ > 3)) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
    //        #define _restrict_ /* No effect */
    //    #else
    //        #define _restrict_ __restrict
    //    #endif
    #endif
#else
    #define _restrict_ restrict
#endif

/* Portability issue between C++, ANSI C and other compilers.
* Boolean type (bool) is standard in C++ and ISO C99
* Enums are equivalent to integer types; however never rely
* on booleans when passing structures from C++ to a library in C,
* because it would corrupt the data because of a wrong alignment. */
#ifndef  __cplusplus
    /* In C99 the boolean types are defined in stdbool.h
    * Unsurprisingly, VSC is not suporting the standard.
    * There are troubles using true and false keywords in C,
    * with data structures also employed in C++ codes.
    * To enable booleans in GCC use the -std=c99
    * Some developpers prefer to use BOOL, TRUE and FALSE(notice the caps)
    * and define them as equivalent integer values.
    *
    * Booleans make codes easier to read.
    */
    #ifndef __bool_true_false_are_defined
        /* Don't define bool, true, and false in C++, except as a GNU extension. */
        #ifndef __cplusplus
            #define bool _Bool
            #define true 1
            #define false 0
        #elif defined(__GNUC__) && !defined(__STRICT_ANSI__)
            /* Define _Bool, bool, false, true as a GNU extension. */
            #define _Bool bool
            #define bool  bool
            #define false false
            #define true  true
        #endif
        #define __bool_true_false_are_defined 1
    #endif
#endif

/* The inline key word is standard in C99 and C++, but not in C89,
* also fussy some compilers which not follow the standard like MSVC. */
#ifndef __cplusplus
#   ifndef __GNUC__
#      define inline __inline
#   endif
#endif

/* This can be used for avoiding the MS "un-standard" CLR safe warnings spam :(
* However a better alternative is to use _CRT_SECURE_NO_DEPRECATE
* in the compiler preprocessor options. */
/*
#if _MSC_VER >= 1400
#  define pragma warning(disable : 4996)
#endif
*/

/* Importing and exporting dynamic libraries with windows version */
#if defined(_MSC_EXPORT_DLL) /* define to compile the library as .dll with MVSC */
#  if defined(DLL_EXPORT)    /* define when building the library */
#   define EXTERNDL __declspec(dllexport)
#  else
#   define EXTERNDL __declspec(dllimport)
#  endif
#else
#define EXTERNDL 
#endif	/* defined(_MSC_EXPORT_DLL) */

/* As far I know constexpr is not yet supported by any compiler.
* This is to remember me this future feature in in C.
#define constexpr const
*/

/* Identifies the operative system */
#if _WIN32
#   define SYSTEM_WINDOWS
#endif
#if _WIN64 
#   define SYSTEM_WINDOWS
#endif
#if __WIN32__
#   define SYSTEM_WINDOWS
#endif
#if __TOS_WIN__ 
#   define SYSTEM_WINDOWS
#endif
#if __WINDOWS__
#   define SYSTEM_WINDOWS
#endif

#ifdef __APPLE__
#   ifdef __MACH__
#       define SYSTEM_MAC
#   endif
#endif

#ifdef __linux__
#   define SYSTEM_LINUX
#endif

#endif  /* _COMMON_DEFiniTIONS_H */

