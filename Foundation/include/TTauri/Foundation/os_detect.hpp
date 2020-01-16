// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <exception>
#include <cstddef>

namespace TTauri {

#define OS_WINDOWS 'W'
#define OS_MACOS 'M'
#define OS_IOS 'i'
#define OS_LINUX 'L'
#define OS_POSIX 'P'
#define OS_UNIX 'U'
#define OS_ANDROID 'A'

/* Create specific macros to detect the operating system.
 */
#if defined(_WIN64)
#define OPERATING_SYSTEM OS_WINDOWS
#elif defined(_WIN32)
#define OPERATING_SYSTEM OS_WINDOWS
#elif defined(__APPLE__)
  #include "TargetConditionals.h"
  #if TARGET_OS_IPHONE == 1
  #define OPERATING_SYSTEM OS_IOS
  #else
  #define OPERATING_SYSTEM OS_MACOS
  #endif
#elif defined(__ANDROID__)
#define OPERATING_SYSTEM OS_ANDROID
#elif defined(__linux)
#define OPERATING_SYSTEM OS_LINUX
#elif defined(__unix)
#define OPERATING_SYSTEM OS_UNIX
#elif defined(__posix)
#define OPERATING_SYSTEM OS_POSIX
#endif

enum class OperatingSystem {
    Windows,
    MacOS,
    iOS,
    Linux,
    Android,
    UNIX,
    Posix
};

/*! Used for describing the look and feel of the application.
 * Use operating supplied macros for detecting APIs
 */
#if OPERATING_SYSTEM == OS_WINDOWS
constexpr auto operatingSystem = OperatingSystem::Windows;
#elif OPERATING_SYSTEM == OS_MACOS
constexpr auto operatingSystem = OperatingSystem::MacOS;
#elif OPERATING_SYSTEM == OS_IOS
constexpr auto operatingSystem = OperatingSystem::iOS;
#elif OPERATING_SYSTEM == OS_ANDROID
constexpr auto operatingSystem = OperatingSystem::ANDROID;
#elif OPERATING_SYSTEM == OS_LINUX
constexpr auto operatingSystem = OperatingSystem::Linux;
#elif OPERATING_SYSTEM == OS_UNIX
constexpr auto operatingSystem = OperatingSystem::UNIX;
#elif OPERATING_SYSTEM == OS_POSIX
constexpr auto operatingSystem = OperatingSystem::Posix;
#else
#error "Could not detect the operating system."
#endif


#define CC_MSVC 'm'
#define CC_GCC 'g'
#define CC_CLANG 'c'

#if defined(__clang__)
#define COMPILER CC_CLANG
#elif defined(_MSC_BUILD)
#define COMPILER CC_MSVC
#elif defined(__GNUC__)
#define COMPILER CC_GCC
#endif

enum class Compiler {
    MSVC,
    gcc,
    clang
};

/*! Used for describing the look and feel of the application.
* Use operating supplied macros for detecting APIs
*/
#if COMPILER == CC_MSVC
constexpr auto compiler = Compiler::MSVC;
#elif COMPILER == CC_GCC
constexpr auto compiler = Compiler::gcc;
#elif COMPILER == CC_CLANG
constexpr auto compiler = Compiler::clang;
#else
#error "Could not detect the compiler."
#endif


#define CPU_X64 'i'
#define CPU_ARM 'a'

#if defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64)
#define PROCESSOR CPU_X64
#elif defined(__arm__) || defined(_M_ARM)
#define PROCESSOR CPU_ARM
#endif

enum class Processor {
    X64,
    ARM
};

#if PROCESSOR == CPU_X64
constexpr auto processor = Processor::X64;
#elif PROCESSOR == CPU_ARM
constexpr auto processor = Processor::ARM;
#else
#error "Could not detect processor."
#endif

#define STRINGIFY(a) #a

#if COMPILER == CC_MSVC
#define ttauri_likely(condition) condition
#define ttauri_unlikely(condition) condition
#define ttauri_unreachable() std::terminate()
#define ttauri_assume(condition) __assume(condition)
#define force_inline __forceinline
#define no_inline __declspec(noinline)
#define clang_suppress(a)
#define gsl_suppress(a) [[gsl::suppress(a)]]
#define gsl_suppress2(a,b) [[gsl::suppress(a)]] [[gsl::suppress(b)]]
#define gsl_suppress3(a,b,c) [[gsl::suppress(a)]] [[gsl::suppress(b)]] [[gsl::suppress(c)]]
#define gsl_suppress4(a,b,c,d) [[gsl::suppress(a)]] [[gsl::suppress(b)]] [[gsl::suppress(c)]] [[gsl::suppress(d)]]
#define gsl_suppress5(a,b,c,d,e) [[gsl::suppress(a)]] [[gsl::suppress(b)]] [[gsl::suppress(c)]] [[gsl::suppress(d)]] [[gsl::suppress(e)]]

#elif COMPILER == CC_CLANG
#define ttauri_likely(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define ttauri_unlikely(condition) __builtin_expect(static_cast<bool>(condition), 0)
#define ttauri_unreachable() __builtin_unreachable()
#define ttauri_assume(condition) do { if (!(condition)) ttauri_unreachable(); } while (false)
#define force_inline inline __attribute__((always_inline))
#define no_inline inline __attribute__((noinline))
#define clang_suppress(a) _Pragma(STRINGIFY(clang diagnostic ignored a))
#define gsl_suppress(a) [[gsl::suppress(#a)]]
#define gsl_suppress2(a,b) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]]
#define gsl_suppress3(a,b,c) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]]
#define gsl_suppress4(a,b,c,d) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]]
#define gsl_suppress5(a,b,c,d,e) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]] [[gsl::suppress(#e)]]

#elif COMPILER == CC_GCC
#define ttauri_likely(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define ttauri_unlikely(condition) __builtin_expect(static_cast<bool>(condition), 0)
#define ttauri_unreachable() __builtin_unreachable()
#define ttauri_assume(condition) do { if (!(condition)) ttauri_unreachable(); } while (false)
#define force_inline inline __attribute__((always_inline))
#define no_inline inline __attribute__((noinline))
#define clang_suppress(a)
#define gsl_suppress(a) [[gsl::suppress(#a)]]
#define gsl_suppress2(a,b) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]]
#define gsl_suppress3(a,b,c) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]]
#define gsl_suppress4(a,b,c,d) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]]
#define gsl_suppress5(a,b,c,d,e) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]] [[gsl::suppress(#e)]]

#else
#define ttauri_likely(condition) condition
#define ttauri_unlikely(condition) condition
#define ttauri_unreachable() std::terminate()
#define ttauri_assume(condition) static_assert(sizeof(condition) == 1)
#define force_inline inline
#define no_inline
#define clang_suppress(a)
#define gsl_suppress(a)
#define gsl_suppress2(a,b)
#define gsl_suppress3(a,b,c)
#define gsl_suppress4(a,b,c,d)
#define gsl_suppress5(a,b,c,d,e)

#endif

#if !defined(NDEBUG)
#undef ttauri_assume
/** In debug mode, replace ttauri_assume() with an ttauri_assert().
 */
#define ttauri_assume(expression) ttauri_assert(expression)
#endif

#if PROCESSOR == CPU_X64
constexpr size_t cache_line_size = 128;
#else
#error "Not implemented"
#endif

/*! File descriptor/handle
 */
#if OPERATING_SYSTEM == OS_WINDOWS
using FileHandle = void *;
#else
using FileHandle = int;
#endif

}
