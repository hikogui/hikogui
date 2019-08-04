// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <exception>


/*! Invariant should be the default for variables.
 * C++ does have an invariant but it requires you to enter the 'const' keyword which
 * is easy to forget. Using a single keyword 'let' for an invariant makes it easier to notice
 * when you have defined a variant.
 */
#define let auto const

#define required_assert(x) if (!(x)) { std::terminate(); }

#define no_default { std::terminate(); }


/* Create specific macros to detect the operating system.
 */
#ifdef _WIN64
#define TTAURI_WINDOWS 1
#elif _WIN32
#define TTAURI_WINDOWS 1
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
#define TTAURI_IOS 1
#define TTAURI_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define TTAURI_IOS 1
#else
#define TTAURI_MACOS 1
#endif
#elif __linux
#define TTAURI_LINUX 1
#elif __unix // all unices not caught above
#define TTAURI_UNIX 1
#elif __posix
#define TTAURI_POSIX 1
#endif

namespace TTauri {

enum class OperatingSystem {
    Windows10,
    MacOS,
    Linux,
    Unix,
    Posix
};

/*! Used for describing the look and feel of the application.
 * Use operating supplied macros for detecting APIs
 */
constexpr auto operatingSystem = OperatingSystem::MacOS;

}
