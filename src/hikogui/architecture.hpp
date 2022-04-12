// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file architecture.hpp
*
* Functions and macros for handling architectural difference between compilers, CPUs and operating systems.
*/

#pragma once

#include <exception>
#include <cstddef>
#include <type_traits>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace hi::inline v1 {

#define HI_BT_DEBUG 'D'
#define HI_BT_RELEASE 'R'

#if defined(NDEBUG)
#define HI_BUILD_TYPE HI_BT_RELEASE
#else
#define HI_BUILD_TYPE HI_BT_DEBUG
#endif

enum class build_type {
    debug = HI_BT_DEBUG,
    release = HI_BT_RELEASE,

    current = HI_BUILD_TYPE
};

#define HI_OS_WINDOWS 'W'
#define HI_OS_MACOS 'A'
#define HI_OS_MOBILE 'M'
#define HI_OS_OTHER 'O'

#if defined(_WIN32)
#define HI_OPERATING_SYSTEM HI_OS_WINDOWS
#elif defined(TARGET_OS_MAC) && !defined(TARGET_OS_IPHONE)
#define HI_OPERATING_SYSTEM HI_OS_MACOS
#elif defined(TARGET_OS_IPHONE) || defined(__ANDROID__)
#define HI_OPERATING_SYSTEM HI_OS_MOBILE
#else
#define HI_OPERATING_SYSTEM HI_OS_OTHER
#endif

enum class operating_system {
    windows = HI_OS_WINDOWS,
    macos = HI_OS_MACOS,
    mobile = HI_OS_MOBILE,
    other = HI_OS_OTHER,

    current = HI_OPERATING_SYSTEM
};

#define HI_CC_MSVC 'm'
#define HI_CC_GCC 'g'
#define HI_CC_CLANG 'c'

#if defined(__clang__)
#define HI_COMPILER HI_CC_CLANG
#elif defined(_MSC_BUILD)
#define HI_COMPILER HI_CC_MSVC
#elif defined(__GNUC__)
#define HI_COMPILER HI_CC_GCC
#else
#error "Could not detect the compiler."
#endif

enum class compiler {
    msvc = HI_CC_MSVC,
    gcc = HI_CC_GCC,
    clang = HI_CC_CLANG,

    current = HI_COMPILER
};

#define HI_CPU_X64 'i'
#define HI_CPU_ARM 'a'
#define HI_CPU_UNKNOWN 'u'

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
#define HI_PROCESSOR HI_CPU_X64
#elif defined(__arm__) || defined(_M_ARM)
#define HI_PROCESSOR HI_CPU_ARM
#else
#define HI_PROCESSOR HI_CPU_UNKNOWN
#endif

enum class processor {
    x64 = HI_CPU_X64,
    arm = HI_CPU_ARM,
    unknown = HI_CPU_UNKNOWN,

    current = HI_PROCESSOR
};

#if HI_PROCESSOR == HI_CPU_X64
#if defined(__AVX512BW__) && defined(__AVX512CD__) && defined(__AVX512DQ__) && defined(__AVX512F__) && defined(__AVX512VL__)
#define HI_X86_64_V4 1
#define HI_X86_64_V3 1
#define HI_X86_64_V2_5 1
#define HI_X86_64_V2 1
#define HI_X86_64_V1 1
#define HI_HAS_SSE
#define HI_HAS_SSE2
#define HI_HAS_SSE3
#define HI_HAS_SSE4_1
#define HI_HAS_SSE4_2
#define HI_HAS_SSSE3
#define HI_HAS_AVX
#define HI_HAS_AVX2
#define HI_HAS_BMI1
#define HI_HAS_BMI2
#define HI_HAS_AVX512F
#define HI_HAS_AVX512BW
#define HI_HAS_AVX512CD
#define HI_HAS_AVX512DQ
#define HI_HAS_AVX512VL

#elif defined(__AVX2__)
#define HI_X86_64_V3 1
#define HI_X86_64_V2_5 1
#define HI_X86_64_V2 1
#define HI_X86_64_V1 1
#define HI_HAS_SSE
#define HI_HAS_SSE2
#define HI_HAS_SSE3
#define HI_HAS_SSE4_1
#define HI_HAS_SSE4_2
#define HI_HAS_SSSE3
#define HI_HAS_AVX
#define HI_HAS_AVX2
#define HI_HAS_BMI1
#define HI_HAS_BMI2

#elif defined(__AVX__)
#define HI_X86_64_V2_5 1
#define HI_X86_64_V2 1
#define HI_X86_64_V1 1
#define HI_HAS_SSE
#define HI_HAS_SSE2
#define HI_HAS_SSE3
#define HI_HAS_SSE4_1
#define HI_HAS_SSE4_2
#define HI_HAS_SSSE3
#define HI_HAS_AVX

// x86_64_v2 can not be selected in MSVC, but can be in gcc and clang.
#elif defined(__SSE4_2__) && defined(__SSSE3__)
#define HI_X86_64_V2 1
#define HI_X86_64_V1 1
#define HI_HAS_SSE
#define HI_HAS_SSE2
#define HI_HAS_SSE3
#define HI_HAS_SSE4_1
#define HI_HAS_SSE4_2
#define HI_HAS_SSSE3

