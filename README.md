## vecLibFort: Full GFORTRAN compatibility for Apple's vecLib BLAS/LAPACK

### Introduction

vecLibFort is lightweight but flexible "shim" designed to correct a
small number of incompatibilities between Apple's supplied BLAS and LAPACK
libraries and FORTRAN code compiled with modern compilers. 

You *will* want this library if 

  * you are compiling your code directly from FORTRAN source; *and*
  * you use Apple's BLAS and/or LAPACK for your linear algebra; *and*
  * you use single-precision or complex arithmetic. 

You *will not* need this library if 

  * you use some other linear algebra package; *or*
  * you call BLAS and LAPACK only from C; *or*
  * you use an alternative BLAS/LAPACK package ([OpenBlas][],[MKL][]); *or*
  * you rely only on double-precision real arithmetic.

You *may* want this library if 

  * you are running a *pre-compiled* program, or linking to a *pre-compiled*
    library, that seems to exhibit bugs related to the incompatibilities
    described in the [next](#background) section. See the section
    [Preloaded (interposing) library](#preloaded) for more details.

<a name="background"></a>
### Background

[Apple's vecLib framework][vecLib] ships with both C and FORTRAN bindings for
BLAS and LAPACK, the de-facto standard libraries for dense numerical linear
algebra. Because there remains quite a bit of useful FORTRAN code out there
that in turn depend on BLAS and LAPACK, this is certainly a welcome provision
from Apple.

Unfortunately, those FORTRAN bindings follow an [F2C][]-style argument passing
convention. Later versions of [GNU Fortran][], on the other hand, default to a
[different, more C-friendly convention][gnufarg]. Most subroutines and 
functions work without modification; in particular, any code that relies solely 
on double-precision *real* arithmetic is fine. If that means you, feel free
to stop reading now. There are two specific types of incompatibilities, 
however, that prove fatal:

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

One potential solution is to force GNU Fortran to adopt the older, F2C-style
calling convention, using the ``-ff2c`` flag. However, there seem to be a few 
incompatibilities that remain on 64-bit platforms, perhaps due to differences
in integer sizes.  (Further information about these differences would be 
appreciated.) And it seems excessive to require entire projects to be compiled
with ``-ff2c``. GNU Fortran presumably has a performance-related reason for 
choosing a different calling convention.

Another option, then, is to create a lightweight "shim" that stands between
each problematic BLAS/LAPACK call and the Fortran code that calls it, 
performing any necessary translation between the calling formats. That
is exactly what this code is designed to do. This code can also be used
to correct existing programs *without recompilation* using OS X's "preload
library" facility.

### Using vecLibFort

This code can be used in one of three ways.

#### Direct inclusion

For new projects, feel free to include the source in the project itself, 
and link wtih ``-framework vecLib`` as usual. Name conflicts will be resolved 
in favor of vecLibFort, which will in turn load the replaced versions of
functions directly from vecLib to perform its computations.

#### Standard dynamic library

Package libraries like Homebrew, MacPorts, Fink, etc. can compile and install 
vecLibFort as a dynamic library. I haven't taken the trouble to build a 
``Makefile``, but this single command should work:

    clang -shared -O -o libvecLibFort.dylib vecLibFort.c \
        -framework vecLib -install_name <path>/libvecLibFort.dylib

where ``<path>`` is where you intend to install the library. Then, when compiling
a project containing FORTRAN code that requires BLAS and LAPACK, replace any 
instances of ``-framework vecLib`` with command ``-L<path> -lvecLibFort``.

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
  [GNU Octave](https://www.gnu.org/software/octave/). You can see the
  relevant source code from Octave [here][blaswrap]. vecLibFort differs from
  Octave in that it resolves the replacements lazily, eliminating the need for
  lookup tables and (hopefully) improving performance. It also implements the 
  full set of BLAS/LAPACK replacements, whereas Octave replaces only a subset.
* The interposing implementation is explained in a variety of places on the 
  Internet, including section 2.6.3.4 of Amit Singh's book "Mac OSX
  Internals." (http://osxbook.com). Point your favorite search engine to the
  term [``DYLD_INSERT_LIBRARIES``][Google] to find a wealth of material.
* In order to make the primary source file as compact as possible, this code
  employs a simple preprocessor library by Paul Fultz II called [Cloak][]. The
  [Boost Preprocessor Library][Boost] is perhaps the more well known example
  of this kind of work, but it is far more complex than needed in this case.

### License

License, schmicense. Do with this what you will. I would appreciate it if you
would give me credit, as I have attempted to do in the previous section. But
I'm not going to get bent out of shape about it. Large piles of cash are
welcome, as are simple emails of gratitude.

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

