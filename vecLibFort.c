/*

Run-time F2C/GFORTRAN translation for Apple's vecLib BLAS/LAPACK

See README.md for full background and usage details.

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>

Michael C. Grant, 21 March 2014
mcg@cvxr.com

*/

#include <stdio.h>
#include "cloak.h"
#include <vecLib/cblas.h>

#define VOIDS_(s,i,id) COMMA_IF(i) void*
#define VOIDS(n) IF(n)(EXPR_S(0)(REPEAT_S(0,DEC(n),VOIDS_,~)),void)
#define VOIDA_(s,i,id) COMMA_IF(i) void *a ## i
#define VOIDA(n) IF(n)(EXPR_S(0)(REPEAT_S(0,DEC(n),VOIDA_,~)),void)
#define PARAM_(s,i,id) COMMA_IF(i)a ## i
#define PARAM(n) IF(n)(EXPR_S(0)(REPEAT_S(0,DEC(n),PARAM_,~)),)

#ifdef VECLIBFORT_VERBOSE
#define DEBUG(...) fprintf(stderr,__VA_ARGS__);
static const char* dynamic_msg = "Entering dynamic %s replacement\n";
static const char* static_msg = "Entering static %s replacement\n";
#define DEBUG_S(x) DEBUG( static_msg, x )
#define DEBUG_D(x) DEBUG( dynamic_msg, x )
#else
#define DEBUG(...)
#define DEBUG_S(x)
#define DEBUG_D(x)
#endif

typedef struct interpose_t_ {
  const void *replacement;
  const void *original;
} interpose_t;

typedef struct c_float_ {
  float r, i;
} c_float;

typedef struct c_double_ {
  double r, i;
} c_double;

#ifdef VECLIBFORT_INTERPOSE

/*
 * INTERPOSING MODE
 *
 * In this mode, dyld is instructed to preload this library even before the
 * executable itself. It reads the __DATA.__interpose section of the library
 * for the interpose information, which it uses to swap out the offending
 * BLAS/LAPACK functions with our replacements. Because vecLib provides two
 * aliases for each function---one with a trailing underscore, and one
 * without---we need two interpose records for each replacement.
 *
 * For instance, for "sdot", we define a static function
 *    static float my_sdot( const int* N, const float* X, const int* incX )
 * add interpose data to signify two substitutions:
 *    sdot_ -> my_sdot
 *    sdot  -> my_sdot
 */

