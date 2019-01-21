 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    Definitions intended to used only internally.
    This header goes in all .c files, but not in any .h 
    Originally its main purpose was to easier the link with TAU

*******************************************************************************/ 


#ifndef _NURBS_INTERNAL_H
#define _NURBS_INTERNAL_H

//#define _DOMINO_OLDFORMAT_SUPPORT

#include "common/check_malloc.h"
#include "nurbs_definitions.h"

/* Alternative definitions */
#ifdef _TAU_
  #ifndef nullptr
    #include <stdio.h>
    #define nullptr NULL
  #endif

  #include "check_malloc.h" /* This is somewhere in TAU */
  #include "util.h"      /* This is somewhere in TAU */

  #define _log_(...) tau_msg(__VA_ARGS__)
  #define _handle_error_(...) tau_msg(__VA_ARGS__)

  #ifndef  __bool_true_false_are_defined
    #define bool int
    #define true 1
    #define false 0
  #endif
#else
  /* nullptr and inline definitions are taken from here.  */
  /* This are required for better compatibility with C++ and cross compilers. */
  #include "common/definitions.h"

  /* trace() is only used for debugging; it should do nothing in release mode */
  #include "common/log.h"

#endif

#endif /*_NURBS_INTERNAL_H */

/**/
