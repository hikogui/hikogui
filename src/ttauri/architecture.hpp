// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <exception>
#include <cstddef>
#include <type_traits>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace tt::inline v1 {

#define TT_BT_DEBUG 'D'
#define TT_BT_RELEASE 'R'

#if defined(NDEBUG)
#define TT_BUILD_TYPE TT_BT_RELEASE
#else
#define TT_BUILD_TYPE TT_BT_DEBUG
#endif

enum class build_type {
    debug = TT_BT_DEBUG,
    release = TT_BT_RELEASE,

    current = TT_BUILD_TYPE
};

#define TT_OS_WINDOWS 'W'
#define TT_OS_MACOS 'A'
#define TT_OS_MOBILE 'M'
#define TT_OS_OTHER 'O'

#if defined(_WIN32)
#define TT_OPERATING_SYSTEM TT_OS_WINDOWS
#elif defined(TARGET_OS_MAC) && !defined(TARGET_OS_IPHONE)
#define TT_OPERATING_SYSTEM TT_OS_MACOS
#elif defined(TARGET_OS_IPHONE) || defined(__ANDROID__)
#define TT_OPERATING_SYSTEM TT_OS_MOBILE
#else
#define TT_OPERATING_SYSTEM TT_OS_OTHER
#endif

enum class operating_system {
    windows = TT_OS_WINDOWS,
    macos = TT_OS_MACOS,
    mobile = TT_OS_MOBILE,
    other = TT_OS_OTHER,

    current = TT_OPERATING_SYSTEM
};

#define TT_CC_MSVC 'm'
#define TT_CC_GCC 'g'
#define TT_CC_CLANG 'c'

#if defined(__clang__)
#define TT_COMPILER TT_CC_CLANG
#elif defined(_MSC_BUILD)
#define TT_COMPILER TT_CC_MSVC
#elif defined(__GNUC__)
#define TT_COMPILER TT_CC_GCC
#else
#error "Could not detect the compiler."
#endif

enum class compiler {
    msvc = TT_CC_MSVC,
    gcc = TT_CC_GCC,
    clang = TT_CC_CLANG,

    current = TT_COMPILER
};

#define TT_CPU_X64 'i'
#define TT_CPU_ARM 'a'
#define TT_CPU_UNKNOWN 'u'

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
#define TT_PROCESSOR TT_CPU_X64
#elif defined(__arm__) || defined(_M_ARM)
#define TT_PROCESSOR TT_CPU_ARM
#else
#define TT_PROCESSOR TT_CPU_UNKNOWN
#endif

enum class processor {
    x64 = TT_CPU_X64,
    arm = TT_CPU_ARM,
    unknown = TT_CPU_UNKNOWN,

    current = TT_PROCESSOR
};

#if TT_PROCESSOR == TT_CPU_X64
#if defined(__AVX512BW__) && defined(__AVX512CD__) && defined(__AVX512DQ__) && defined(__AVX512F__) && defined(__AVX512VL__)
#define TT_X86_64_V4 1
#define TT_X86_64_V3 1
#define TT_X86_64_V2_5 1
#define TT_X86_64_V2 1
#define TT_X86_64_V1 1
#define TT_HAS_SSE
#define TT_HAS_SSE2
#define TT_HAS_SSE3
#define TT_HAS_SSE4_1
#define TT_HAS_SSE4_2
#define TT_HAS_SSSE3
#define TT_HAS_AVX
#define TT_HAS_AVX2
#define TT_HAS_BMI1
#define TT_HAS_BMI2
#define TT_HAS_AVX512F
#define TT_HAS_AVX512BW
#define TT_HAS_AVX512CD
#define TT_HAS_AVX512DQ
#define TT_HAS_AVX512VL

#elif defined(__AVX2__)
#define TT_X86_64_V3 1
#define TT_X86_64_V2_5 1
#define TT_X86_64_V2 1
#define TT_X86_64_V1 1
#define TT_HAS_SSE
#define TT_HAS_SSE2
#define TT_HAS_SSE3
#define TT_HAS_SSE4_1
#define TT_HAS_SSE4_2
#define TT_HAS_SSSE3
#define TT_HAS_AVX
#define TT_HAS_AVX2
#define TT_HAS_BMI1
#define TT_HAS_BMI2

#elif defined(__AVX__)
#define TT_X86_64_V2_5 1
#define TT_X86_64_V2 1
#define TT_X86_64_V1 1
#define TT_HAS_SSE
#define TT_HAS_SSE2
#define TT_HAS_SSE3
#define TT_HAS_SSE4_1
#define TT_HAS_SSE4_2
#define TT_HAS_SSSE3
#define TT_HAS_AVX

