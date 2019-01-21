 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    Basis types and defitions

*******************************************************************************/

#ifndef _NURBS_DEFINITiONS_H
#define _NURBS_DEFINITiONS_H

#define DOMINO_NURBS_LABEL_LEN 128  /* IGES labels has only 8 caracters */

/*------------------------------------------------------------------------------
      DEFINITIONS
------------------------------------------------------------------------------*/
//#define _TAU_

typedef double _NurbsFloat;

/* Main precission */
#ifdef _TAU_
  #include "tau_typedefs.h"       /* This is somewhere in TAU */
  #define NurbsFloat TauDouble
#endif

#ifndef NurbsFloat
    #define NurbsFloat _NurbsFloat
#endif

/*------------------------------------------------------------------------------
      BASIC DATA STRUCTURES 
------------------------------------------------------------------------------*/

/** Three dimensional vector. */
typedef struct 
{
    NurbsFloat x;
    NurbsFloat y;
    NurbsFloat z;

}_NurbsVector3;

/** Four dimensional vector. */
typedef struct 
{
    NurbsFloat x;
    NurbsFloat y;
    NurbsFloat z;
    NurbsFloat w;

}_NurbsVector4;

/* A three component vector data structures (not yet a standard type in C). */
#ifndef NurbsVector3 
    #define NurbsVector3 _NurbsVector3
#endif

/* A four component vector data structures (not yet a standard type in C). */
#ifndef NurbsVector4 
    #define NurbsVector4 _NurbsVector4
#endif

#define NURBS_ERROR_VALUE (9.9692099683868690e+36f)

#endif /* _NURBS_DEFINITiONS_H */

/**/
