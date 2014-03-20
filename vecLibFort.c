/*

Run-time F2C/GFORTRAN translation for Apple's vecLib BLAS/LAPACK

See README.md for full background and usage details.

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

#define U(x) x ## _

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
static ft_ ## name *fp_ ## name = 0; \
D2F_CALL_(name,name,n) \
D2F_CALL_(name ## _,name,n)

#define CPLX_CALL_(type,fname,name,n) \
c_ ## type fname( VOIDA(n) ) \
{ \
  c_ ## type cplx; \
  static void* func = 0; \
  DEBUG_D( #name "_" ) \
  if ( !fp_ ## name ) fp_ ## name = loadsym( #name "_" ); \
  ((ft_ ## name)fp_ ## name)( &cplx, PARAM(n) ); \
  return cplx; \
}

#define CPLX_CALL(type,name,n) \
typedef void (*f_ ## name)( VOIDS(INC(n)) ); \
static ft_ ## name *fp_ ## name = 0; \
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

float sdsdot_( const int* N, const float* alpha, const float* X, const int* incX, const float* Y, const int* incY )
{
  DEBUG_S( "sdsdot" )
  return cblas_sdsdot( *N, *alpha, X, *incX, Y, *incY );
}

float sdot_( const int* N, const float* X, const int* incX, const float* Y, const int* incY )
{
  DEBUG_S( "sdot" )
  return cblas_sdot( *N, X, *incX, Y, *incY );
}

float snrm2_( const int* N, const float* X, const int* incX )
{
  DEBUG_S( "snrm2" )
  return cblas_snrm2( *N, X, *incX );
}

float sasum_( const int* N, const float *X, const int* incX )
{
  DEBUG_S( "sasum" )
  return cblas_sasum( *N, X, *incX );
}

c_float cdotu_( const int* N, const void* X, const int* incX, const void* Y, const int* incY )
{
  DEBUG_S( "cdotu" )
  c_float ans;
  cblas_cdotu_sub( *N, X, *incX, Y, *incY, &ans );
  return ans;
}

c_float cdotc_( const int* N, const void* X, const int* incX, const void* Y, const int* incY )
{
  DEBUG_S( "cdotc" )
  c_float ans;
  cblas_cdotc_sub( *N, X, *incX, Y, *incY, &ans );
  return ans;
}

float scnrm2_( const int* N, const void* X, const int* incX )
{
  DEBUG_S( "scnrm2" )
  return cblas_scnrm2( *N, X, *incX );
}

float scasum_( const int* N, const void *X, const int* incX )
{
  DEBUG_S( "scasum" )
  return cblas_scasum( *N, X, *incX );
}

c_double zdotu_( const int* N, const void* X, const int* incX, const void* Y, const int* incY )
{
  DEBUG_S( "zdotu" )
  c_double ans;
  cblas_zdotu_sub( *N, X, *incX, Y, *incY, &ans );
  return ans;
}

c_double zdotc_( const int* N, const void* X, const int* incX, const void* Y, const int* incY )
{
  DEBUG_S( "zdotc" )
  c_double ans;
  cblas_zdotc_sub( *N, X, *incX, Y, *incY, &ans );
  return ans;
}

#endif