#else
#define HI_X86_64_V1 1
#define HI_HAS_SSE
#define HI_HAS_SSE2
#endif
#endif

#if defined(HI_X86_64_V1)
constexpr bool x86_64_v1 = true;
#else
constexpr bool x86_64_v1 = false;
#endif

#if defined(HI_X86_64_V2)
constexpr bool x86_64_v2 = true;
#else
constexpr bool x86_64_v2 = false;
#endif

#if defined(HI_X86_64_V2_5)
constexpr bool x86_64_v2_5 = true;
#else
constexpr bool x86_64_v2_5 = false;
#endif

#if defined(HI_X86_64_V3)
constexpr bool x86_64_v3 = true;
#else
constexpr bool x86_64_v3 = false;
#endif

#if defined(HI_X86_64_V4)
constexpr bool x86_64_v4 = true;
#else
constexpr bool x86_64_v4 = false;
#endif

#define hi_stringify_(x) #x
#define hi_stringify(x) hi_stringify_(x)

#define hi_cat_(a, b) a ## b
#define hi_cat(a, b) hi_cat_(a, b)

#if HI_COMPILER == HI_CC_MSVC

/** Marker to tell the compiler that this line will never be executed.
 * 
 * This marker allows the compiler to do certain optimization.
 */
#define hi_unreachable() __assume(0)

/** Mark an expression as true.
 *
 * The expression inside hi_assume() can be used by the compiler
 * to optimize the code (before and after) based on the fact that
 * the expression is true.
 */
#define hi_assume(condition) __assume(condition)

/** 
 */
#define hi_force_inline __forceinline
#define hi_no_inline __declspec(noinline)
#define hi_restrict __restrict
#define hi_warning_push() _Pragma("warning( push )")
#define hi_warning_pop() _Pragma("warning( pop )")
#define hi_msvc_pragma(a) _Pragma(a)
#define hi_msvc_suppress(code) _Pragma(hi_stringify(warning(disable:code)))
#define hi_clang_suppress(a)

/** Attribute to export a function, class, variable in the shared library or dll.
 */
#define hi_export __declspec(dllexport)

#elif HI_COMPILER == HI_CC_CLANG
#define hi_unreachable() __builtin_unreachable()
#define hi_assume(condition) __builtin_assume(static_cast<bool>(condition))
#define hi_force_inline inline __attribute__((always_inline))
#define hi_no_inline __attribute__((noinline))
#define hi_restrict __restrict__
#define hi_warning_push() _Pragma("warning(push)")
#define hi_warning_pop() _Pragma("warning(push)")
#define hi_msvc_pragma(a)
#define hi_clang_suppress(a) _Pragma(hi_stringify(clang diagnostic ignored a))
#define hi_export

#elif HI_COMPILER == HI_CC_GCC
#define hi_unreachable() __builtin_unreachable()
#define hi_assume(condition) \
    do { \
        if (!(condition)) \
            hi_unreachable(); \
    } while (false)
#define hi_force_inline inline __attribute__((always_inline))
#define hi_no_inline __attribute__((noinline))
#define hi_restrict __restrict__
#define hi_warning_push() _Pragma("warning(push)")
#define hi_warning_pop() _Pragma("warning(pop)")
#define hi_msvc_pragma(a)
#define hi_clang_suppress(a)
#define msvc_pragma(a)

#else
#define hi_unreachable() std::terminate()
#define hi_assume(condition) static_assert(sizeof(condition) == 1)
#define hi_force_inline inline
#define hi_no_inline
#define hi_restrict
#define hi_warning_push()
#define hi_warning_pop()
#define hi_msvc_pragma(a)
#define hi_clang_suppress(a)
#define msvc_pragma(a)

#endif

#if HI_PROCESSOR == HI_CPU_X64
/** Minimum offset between two objects to avoid false sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */
constexpr std::size_t hardware_destructive_interference_size = 128;

/** Maximum size of contiguous memory to promote true sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */
constexpr std::size_t hardware_constructive_interference_size = 64;
#elif HI_PROCESSOR == HI_CPU_ARM
/** Minimum offset between two objects to avoid false sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */
constexpr std::size_t hardware_destructive_interference_size = 64;

/** Maximum size of contiguous memory to promote true sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */
constexpr std::size_t hardware_constructive_interference_size = 64;

#elif HI_PROCESSOR == HI_CPU_UNKNOWN
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;
#else
#error "Missing implementation of hardware_destructive_interference_size and hardware_constructive_interference_size"
#endif

#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
using os_handle = void *;
using file_handle = os_handle;

#elif HI_OPERATING_SYSTEM == HI_OS_MACOS
using os_handle = int;
using file_handle = int;

#elif HI_OPERATING_SYSTEM == HI_OS_LINUX
using os_handle = int;
using file_handle = int;

#else
#error "file_handle Not implemented."
#endif

} // namespace hi::inline v1
