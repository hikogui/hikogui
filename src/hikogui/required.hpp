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

#ifndef hilet
/** Invariant should be the default for variables.
 *
 * C++ does have an invariant but it requires you to enter the 'const' keyword which
 * is easy to forget. Using a single keyword 'hilet' for an invariant makes it easier to notice
 * when you have defined a variant.
 */
#define hilet auto const
#endif

#ifndef hi_forward
/** Forward a value, based on the decltype of the value.
 */
#define hi_forward(x) std::forward<decltype(x)>(x)
#endif

namespace hi::inline v1 {


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

#define hi_return_on_self_assignment(other) \
    if (&(other) == this) [[unlikely]] \
        return *this;

} // namespace hi::inline v1
