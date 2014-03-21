## A GNU Fortran interface to Apple's vecLib BLAS/LAPACK

### Introduction

vecLibFort is lightweight but flexible "shim" designed to rectify
the incompatibilities between the vecLib BLAS and LAPACK libraries
shipped with Mac OS X and FORTRAN code compiled with modern compilers
such as [GNU Fortran][].

You *will* want this code if you are...

  * compiling your code directly from FORTRAN source; *and*
  * using Apple's BLAS and/or LAPACK for your linear algebra; *and*
  * using single-precision or complex arithmetic. 

You *will not* need this code if you are...

  * using some other linear algebra package; *or*
  * calling BLAS and LAPACK only from C; *or*
  * using an alternative BLAS/LAPACK package ([OpenBlas][],[MKL][]); *or*
  * using only on double-precision real arithmetic.

You *may* want this code if you are...

  * running a *pre-compiled* program, or linking to a *pre-compiled*
    library, that seems to exhibit bugs described in the [next](#background)
    section. See the section [Preloaded (interposing) library](#preloaded) 
    for more details on how you may be able to fix these programs without
    recompilation.

<a name="background"></a>
### Background

[Apple's vecLib framework][vecLib] provides both C and FORTRAN bindings for
BLAS and LAPACK, the de-facto standard libraries for dense numerical linear
algebra. Because there remains quite a bit of useful FORTRAN code out there
that in turn depend on BLAS and LAPACK, this is certainly a welcome provision
from Apple.

Unfortunately, those FORTRAN bindings follow an [F2C][]-style return value
convention, while [GNU Fortran][] uses a [different convention][gnufarg]. Most
subroutines and functions work without modification; in particular, if you 
rely solely on double-precision *real* arithmetic, you are fine. For single
precision or complex arithmetic, there are two fatal incompatibilities:

* Functions whose FORTRAN specifications call for returning single-precision
  real values, such as ``sdot_`` and ``snrm2_``, actually return 
  *double-precision* results in the Apple/F2C calling convention. GNU Fortran, 
  on the other hand, expects to receive the single-precision result.
* Functions designed to return complex values, whether single-precision or
  double-precision, are converted to subroutines in the Apple/F2C convention, 
  with a pointer to the return value serving as the first argument. (Note that
  this differs from the CBLAS convention of passing a pointer to the
  return value as the *final* argument.) GNU Fortran, on the other hand,
  expects these values to be returned as a C-style return value.

For programs that use single-precision or complex arithmetic, then, these
incompatibilities *must* be addressed or incorrect results and crashes can
occur. In some projects, these errors go uncorrected, because the use cases
that exercise them are uncommon.

One solution is to force GNU Fortran to adopt the older, F2C-style return
value convention, using the ``-ff2c`` flag. If that solution is sufficient
for you, then I encourage you to adopt it. Unfortunately, this may not be
possible if there is other code or other libraries that you rely upon that
assume the default GNU Fotran convention. And don't forget to rewrite your
C code according to the F2C return value conventions.

The approach taken by vecLibFort is to provide a thin translation layer
between the F2C and GFortran worlds, for the few functions where there is a
difference. For BLAS, this is simply a matter of wrapping Apple's CBLAS
calls in a FORTRAN-friendly wrapper. For LAPACK, a bit of dlopen/dlsym
trickery is required to avoid name conflicts.

Still another option is to use a different BLAS and LAPACK library, such
as [MKL][] or [OpenBlas][]. I am sure there are good arguments to be made
for all three options.

### Using vecLibFort

This code can be used in one of three ways.

#### Direct inclusion

For new projects, feel free to add ``vecLibFort.c``, ``static.h``, and
``cloak.h`` to your project, and link with ``-framework vecLib`` as usual.
Name conflicts will be resolved favor of the statically linked vecLibFor, 
which will in turn load the replaced versions of functions directly from 
vecLib to perform its computations.

#### Standard dynamic library

Package libraries like Homebrew, MacPorts, Fink, etc. can compile and install 
vecLibFort as a dynamic library. I haven't taken the trouble to build a 
``Makefile``, but this single command will work:

    clang -shared -O -o libvecLibFort.dylib vecLibFort.c \
        -Wl,-reexport_framework -Wl,vecLib -install_name <path>/libvecLibFort.dylib

where ``<path>`` is where you intend to install the library. The use of
``-reexport_framework`` instead of ``-framework`` allows vecLibFort to serve
as a complete replacement for vecLib by exporting its full symbol list.
Therefore, to use this library, you must simply replace any instances of
``-framework vecLib`` in your build steps with ``-lvecLibFort``, adding an
``-L<path>`` if you installed it in a non-standard location.

(*Technical note*: you could also just use ``-framework`` instead of
``-reexport_framework``, and then include both link commands when compiling
the executable. I chose this approach because I had some issues with link
order when linking a project with multiple dependencies that relied separately
on BLAS and LAPACK. For all I know, I was making a silly mistake, but the
``reexport_framework`` approach smoothed everything out.)

This approach should operate identically to direct inclusion; vecLibFort will
override and replace the offending commands by nature of its link order.

<a name="preloaded"></a>
#### Preloaded (interposing) library

Suppose you have a program that is already compiled, but which apparently 
exhibits the errors discussed herein. Or perhaps you are using a precompiled
third-party library that has not implemented measures like these itself; but
because it has already been linked to vecLib, the bugs are baked in. (If you 
can alter the linking information of a dynamic library, I bow to your skill.)

In these cases, there is a *preload* feature of Mac OSX's ``dyld`` system that
can come in quite handy. The OS makes it possible to specify a library to be
*preloaded* before the application, with a list of instructions to replace
functions with alternate versions. To take advantage of this, we need to build
a new shared library with the -DINTERPOSE flag set:

    clang -shared -O -DVECLIBFORT_INTERPOSE -o libvecLibFortI.dylib vecLibFort.c \
        -framework vecLib -install_name <path>/libvecLibFortI.dylib

(There is no point in using ``-reexport_framework`` here.)
Armed with this library, you can invoke the preloading system using the
[``DYLD_INSERT_LIBRARIES`` environment variable][DYLD]. For instance,

    DYLD_INSERT_LIBRARIES=<path>/libvecLibFortI.dylib program

will run the program ``program`` but with the BLAS and LAPACK calls corrected.

Of course, this may not work---it may be that the bugs you are seeing are not
in fact caused by the specific issues addressed by vecLibFort. Or I might not
have implemented something correctly. (Bug reports are welcome.) And you
should *not* use this if the program or library *already* uses the F2C 
calling conventions correctly; you *will* break it.

### Inspirations

This code in ``vecLibFort.c`` is new, but the concepts that undergird it are 
most certainly not. The inspirations include:

* The [dotwrp project][dotwrp] project provides a simple FORTRAN-based wrapper
  for the 5 most common problematic BLAS functions. Thanks to vecLib's CBLAS 
  interface, the substitutions can be made statically. We have extended this 
  approach to cover all of the relevant BLAS calls, and implemented it in C.
* The dynamic substitution approach is heavily inspired by the method used by
  [GNU Octave](https://www.gnu.org/software/octave/), as contributed by Jarno
  Rajahaime. You can see the [here][blaswrap]. vecLibFort differs from Octave
  in that it resolves the replacements lazily, eliminating the need for
  lookup tables and (hopefully) improving performance. It also implements the 
  full set of BLAS/LAPACK replacements, whereas Octave replaces only a subset.
* The interposing implementation is explained in a variety of places on the 
  Internet, including section 2.6.3.4 of Amit Singh's book "Mac OSX
  Internals." (http://osxbook.com). Point your favorite search engine to the
  term [``DYLD_INSERT_LIBRARIES``][Google] to find a wealth of material.
* In order to make the primary source file as compact as possible, this code
  employs a simple preprocessor library by Paul Fultz II called [Cloak][]. The
  [Boost Preprocessor Library][Boost] is perhaps a more well known example
  of this kind of work, but it is far more complex than needed in this case.

### License

##### English

License, schmicense. Do with this what you will. I would appreciate it if you
would give me credit, as I have attempted to do in the previous section. But
I'm not going to get bent out of shape about it. Large piles of cash are
welcome, as are simple emails of gratitude, or ([unlicensed][]) pull requests!

##### Legalese

> This is free and unencumbered software released into the public domain.
> 
> Anyone is free to copy, modify, publish, use, compile, sell, or
> distribute this software, either in source code form or as a compiled
> binary, for any purpose, commercial or non-commercial, and by any
> means.
> 
> In jurisdictions that recognize copyright laws, the author or authors
> of this software dedicate any and all copyright interest in the
> software to the public domain. We make this dedication for the benefit
> of the public at large and to the detriment of our heirs and
> successors. We intend this dedication to be an overt act of
> relinquishment in perpetuity of all present and future rights to this
> software under copyright law.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
> EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
> MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
> IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
> OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
> ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
> OTHER DEALINGS IN THE SOFTWARE.
> 
> For more information, please refer to <http://unlicense.org/>

[vecLib]:https://developer.apple.com/library/mac/documentation/Performance/Conceptual/vecLib/Reference/reference.html
[GNU Fortran]:http://gcc.gnu.org/fortran/
[gnufarg]:http://gcc.gnu.org/onlinedocs/gfortran/Argument-passing-conventions.html
[F2C]:http://www.netlib.org/f2c/
[DYLD]:https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man1/dyld.1.html
[dotwrp]:https://github.com/tenomoto/dotwrp
[GNU Octave]:https://www.gnu.org/software/octave/
[blaswrap]:http://hg.savannah.gnu.org/hgweb/octave/file/tip/liboctave/cruft/misc/blaswrap.c
[Google]:https://www.google.com/search?q=DYLD_INSERT_LIBRARIES
[Cloak]:https://github.com/pfultz2/Cloak/blob/master/cloak.h
[Boost]:http://www.boost.org/doc/libs/1_55_0/libs/preprocessor/doc/index.html 
[OpenBLAS]:http://www.openblas.net/
[MKL]:http://software.intel.com/en-us/intel-mkl
[unlicensed]:http://unlicense.org
[blasbug]:http://www.macresearch.org/lapackblas-fortran-106

