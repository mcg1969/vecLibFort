/*

Run-time F2C/GFORTRAN translation for Apple's vecLib BLAS/LAPACK

The BLAS and LAPACK libraries shipped as part of Apple's Accelerate
framework---at least for 64-bit Intel Macs---use an F2C-style calling
convention. Unfortunately, newer gfortran compilers use a different
convention by default. It is still possible to force gfortran to use
the F2C convention using the -ff2c flag, but some minor differences
between the two apparently remain. The purpose of this code is to
provide gfortran-compatible entry points for those BLAS and LAPACK
routines that are affected by this incompatibility. Two types of
adaptations are necessary:

--- Functions that return single-precision values in gfotran
    return double-precision values in F2C. The replacement code
    automatically downcasts the F2C result to 
--- Functions that return single- or double-precision complex
    values 

The inspiration for this code came from Octave's "blaswrap.c" code.
Indeed, when constructed without BUILD_INTERPOSE defined, this code
mimics what that code does, but with several differences:
--- Instead of building a lookup table for all of the functions,
    each function separately resolves and stores a pointer to
    the function it is replacing.
--- This resolution process occurs the first time a given function
    is called; only those functions actually used will be loaded.
--- All of the relevant BLAS and LAPACK functions are represented.

Furthermore, when BUILD_INTERPOSE is defined, you can compile this
code to be a shared library that use's DYLD's built-in "interpose"
capability. This allows you to "fix" code that has already been
compiled. To use it, create a shared library---call it, say,
"veclib_gfort.dylib"---and call the program thusly:

DYLIB_INSERT_LIBRARIES=/path/to/veclib_gfort.dylib program

This causes the interpose library to be loaded before the program
and its dependencies, and its rewritten functions will be swapped
in for the originals automatically.

This approach is explained by Amit Singh in his book 
"Mac OSX Internals", section 2.6.3.4. http://osxbook.com
It's frankly not difficult to find examples of this approach all
over the web. Point your favorite search engine to "__interpose osx"
or "DYLD_INTERPOSE" for examples and discussion.

*/

#include <stdio.h>
#include "cloak.h"

#define VOIDS_(s,i,id) COMMA_IF(i) void*
#define VOIDS(n) IF(n)(EXPR_S(0)(REPEAT_S(0,DEC(n),VOIDS_,~)),void)
#define VOIDA_(s,i,id) COMMA_IF(i) void *a ## i
#define VOIDA(n) IF(n)(EXPR_S(0)(REPEAT_S(0,DEC(n),VOIDA_,~)),void)
#define PARAM_(s,i,id) COMMA_IF(i)a ## i
#define PARAM(n) IF(n)(EXPR_S(0)(REPEAT_S(0,DEC(n),PARAM_,~)),)

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

#ifdef BUILD_INTERPOSE

#define INTERPOSE(name) \
__attribute__((used)) interpose_t interpose_ ## name \
__attribute__ ((section ("__DATA,__interpose"))) = \
{ (const void*)&my_ ## name, (const void*)&name };

#define D2F_CALL(name,n) \
extern double name( VOIDS(n) ); \
static float my_ ## name ( VOIDA(n) ) \
{ return (float)name( PARAM(n) ); } \
INTERPOSE(name)

#define CPLX_CALL(type,name,n) \
extern void name( VOIDS(INC(n)) ); \
static c_ ## type my_ ## name ( VOIDA(n) ) \
{ \
  c_ ## type cplx; \
  name( &cplx, PARAM(n) ); \
  return cplx; \
} \
INTERPOSE(name)

#else

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#define VECLIB_FILE "/System/Library/Frameworks/vecLib.framework/vecLib"

static void * veclib = 0;

static void unloadlib(void)
{
  dlclose (veclib);
}

static void loadlib(void)
{
  static const char* veclib_loc = VECLIB_FILE;
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
  void *ans = dlsym( veclib, nm );
  if ( ans != 0 ) return ans;
  fprintf( stderr, "vecLib symbol '%s' could not be resolved; aborting.\n", nm );
  abort();
}

#define D2F_CALL(name,n) \
typedef double (*f_ ## name)( VOIDS(n) ); \
float name ( VOIDA(n) ) \
{ \
  static void* func = 0; \
  if ( !func ) func = (f_ ## name)loadsym( #name ); \
  return ((f_ ## name)func)( PARAM(n) ); \
}

#define CPLX_CALL(type,name,n) \
typedef void (*f_ ## name)( VOIDS(INC(n)) ); \
c_ ## type name ( VOIDA(n) ) \
{ \
  c_ ## type cplx; \
  static void* func = 0; \
  if ( !func ) func = loadsym( #name ); \
  ((f_ ## name)name)( &cplx, PARAM(n) ); \
  return cplx; \
}

#endif

D2F_CALL(clangb_,7)
D2F_CALL(clange_,6)
D2F_CALL(clangt_,5)
D2F_CALL(clanhb_,7)
D2F_CALL(clanhe_,6)
D2F_CALL(clanhf_,6)
D2F_CALL(clanhp_,5)
D2F_CALL(clanhs_,5)
D2F_CALL(clanht_,4)
D2F_CALL(clansb_,7)
D2F_CALL(clansp_,5)
D2F_CALL(clansy_,6)
D2F_CALL(clantb_,8)
D2F_CALL(clantp_,6)
D2F_CALL(clantr_,8)
D2F_CALL(scsum1_,3)

D2F_CALL(slamch_,1)
D2F_CALL(slaneg_,6)
D2F_CALL(slamc3_,2)
D2F_CALL(slangb_,7)
D2F_CALL(slange_,6)
D2F_CALL(slangt_,5)
D2F_CALL(slanhs_,5)
D2F_CALL(slansb_,7)
D2F_CALL(slansp_,5)
D2F_CALL(slanst_,4)
D2F_CALL(slansy_,6)
D2F_CALL(slantb_,8)
D2F_CALL(slantp_,6)
D2F_CALL(slantr_,8)
D2F_CALL(slapy2_,2)
D2F_CALL(slapy3_,3)

D2F_CALL(sdsdot_,6)
D2F_CALL(sdot_,5)
D2F_CALL(snrm2_,3)
D2F_CALL(sasum_,3)

CPLX_CALL(float,cdotu_,5)
CPLX_CALL(float,cdotc_,5)
D2F_CALL(scnrm2_,3)
D2F_CALL(scasum_,3)

CPLX_CALL(double,zdotu_,5)
CPLX_CALL(double,zdotc_,5)


