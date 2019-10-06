// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/os_detect.hpp"
#include "TTauri/Required/assert.hpp"
#include <exception>
#include <stdexcept>
#include <type_traits>

/*! Invariant should be the default for variables.
 * C++ does have an invariant but it requires you to enter the 'const' keyword which
 * is easy to forget. Using a single keyword 'let' for an invariant makes it easier to notice
 * when you have defined a variant.
 */
#define let auto const



namespace TTauri {

/*! Signed size/index into an array.
*/
using ssize_t = std::make_signed_t<size_t>;

constexpr size_t cache_line_size = 128;


}
