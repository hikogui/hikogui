// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "os_detect.hpp"
#include "assert.hpp"
#include <type_traits>
#include <cstdint>

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

} // namespace tt