#define INTERPOSE(name) \
__attribute__((used)) interpose_t interpose_ ## name [] \
__attribute__ ((section ("__DATA,__interpose"))) = \
{ { (const void*)&my_ ## name, (const void*)&name }, \
  { (const void*)&my_ ## name, (const void*)&name ## _ } };

#define D2F_CALL(name,n) \
extern double name( VOIDS(n) ); \
extern double name ## _( VOIDS(n) ); \
static float my_ ## name ( VOIDA(n) ) \
{ return (float)name ## _( PARAM(n) ); } \
INTERPOSE(name)

#define CPLX_CALL(type,name,n) \
extern void name( VOIDS(INC(n)) ); \
extern void name ## _( VOIDS(INC(n)) ); \
static c_ ## type my_ ## name ( VOIDA(n) ) \
{ \
  c_ ## type cplx; \
  name ## _( &cplx, PARAM(n) ); \
  return cplx; \
} \
INTERPOSE(name)

#else

/*
 * DYNAMIC SUBSTITUTION MODE
 * 
 * In this mode, we give our functions identical names, and rely on link
 * order to ensure that these take precedence over those declared in vecLib.
 * Thus whenever the main code attempts to call one of the covered functions,
 * it will be directed to one of our wrappers instead.
 *
 * Because vecLib provides two aliases for each function---one with a
 * trailing underscore, and one without---we actually need two separate
 * replacement functions (at least until we can figure out how to do aliases
 * cleanly in clang.) Each pair of replacements controls a single static
 * pointer to the replacement function. On the first invocation of either,
 * this pointer is retrieved using a dlsym() command.
 *
 * For instance, for "sdot", we define two functions
 *    float sdot_( const int* N, const float* X, const int* incX )
 *    float sdot ( const int* N, const float* X, const int* incX )
 * On the first invocation of either, the "sdot_" symbol from vecLib is
 * retrieved using the dlsym() command and stored in
 *    static void* fp_dot;
 * In theory, we could create just one replacement with two aliases, but 
 * clang has thus far been uncooperative. Any assistance on this matter would
 * be appreciated. 
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#define VECLIB_FILE "/System/Library/Frameworks/vecLib.framework/vecLib"

static void * veclib = 0;

static void unloadlib(void)
{
  DEBUG( "Unloading vecLib\n" );
  dlclose (veclib);
}

static void loadlib(void)
{
  static const char* veclib_loc = VECLIB_FILE;
  DEBUG( "Loading library: %s\n", veclib_loc )
  veclib = dlopen (veclib_loc, RTLD_LOCAL | RTLD_NOLOAD | RTLD_FIRST);
  if ( veclib == 0 ) {
    fprintf( stderr, "Failed to open vecLib library; aborting.\n   Location: %s\n", veclib );
    abort ();
  }
  atexit(unloadlib); 
}

static void* loadsym( const char* nm )
{
  if ( veclib == 0 ) loadlib();
  DEBUG( "Loading function: %s\n", nm )
  void *ans = dlsym( veclib, nm );
  if ( ans != 0 ) return ans;
  fprintf( stderr, "vecLib symbol '%s' could not be resolved; aborting.\n", nm );
  abort();
}

#define D2F_CALL_(fname,name,n) \
float fname( VOIDA(n) ) \
{ \
  DEBUG_D( #name "_" ) \
  if ( !fp_ ## name ) fp_ ## name = loadsym( #name "_" ); \
  return ((ft_ ## name)fp_ ## name)( PARAM(n) ); \
}

#define D2F_CALL(name,n) \
typedef double (*ft_ ## name)( VOIDS(n) ); \
static void *fp_ ## name = 0; \
D2F_CALL_(name,name,n) \
D2F_CALL_(name ## _,name,n)

#define CPLX_CALL_(type,fname,name,n) \
c_ ## type fname( VOIDA(n) ) \
{ \
  c_ ## type cplx; \
  DEBUG_D( #name "_" ) \
  if ( !fp_ ## name ) fp_ ## name = loadsym( #name "_" ); \
  ((ft_ ## name)fp_ ## name)( &cplx, PARAM(n) ); \
  return cplx; \
}

#define CPLX_CALL(type,name,n) \
typedef void (*f_ ## name)( VOIDS(INC(n)) ); \
static void *fp_ ## name = 0; \
CPLX_CALL_(type,name,name,n) \
CPLX_CALL_(type,name ## _,name,n)

#endif

D2F_CALL(clangb,7)
D2F_CALL(clange,6)
D2F_CALL(clangt,5)
D2F_CALL(clanhb,7)
D2F_CALL(clanhe,6)
D2F_CALL(clanhf,6)
D2F_CALL(clanhp,5)
D2F_CALL(clanhs,5)
D2F_CALL(clanht,4)
D2F_CALL(clansb,7)
D2F_CALL(clansp,5)
D2F_CALL(clansy,6)
D2F_CALL(clantb,8)
D2F_CALL(clantp,6)
D2F_CALL(clantr,8)
D2F_CALL(scsum1,3)

D2F_CALL(slamch,1)
D2F_CALL(slaneg,6)
D2F_CALL(slamc3,2)
D2F_CALL(slangb,7)
D2F_CALL(slange,6)
D2F_CALL(slangt,5)
D2F_CALL(slanhs,5)
D2F_CALL(slansb,7)
D2F_CALL(slansp,5)
D2F_CALL(slanst,4)
D2F_CALL(slansy,6)
D2F_CALL(slantb,8)
D2F_CALL(slantp,6)
D2F_CALL(slantr,8)
D2F_CALL(slapy2,2)
D2F_CALL(slapy3,3)

#if defined(VECLIBFORT_DYNBLAS) || defined(VECLIBFORT_INTERPOSE)

D2F_CALL(sdsdot,6)
D2F_CALL(sdot,5)
D2F_CALL(snrm2,3)
D2F_CALL(sasum,3)

CPLX_CALL(float,cdotu,5)
CPLX_CALL(float,cdotc,5)
D2F_CALL(scnrm2,3)
D2F_CALL(scasum,3)

CPLX_CALL(double,zdotu,5)
CPLX_CALL(double,zdotc,5)

#else

/*
 * STATIC SUBSTITUTION MODE
 * 
 * For BLAS functions, we have access to CBLAS versions of each function.
 * So the hoops we need to jump through to resolve the name clashes in the
 * dynamic substitution mode can be avoided. Instead, we simply create the
 * replacement functions to call the CBLAS counterparts instead.
 *
 * To void duplicating code, we've stored these functions in "static.h",
 * and load them twice---once for the functions with trailing underscores
 * (e.g., "sdot_"), and once without (e.g., "sdot"). In theory, we could
 * create just one replacement with two aliases, but clang has thus far been
 * uncooperative. Any assistance on this matter would be appreciated.
 */

#include "static.h"
#define ADD_UNDERSCORE
#include "static.h"

#endif


