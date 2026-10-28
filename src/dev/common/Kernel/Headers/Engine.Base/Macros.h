#pragma once

#define KiB    << 10
#define KB     * 1000
#define MiB    << 20
#define MB     * 1000 * 1000
#define GiB    << 30
#define GB     * 1000 * 1000 * 1000

#define sizeof_array(a) (sizeof((a)) / sizeof((a)[0]))

#ifndef CompileTimeAssertion
#define CompileTimeAssertion(x) static_assert(x)
#endif

#ifndef CompileTimeAssertSize
#define CompileTimeAssertSize(t, s) CompileTimeAssertion(sizeof(t) == s)
#endif

#ifndef RoundUp
#define RoundUp(N, A) (((N) + (A) - 1) & ~((A) - 1))
#endif

// EXTERN_C
#ifdef EXTERN_C
#undef EXTERN_C
#endif
#define EXTERN_C extern "C"

// BIT
#define _BIT(ix) (1 << (ix - 1))

// MASK
#define _MASK(bits) ((1 << (bits)) - 1)

// interface
#ifdef interface
#undef interface
#endif
#define interface struct

// NIL
#ifdef NIL
#undef NIL
#endif
#define NIL 0

#ifndef YES
#define YES 1
#endif

#ifndef NO
#define NO 0
#endif

#ifndef pure
#define pure = 0
#endif

#define __maybe(b, v) ((b) ? (v) : NIL)

// nodefine
#ifdef nodefine
#undef nodefine
#endif
#define nodefine virtual

#ifdef offsetof
#undef offsetof
#endif
#define offsetof(t, m) ((size_t)&(((t*)0)->m))

#define make_cc(a, b, c, d) ((unsigned(a) << 24) | (unsigned(b) << 16) | (unsigned(c) << 8) | unsigned(d))

#define RUN_ONCE(ret) static bool RAN_ONCE = false; do { if(RAN_ONCE) { return ret; } } while(false)

#if defined(__x86_64__) || defined(_WIN64) || defined(_M_AMD64) || defined(_AMD64__)
    #define SvcArchitectureX86_64 1
#elif defined(_M_ARM)
    #define SvcArchitectureARM 1
#elif defined(_M_ARM64)
    #define SvcArchitectureARM64 1
#else
    #define SvcArchitectureX86 1
#endif

#if defined(__GNUC__) && !defined(_MSC_VER)
    #define SvcCompilerGNU 1
#elif defined(__clang__) && !defined(_MSC_VER)
    #define SvcCompilerCLANG 1
    #define SvcCompilerVERSION (__clang_major__ * 100 + __clang_minor__)
#else
    #define SvcCompilerMSVC 1
    #define SvcCompilerVERSION _MSC_VER

    #ifdef __clang__
        #define SvcCompilerCLANG 1
    #endif

    #if (_MSC_VER <= 1200) // If VC6.x and earlier...
        #if (_MSC_VER < 1200)
            #define SvcCompilerMSVCOLD 1
        #else
            #define SvcCompilerMSVC6 1
        #endif

    #elif (_MSC_VER <= 1300) // If VC7.0 and earlier...
        #define SvcCompilerMSVC7 1

    #elif (_MSC_VER < 1400) // VS2003       _MSC_VER of 1300 means VC7 (VS2003)
        #define SvcCompilerMSVC7_2003 1
        #define SvcCompilerMSVC7_1    1

    #elif (_MSC_VER < 1500) // VS2005       _MSC_VER of 1400 means VC8 (VS2005)
        #define SvcCompilerMSVC8_2005 1
        #define SvcCompilerMSVC8_0    1
    
    #elif (_MSC_VER < 1600) // VS2008.      _MSC_VER of 1500 means VC9 (VS2008)
        #define SvcCompilerMSVC9_2008 1
        #define SvcCompilerMSVC9_0    1

    #elif (_MSC_VER < 1700) // VS2010       _MSC_VER of 1600 means VC10 (VS2010)
        #define SvcCompilerMSVC2010 1
        #define SvcCompilerMSVC10_0  1

    #elif (_MSC_VER < 1800) // VS2012       _MSC_VER of 1700 means VS2011/VS2012
        #define SvcCompilerMSVC2011 1   // Microsoft changed the name to VS2012 before shipping, despite referring to it as VS2011 up to just a few weeks before shipping.
        #define SvcCompilerMSVC11_0  1
        #define SvcCompilerMSVC2012 1
        #define SvcCompilerMSVC12_0  1

    #elif (_MSC_VER < 1900) // VS2013       _MSC_VER of 1800 means VS2013
        #define SvcCompilerMSVC2013 1
        #define SvcCompilerMSVC13_0  1

    #elif (_MSC_VER < 1910) // VS2015       _MSC_VER of 1900 means VS2015
        #define SvcCompilerMSVC2015 1
        #define SvcCompilerMSVC14_0  1

    #elif (_MSC_VER < 1920) // VS2017       _MSC_VER of 1910 means VS2017
        #define SvcCompilerMSVC2017 1
        #define SvcCompilerMSVC14_1  1
    #elif (_MSC_VER < 1930)
        #define SvcCompilerMSVC2019 1
        #define SvcCompilerMSVC14_2  1
    #else
        #define SvcCompilerMSVC2022 1
        #define SvcCompilerMSVC14_3  1
    #endif
