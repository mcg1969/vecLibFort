/*
 * Modeled from Apple's vecLib-760.10 instance of vecLib.h:
 * /Library/Developer/CommandLineTools/SDKs/MacOSX11.3.sdk/System/Library/Frameworks/Accelerate.framework/Frameworks/vecLib.framework/Headers/vecLib.h
 */

#ifndef __VECLIB__
#define __VECLIB__

#ifndef __VECLIBTYPES__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/vecLibTypes.h>
#endif

#ifndef __VBASICOPS__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/vBasicOps.h>
#endif

#ifndef __VBIGNUM__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/vBigNum.h>
#endif

#ifndef __VECTOROPS__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/vectorOps.h>
#endif

#ifndef __VFP__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/vfp.h>
#endif

#ifndef __VDSP__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/vDSP.h>
#endif

#if defined __ppc__ || defined __i386__
#ifndef __VDSP_TRANSLATE__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/vDSP_translate.h>
#endif
#endif

#ifndef CBLAS_H	
#include <Accelerate/../Frameworks/vecLib.framework/Headers/cblas.h>
#endif

#ifndef __CLAPACK_H
#include <Accelerate/../Frameworks/vecLib.framework/Headers/clapack.h>
#endif

#ifndef __LINEAR_ALGEBRA_PUBLIC_HEADER__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/LinearAlgebra/LinearAlgebra.h>
#endif

#ifndef __SPARSE_HEADER__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/Sparse/Sparse.h>
#include <Accelerate/../Frameworks/vecLib.framework/Headers/Sparse/Solve.h>
#endif

#ifndef __QUADRATURE_PUBLIC_HEADER__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/Quadrature/Quadrature.h>
#endif // __QUADRATURE_PUBLIC_HEADER__

#ifndef __BNNS_HEADER__
#include <Accelerate/../Frameworks/vecLib.framework/Headers/BNNS/bnns.h>
#endif // __BNNS_HEADER__

#ifndef __VFORCE_H
#include <Accelerate/../Frameworks/vecLib.framework/Headers/vForce.h>
#endif

#endif /* __VECLIB__ */
