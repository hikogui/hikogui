// Copyright Take Vos 2019-2022.
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

#define HI_CPU_X86 'i'
#define HI_CPU_X64 'I'
#define HI_CPU_ARM 'a'
#define HI_CPU_ARM64 'A'
#define HI_CPU_UNKNOWN '-'

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64) || defined(_M_X64)
#define HI_PROCESSOR HI_CPU_X64
#elif defined(__aarch64__) || defined(_M_ARM64)
#define HI_PROCESSOR HI_CPU_ARM64
#elif defined(__i386__) || defined(_M_IX86)
#define HI_PROCESSOR HI_CPU_X86
#elif defined(__arm__) || defined(__arm) || defeind(_ARM) || defined(_M_ARM)
#define HI_PROCESSOR HI_CPU_ARM
#else
#define HI_PROCESSOR HI_CPU_UNKNOWN
#endif

enum class processor {
    x86 = HI_CPU_X86,
    x64 = HI_CPU_X64,
    arm = HI_CPU_ARM,
    arm64 = HI_CPU_ARM64,
    unknown = HI_CPU_UNKNOWN,

    current = HI_PROCESSOR
};

#if HI_PROCESSOR == HI_CPU_X86
using intreg_t = int32_t;
using uintreg_t = uint32_t;
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;
#elif HI_PROCESSOR == HI_CPU_X64
using intreg_t = int64_t;
using uintreg_t = uint64_t;
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;
#elif HI_PROCESSOR == HI_CPU_ARM
using intreg_t = int32_t;
using uintreg_t = uint32_t;
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;
#elif HI_PROCESSOR == HI_CPU_ARM64
using intreg_t = int64_t;
using uintreg_t = uint64_t;
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;
#else
#error "missing implementation for CPU specific register and cache-line sizes"
#endif

#if defined(__AVX512BW__) && defined(__AVX512CD__) && defined(__AVX512DQ__) && defined(__AVX512F__) && defined(__AVX512VL__)
#define HI_X86_64_V4 1
#define HI_X86_64_V3 1
#define HI_X86_64_V2_5 1
#define HI_X86_64_V2 1
#define HI_X86_64_V1 1
#define HI_HAS_SSE 1
#define HI_HAS_SSE2 1
#define HI_HAS_SSE3 1
#define HI_HAS_SSE4_1 1
#define HI_HAS_SSE4_2 1
#define HI_HAS_SSSE3 1
#define HI_HAS_AVX 1
#define HI_HAS_AVX2 1
#define HI_HAS_BMI1 1
#define HI_HAS_BMI2 1
#define HI_HAS_AVX512F 1
#define HI_HAS_AVX512BW 1
#define HI_HAS_AVX512CD 1
#define HI_HAS_AVX512DQ 1
#define HI_HAS_AVX512VL 1

#elif defined(__AVX2__)
#define HI_X86_64_V3 1
#define HI_X86_64_V2_5 1
#define HI_X86_64_V2 1
#define HI_X86_64_V1 1
#define HI_HAS_SSE 1
#define HI_HAS_SSE2 1
#define HI_HAS_SSE3 1
#define HI_HAS_SSE4_1 1
#define HI_HAS_SSE4_2 1
#define HI_HAS_SSSE3 1
#define HI_HAS_AVX 1
#define HI_HAS_AVX2 1
#define HI_HAS_BMI1 1
#define HI_HAS_BMI2 1

#elif defined(__AVX__)
#define HI_X86_64_V2_5 1
#define HI_X86_64_V2 1
#define HI_X86_64_V1 1
#define HI_HAS_SSE 1
#define HI_HAS_SSE2 1
#define HI_HAS_SSE3 1
#define HI_HAS_SSE4_1 1
#define HI_HAS_SSE4_2 1
#define HI_HAS_SSSE3 1
#define HI_HAS_AVX 1

// x86_64_v2 can not be selected in MSVC, but can be in gcc and clang.
#elif defined(__SSE4_2__) && defined(__SSSE3__)
#define HI_X86_64_V2 1
#define HI_X86_64_V1 1
#define HI_HAS_SSE 1
#define HI_HAS_SSE2 1
#define HI_HAS_SSE3 1
#define HI_HAS_SSE4_1 1
#define HI_HAS_SSE4_2 1
#define HI_HAS_SSSE3 1

#elif HI_PROCESSOR == HI_CPU_X64
#define HI_X86_64_V1 1
#define HI_HAS_SSE 1
#define HI_HAS_SSE2 1

#elif HI_PROCESSOR == HI_CPU_X86
#elif HI_PROCESSOR == HI_CPU_ARM64
#elif HI_PROCESSOR == HI_CPU_ARM
#endif

//#if defined(HI_X86_64_V1)
//constexpr bool x86_64_v1 = true;
//#else
//constexpr bool x86_64_v1 = false;
//#endif
//
//#if defined(HI_X86_64_V2)
//constexpr bool x86_64_v2 = true;
//#else
//constexpr bool x86_64_v2 = false;
//#endif
//
//#if defined(HI_X86_64_V2_5)
//constexpr bool x86_64_v2_5 = true;
//#else
//constexpr bool x86_64_v2_5 = false;
//#endif
//
//#if defined(HI_X86_64_V3)
//constexpr bool x86_64_v3 = true;
//#else
//constexpr bool x86_64_v3 = false;
//#endif
//
//#if defined(HI_X86_64_V4)
//constexpr bool x86_64_v4 = true;
//#else
//constexpr bool x86_64_v4 = false;
//#endif

#if HI_PROCESSOR == HI_CPU_X64
/** Minimum offset between two objects to avoid false sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */

/** Maximum size of contiguous memory to promote true sharing. Guaranteed to be at least alignof(std::max_align_t)
 * Part of c++17 but never implemented by clang or gcc.
 */
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