#endif

#if defined(__ICL) || defined(__ICC)
    #define SvcCompilerINTEL 1

    #if defined(_MSC_VER)
        #define SvcCompilerMSVC 1
    #endif
    
    #if defined(__GNUC__)
        #define SvcCompilerGNU 1
    #endif
#endif

#ifndef expect_true
    #if defined(SvcCompilerCLANG) || defined(SvcCompilerGNU)
        #define expect_true(x) __builtin_expect(!!(x), 1)
    #else
        #define expect_true(x) (x)
    #endif
#endif

#ifndef expect_false
#if defined(SvcCompilerCLANG) || defined(SvcCompilerGNU)
        #define expect_false(x) __builtin_expect(!!(x), 0)
    #else
        #define expect_false(x) (x)
    #endif
#endif

#if defined(SvcCompilerGNU)
    #define SvcAssemblyStyleATT 1
#elif defined(SvcCompilerMSVC) || defined(SvcCompilerINTEL)
    #define SvcAssemblyStyleINTEL 1
#endif

#if defined(SvcArchitectureX86_64) || defined(SvcArchitectureAMD64)
    #define __wordsize 64
#else
    #define __wordsize 32
#endif

#ifdef SvcCompilerMSVC
    #if defined(ENGINE_BUILD_DLL)
        #define LIB_LINKAGE __declspec(dllexport)
    #elif !defined(ENGINE_BUILD_STATIC)
        #define LIB_LINKAGE __declspec(dllimport)
    #else
        #define LIB_LINKAGE
    #endif
#else
    #define LIB_LINKAGE __attribute__((visibility("default")))
#endif

#if __wordsize == 64
    #define SvcArchitecture 64
#else
    #define SvcArchitecture 32
#endif

#if defined(SvcCompilerMSVC)
    #if defined(SvcCompilerCLANG)
        #define SvcCompilerShortName "clang"
        #define SvcCompilerString "Clang"
    #else
        #define SvcCompilerShortName "msvc"
        #define SvcCompilerString "Microsoft Visual C++"
    #endif
    #if defined(SvcCompilerMSVC6)
        #define __msvc "msvc6"
    #elif defined(SvcCompilerMSVC7)
        #define __msvc "msvc7_2003"
    #elif defined(SvcCompilerMSVC8_0)
        #define __msvc "msvc8_2005"
    #elif defined(SvcCompilerMSVC9_0)
        #define __msvc "msvc9_2008"
    #elif defined(SvcCompilerMSVC10_0)
        #define __msvc "msvc10_2010"
    #elif defined(SvcCompilerMSVC11_0)
        #define __msvc "msvc11_11"
    #elif defined(SvcCompilerMSVC12_0)
        #define __msvc "msvc12_2012"
    #elif defined(SvcCompilerMSVC13_0)
        #define __msvc "msvc13_2013"
    #elif defined(SvcCompilerMSVC14_0)
        #define __msvc "msvc14_0_2015"
    #elif defined(SvcCompilerMSVC14_1)
        #define __msvc "msvc14_1_2017"
    #elif defined(SvcCompilerMSVC14_2)
        #define __msvc "msvc14_2_2019"
    #elif defined(SvcCompilerMSVC14_3)
        #define __msvc "msvc14_3_2022"
    #endif
#elif defined(SvcCompilerGNU)
    #define SvcCompilerShortName "gcc"
    #define SvcCompilerString "GNU C Compiler"
    #define __msvc ""
    #elif defined(SvcCompilerCLANG)
    #define SvcCompilerShortName "clang"
    #define SvcComilerString "Clang"
    #define __msvc ""
#endif

#if SvcArchitecture==64
    #if defined(SvcArchitectureARM64)
        #define __arch "arm64"
    #else
        #define __arch "x86_64"
    #endif
#else
    #if defined(SvcArchitectureARM)
        #define __arch "arm"
    #else
        #define __arch "x86"
    #endif
#endif

// Microsoft Visual C++ (msvc14_3_2022_x86_64)
#define SvcCompilerLongName SvcCompilerString " (" __msvc "_" __arch ")"