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