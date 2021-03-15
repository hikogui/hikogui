// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <exception>
#include <cstddef>
#include <type_traits>

namespace tt {

#define TT_BT_DEBUG 'D'
#define TT_BT_RELEASE 'R'

#if defined(NDEBUG)
#define TT_BUILD_TYPE TT_BT_RELEASE
#else
#define TT_BUILD_TYPE TT_BT_DEBUG
#endif

enum class BuildType {
    Debug = TT_BT_DEBUG,
    Release = TT_BT_RELEASE,

    current = TT_BUILD_TYPE
};

#define TT_OS_WINDOWS 'W'
#define TT_OS_MACOS 'M'
#define TT_OS_IOS 'i'
#define TT_OS_LINUX 'L'
#define TT_OS_POSIX 'P'
#define TT_OS_UNIX 'U'
#define TT_OS_ANDROID 'A'

/* Create specific macros to detect the operating system.
 */
#if defined(_WIN64)
#define  TT_OPERATING_SYSTEM TT_OS_WINDOWS
#elif defined(_WIN32)
#define  TT_OPERATING_SYSTEM TT_OS_WINDOWS
#elif defined(__APPLE__)
  #include "TargetConditionals.h"
  #if TARGET_OS_IPHONE == 1
  #define  TT_OPERATING_SYSTEM TT_OS_IOS
  #else
  #define  TT_OPERATING_SYSTEM TT_OS_MACOS
  #endif
#elif defined(__ANDROID__)
#define  TT_OPERATING_SYSTEM TT_OS_ANDROID
#elif defined(__linux)
#define  TT_OPERATING_SYSTEM TT_OS_LINUX
#elif defined(__unix)
#define  TT_OPERATING_SYSTEM TT_OS_UNIX
#elif defined(__posix)
#define  TT_OPERATING_SYSTEM TT_OS_POSIX
#else
#error "Could not detect the operating system."
#endif

enum class OperatingSystem {
    Windows = TT_OS_WINDOWS,
    MacOS = TT_OS_MACOS,
    iOS = TT_OS_IOS,
    Linux = TT_OS_LINUX,
    Android = TT_OS_ANDROID,
    UNIX = TT_OS_UNIX,
    Posix = TT_OS_POSIX,

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

enum class Compiler {
    MSVC = TT_CC_MSVC,
    gcc = TT_CC_GCC,
    clang = TT_CC_CLANG,

    current = TT_COMPILER
};

#define TT_CPU_X64 'i'
#define TT_CPU_ARM 'a'

#define TT_CPPVER_17 '7'
#define TT_CPPVER_20 '2'

#if __cplusplus > 201703L
#define TT_CPP_VERSION TT_CPPVER_20
#elif __cplusplus == 201703L
#define TT_CPP_VERSION TT_CPPVER_17
#else
#error "Unknown C++ version"
#endif
enum class CPPVersion {
    CPP17 = TT_CPPVER_17,
    CPP20 = TT_CPPVER_20,

    current = TT_CPP_VERSION
};

#if defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64)
#define TT_PROCESSOR TT_CPU_X64
#elif defined(__arm__) || defined(_M_ARM)
#define TT_PROCESSOR TT_CPU_ARM
#else
#error "Could not detect processor."
#endif

enum class Processor {
    x64 = TT_CPU_X64,
    ARM = TT_CPU_ARM,

    current = TT_PROCESSOR
};

#define tt_stringify(a) #a

#if TT_COMPILER == TT_CC_MSVC
#define tt_unreachable() __assume(0)
#define tt_assume(condition) __assume(condition)
#define tt_assume2(condition, msg) __assume(condition)
#define tt_force_inline __forceinline
#define tt_no_inline __declspec(noinline)
#define clang_suppress(a)
#define msvc_suppress(a) _Pragma(tt_stringify(warning(disable:a)))

#elif TT_COMPILER == TT_CC_CLANG
#define tt_unreachable() __builtin_unreachable()
#define tt_assume(condition) __builtin_assume(static_cast<bool>(condition))
#define tt_assume2(condition, msg) __builtin_assume(static_cast<bool>(condition))
#define tt_force_inline inline __attribute__((always_inline))
#define tt_no_inline __attribute__((noinline))
#define clang_suppress(a) _Pragma(tt_stringify(clang diagnostic ignored a))
#define msvc_suppress(a)

#elif TT_COMPILER == TT_CC_GCC
#define tt_unreachable() __builtin_unreachable()
#define tt_assume(condition) do { if (!(condition)) tt_unreachable(); } while (false)
#define tt_assume2(condition, msg) do { if (!(condition)) tt_unreachable(); } while (false)
#define tt_force_inline inline __attribute__((always_inline))
#define tt_no_inline __attribute__((noinline))
#define clang_suppress(a)
#define msvc_suppress(a)

#else
#define tt_unreachable() std::terminate()
#define tt_assume(condition) static_assert(sizeof(condition) == 1)
#define tt_assume2(condition, msg) static_assert(sizeof(condition) == 1, msg)
#define tt_force_inline inline
#define tt_no_inline
#define clang_suppress(a)
#define msvc_suppress(a)

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
#else
#error "Missing implementation of hardware_destructive_interference_size and hardware_constructive_interference_size"
#endif

constexpr bool has_sse = Processor::current == Processor::x64;

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

}
