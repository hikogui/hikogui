// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#define OS_WINDOWS 'W'
#define OS_MACOS 'M'
#define OS_IOS 'i'
#define OS_LINUX 'L'
#define OS_POSIX 'P'
#define OS_UNIX 'U'
#define OS_ANDROID 'A'

/* Create specific macros to detect the operating system.
 */
#ifdef _WIN64
#define OPERATING_SYSTEM OS_WINDOWS
#elif _WIN32
#define OPERATING_SYSTEM OS_WINDOWS
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
#define OPERATING_SYSTEM OS_IOS
#elif TARGET_OS_IPHONE
#define OPERATING_SYSTEM OS_IOS
#else
#define OPERATING_SYSTEM OS_MACOS
#endif
#elif __ANDROID__
#define OPERATING_SYSTEM OS_ANDROID
#elif __linux
#define OPERATING_SYSTEM OS_LINUX
#elif __unix
#define OPERATING_SYSTEM OS_UNIX
#elif __posix
#define OPERATING_SYSTEM OS_POSIX
#endif

namespace TTauri {

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

}