// x86_64_v2 can not be selected in MSVC, but can be in gcc and clang.
#elif defined(__SSE4_2__) && defined(__SSSE3__)
#define TT_X86_64_V2 1
#define TT_X86_64_V1 1
#define TT_HAS_SSE
#define TT_HAS_SSE2
#define TT_HAS_SSE3
#define TT_HAS_SSE4_1
#define TT_HAS_SSE4_2
#define TT_HAS_SSSE3

#else
#define TT_X86_64_V1 1
#define TT_HAS_SSE
#define TT_HAS_SSE2
#endif
#endif

#if defined(TT_X86_64_V1)
constexpr bool x86_64_v1 = true;
#else
constexpr bool x86_64_v1 = false;
#endif

#if defined(TT_X86_64_V2)
constexpr bool x86_64_v2 = true;
#else
constexpr bool x86_64_v2 = false;
#endif

#if defined(TT_X86_64_V2_5)
constexpr bool x86_64_v2_5 = true;
#else
constexpr bool x86_64_v2_5 = false;
#endif

#if defined(TT_X86_64_V3)
constexpr bool x86_64_v3 = true;
#else
constexpr bool x86_64_v3 = false;
#endif

#if defined(TT_X86_64_V4)
constexpr bool x86_64_v4 = true;
#else
constexpr bool x86_64_v4 = false;
#endif

#define tt_stringify_(x) #x
#define tt_stringify(x) tt_stringify_(x)

#define tt_cat_(a, b) a ## b
#define tt_cat(a, b) tt_cat_(a, b)

#if TT_COMPILER == TT_CC_MSVC
#define tt_unreachable() __assume(0)
#define tt_assume(condition) __assume(condition)
#define tt_force_inline __forceinline
#define tt_no_inline __declspec(noinline)
#define tt_restrict __restrict
#define tt_warning_push() _Pragma("warning( push )")
#define tt_warning_pop() _Pragma("warning( pop )")
#define tt_msvc_pragma(a) _Pragma(a)
#define tt_msvc_suppress(code) _Pragma(tt_stringify(warning(disable:code)))
#define tt_clang_suppress(a)

/** Attribute to export a function, class, variable in the shared library or dll.
 */
#define tt_export __declspec(dllexport)

#elif TT_COMPILER == TT_CC_CLANG
#define tt_unreachable() __builtin_unreachable()
#define tt_assume(condition) __builtin_assume(static_cast<bool>(condition))
#define tt_force_inline inline __attribute__((always_inline))
#define tt_no_inline __attribute__((noinline))
#define tt_restrict __restrict__
#define tt_warning_push() _Pragma("warning(push)")
#define tt_warning_pop() _Pragma("warning(push)")
#define tt_msvc_pragma(a)
#define tt_clang_suppress(a) _Pragma(tt_stringify(clang diagnostic ignored a))
#define tt_export

#elif TT_COMPILER == TT_CC_GCC
#define tt_unreachable() __builtin_unreachable()
#define tt_assume(condition) \
    do { \
        if (!(condition)) \
            tt_unreachable(); \
    } while (false)
#define tt_force_inline inline __attribute__((always_inline))
#define tt_no_inline __attribute__((noinline))
#define tt_restrict __restrict__
#define tt_warning_push() _Pragma("warning(push)")
#define tt_warning_pop() _Pragma("warning(pop)")
#define tt_msvc_pragma(a)
#define tt_clang_suppress(a)
#define msvc_pragma(a)

#else
#define tt_unreachable() std::terminate()
#define tt_assume(condition) static_assert(sizeof(condition) == 1)
#define tt_force_inline inline
#define tt_no_inline
#define tt_restrict
#define tt_warning_push()
#define tt_warning_pop()
#define tt_msvc_pragma(a)
#define tt_clang_suppress(a)
#define msvc_pragma(a)

#endif

#if TT_PROCESSOR == TT_CPU_X64
/** Minimum offset between two objects to avoid false sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */
constexpr size_t hardware_destructive_interference_size = 128;

/** Maximum size of contiguous memory to promote true sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */
constexpr size_t hardware_constructive_interference_size = 64;
#elif TT_PROCESSOR == TT_CPU_ARM
/** Minimum offset between two objects to avoid false sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */
constexpr size_t hardware_destructive_interference_size = 64;

/** Maximum size of contiguous memory to promote true sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */
constexpr size_t hardware_constructive_interference_size = 64;

#elif TT_PROCESSOR == TT_CPU_UNKNOWN
constexpr size_t hardware_destructive_interference_size = 128;
constexpr size_t hardware_constructive_interference_size = 64;
#else
#error "Missing implementation of hardware_destructive_interference_size and hardware_constructive_interference_size"
#endif

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
using os_handle = void *;
using file_handle = os_handle;

#elif TT_OPERATING_SYSTEM == TT_OS_MACOS
using os_handle = int;
using file_handle = int;

#elif TT_OPERATING_SYSTEM == TT_OS_LINUX
using os_handle = int;
using file_handle = int;

#else
#error "file_handle Not implemented."
#endif

} // namespace tt::inline v1
