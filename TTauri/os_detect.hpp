// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

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

}
