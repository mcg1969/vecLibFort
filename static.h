/*

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

#ifdef ADD_UNDERSCORE
#define FNAME(x) x ## _
#else
#define FNAME(x) x
#endif

float FNAME(sdsdot)( const int* N, const float* alpha, const float* X, const int* incX, const float* Y, const int* incY )
{
  DEBUG_S( "sdsdot" )
  return cblas_sdsdot( *N, *alpha, X, *incX, Y, *incY );
}

float FNAME(sdot)( const int* N, const float* X, const int* incX, const float* Y, const int* incY )
{
  DEBUG_S( "sdot" )
  return cblas_sdot( *N, X, *incX, Y, *incY );
}

float FNAME(snrm2)( const int* N, const float* X, const int* incX )
{
  DEBUG_S( "snrm2" )
  return cblas_snrm2( *N, X, *incX );
}

float FNAME(sasum)( const int* N, const float *X, const int* incX )
{
  DEBUG_S( "sasum" )
  return cblas_sasum( *N, X, *incX );
}

c_float FNAME(cdotu)( const int* N, const void* X, const int* incX, const void* Y, const int* incY )
{
  DEBUG_S( "cdotu" )
  c_float ans;
  cblas_cdotu_sub( *N, X, *incX, Y, *incY, &ans );
  return ans;
}

c_float FNAME(cdotc)( const int* N, const void* X, const int* incX, const void* Y, const int* incY )
{
  DEBUG_S( "cdotc" )
  c_float ans;
  cblas_cdotc_sub( *N, X, *incX, Y, *incY, &ans );
  return ans;
}

float FNAME(scnrm2)( const int* N, const void* X, const int* incX )
{
  DEBUG_S( "scnrm2" )
  return cblas_scnrm2( *N, X, *incX );
}

float FNAME(scasum)( const int* N, const void *X, const int* incX )
{
  DEBUG_S( "scasum" )
  return cblas_scasum( *N, X, *incX );
}

c_double FNAME(zdotu)( const int* N, const void* X, const int* incX, const void* Y, const int* incY )
{
  DEBUG_S( "zdotu" )
  c_double ans;
  cblas_zdotu_sub( *N, X, *incX, Y, *incY, &ans );
  return ans;
}

c_double FNAME(zdotc)( const int* N, const void* X, const int* incX, const void* Y, const int* incY )
{
  DEBUG_S( "zdotc" )
  c_double ans;
  cblas_zdotc_sub( *N, X, *incX, Y, *incY, &ans );
  return ans;
}

#undef FNAME
