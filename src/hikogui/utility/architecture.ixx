// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file architecture.ixx
 *
 * Functions and macros for handling architectural difference between compilers, CPUs and operating systems.
 */

module;

#include "../macros.hpp"

#include <exception>
#include <cstddef>
#include <type_traits>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

export module hikogui_utility_architecture;

export namespace hi::inline v1 {

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


#if (HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG) && (HI_PROCESSOR == HI_CPU_X64 || HI_PROCESSOR == HI_CPU_ARM64)
#define HI_HAS_INT128 1
/** Signed 128 bit integer.
 */
using int128_t = __int128;

/** Unsigned 128 bit integer.
 */
using uint128_t = unsigned __int128;

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
