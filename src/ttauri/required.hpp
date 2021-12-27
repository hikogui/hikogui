// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file required.hpp
 *
 * This file includes required definitions.
 */

#pragma once

#include <cstddef>
#include <string>
#include <chrono>

#ifndef ttlet
/** Invariant should be the default for variables.
 *
 * C++ does have an invariant but it requires you to enter the 'const' keyword which
 * is easy to forget. Using a single keyword 'ttlet' for an invariant makes it easier to notice
 * when you have defined a variant.
 */
#define ttlet auto const
#endif

// Windows.h adds a "IN" macro that is used in this enum.
#ifdef IN
#undef IN
#endif

namespace tt::inline v1 {


/** Signed size/index into an array.
 */
using ssize_t = std::ptrdiff_t;

#define ssizeof(x) (static_cast<ssize_t>(sizeof(x)))

constexpr std::size_t operator"" _uz(unsigned long long lhs) noexcept
{
    return static_cast<std::size_t>(lhs);
}

constexpr std::size_t operator"" _zu(unsigned long long lhs) noexcept
{
    return static_cast<std::size_t>(lhs);
}

constexpr std::ptrdiff_t operator"" _z(unsigned long long lhs) noexcept
{
    return static_cast<std::ptrdiff_t>(lhs);
}

#define tt_return_on_self_assignment(other) \
    if (&(other) == this) [[unlikely]] \
        return *this;

} // namespace tt::inline v1
