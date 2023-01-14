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
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;
using intreg_t = int32_t;
using uintreg_t = uint32_t;
#elif HI_PROCESSOR == HI_CPU_X64
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;
using intreg_t = int64_t;
using uintreg_t = uint64_t;
#elif HI_PROCESSOR == HI_CPU_ARM
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;
using intreg_t = int32_t;
using uintreg_t = uint32_t;
#elif HI_PROCESSOR == HI_CPU_ARM64
constexpr std::size_t hardware_destructive_interference_size = 128;
constexpr std::size_t hardware_constructive_interference_size = 64;
using intreg_t = int64_t;
using uintreg_t = uint64_t;
#else
#error "missing implementation for CPU specific register and cache-line sizes"
#endif

#if defined(__AVX512BW__) && defined(__AVX512CD__) && defined(__AVX512DQ__) && defined(__AVX512F__) && defined(__AVX512VL__)
#define HI_X86_64_LEVEL 4
#elif defined(__AVX2__)
#define HI_X86_64_LEVEL 3
#elif defined(__SSE4_2__) && defined(__SSSE3__)
#define HI_X86_64_LEVEL 2
#elif HI_PROCESSOR == HI_CPU_X64
#define HI_X86_64_LEVEL 1
#endif

#if defined(HI_X86_64_MAX_LEVEL) && defined(HI_X86_64_LEVEL) && HI_X86_64_MAX_LEVEL < HI_X86_64_LEVEL
#undef HI_X86_64_LEVEL
#define HI_X86_64_LEVEL HI_X86_64_MAX_LEVEL
#endif

#if defined(HI_X86_64_LEVEL) && HI_X86_64_LEVEL >= 4
#define HI_HAS_AVX512F 1
#define HI_HAS_AVX512BW 1
#define HI_HAS_AVX512CD 1
#define HI_HAS_AVX512DQ 1
#define HI_HAS_AVX512VL 1
#endif

#if defined(HI_X86_64_LEVEL) && HI_X86_64_LEVEL >= 3
#define HI_HAS_AVX 1
#define HI_HAS_AVX2 1
#define HI_HAS_BMI1 1
#define HI_HAS_BMI2 1
#define HI_HAS_F16C 1
#define HI_HAS_FMA 1
#define HI_HAS_LZCNT 1
#define HI_HAS_MOVBE 1
#define HI_HAS_OSXSAVE 1
#endif

#if defined(HI_X86_64_LEVEL) && HI_X86_64_LEVEL >= 2
#define HI_HAS_CMPXCHG16B 1
#define HI_HAS_LAHF_SAHF 1
#define HI_HAS_POPCNT 1
#define HI_HAS_SSE3 1
#define HI_HAS_SSE4_1 1
#define HI_HAS_SSE4_2 1
#define HI_HAS_SSSE3 1
#endif

#if defined(HI_X86_64_LEVEL) && HI_X86_64_LEVEL >= 1
#define HI_HAS_CMOV 1
#define HI_HAS_CX8 1
#define HI_HAS_FPU 1
#define HI_HAS_FXSR 1
#define HI_HAS_MMX 1
#define HI_HAS_OSFXSR 1
#define HI_HAS_SCE 1
#define HI_HAS_SSE 1
#define HI_HAS_SSE2 1
#endif

#if (HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG) && (HI_PROCESSOR == HI_CPU_X64 || HI_PROCESSOR == HI_CPU_ARM64)
#define HI_HAS_INT128 1
/** Signed 128 bit integer.
 */
using int128_t = __int128_t;

/** Unsigned 128 bit integer.
 */
using uint128_t = unsigned __int128_t;

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
