// Copyright 2019 Pokitec
// All rights reserved.

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
#define tt_likely(condition) condition
#define tt_unlikely(condition) condition
#define tt_unreachable() __assume(0)
#define tt_assume(condition, ...) __assume(condition)
#define tt_force_inline __forceinline
#define tt_no_inline inline __declspec(noinline)
#define clang_suppress(a)
#define msvc_suppress(a) _Pragma(tt_stringify(warning(disable:a)))

#elif TT_COMPILER == TT_CC_CLANG
#define tt_likely(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define tt_unlikely(condition) __builtin_expect(static_cast<bool>(condition), 0)
#define tt_unreachable() __builtin_unreachable()
#define tt_assume(condition, ...) __builtin_assume(static_cast<bool>(condition))
#define tt_force_inline inline __attribute__((always_inline))
#define tt_no_inline inline __attribute__((noinline))
#define clang_suppress(a) _Pragma(tt_stringify(clang diagnostic ignored a))
#define msvc_suppress(a)

#elif TT_COMPILER == TT_CC_GCC
#define tt_likely(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define tt_unlikely(condition) __builtin_expect(static_cast<bool>(condition), 0)
#define tt_unreachable() __builtin_unreachable()
#define tt_assume(condition, ...) do { if (!(condition)) tt_unreachable(); } while (false)
#define tt_force_inline inline __attribute__((always_inline))
#define tt_no_inline inline __attribute__((noinline))
#define clang_suppress(a)
#define msvc_suppress(a)

#else
#define tt_likely(condition) condition
#define tt_unlikely(condition) condition
#define tt_unreachable() std::terminate()
#define tt_assume(condition, ...) static_assert(sizeof(condition) == 1 __VA_OPT__(,) __VA_ARGS__)
#define tt_force_inline inline
#define tt_no_inline
#define clang_suppress(a)
#define msvc_suppress(a)

#endif

#if TT_BUILD_TYPE == TT_BT_DEBUG
#undef tt_assume
/** In debug mode, replace tt_assume() with an tt_assert().
*/
#define tt_assume(...) tt_assert(__VA_ARGS__)
#endif

constexpr size_t cache_line_size =
    Processor::current == Processor::x64 ? 128 :
    Processor::current == Processor::ARM ? 64 :
    0;

/*! File descriptor/handle
 */
using FileHandle =
    std::conditional_t<OperatingSystem::current == OperatingSystem::Windows,void *,
    std::conditional_t<OperatingSystem::current == OperatingSystem::MacOS,int,
    void>>;

}
