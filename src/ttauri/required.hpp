// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <cstddef>
#include <string>
#include <chrono>

namespace tt {

using namespace std::literals;

/*! Invariant should be the default for variables.
 * C++ does have an invariant but it requires you to enter the 'const' keyword which
 * is easy to forget. Using a single keyword 'ttlet' for an invariant makes it easier to notice
 * when you have defined a variant.
 */
#ifndef ttlet
#define ttlet auto const
#endif

/*! Signed size/index into an array.
 */
using ssize_t = std::ptrdiff_t;

#define ssizeof(x) (static_cast<ssize_t>(sizeof(x)))

constexpr size_t operator"" _uz(unsigned long long lhs) noexcept
{
    return static_cast<size_t>(lhs);
}

constexpr size_t operator"" _zu(unsigned long long lhs) noexcept
{
    return static_cast<size_t>(lhs);
}

constexpr ssize_t operator"" _z(unsigned long long lhs) noexcept
{
    return static_cast<ssize_t>(lhs);
}

} // namespace tt