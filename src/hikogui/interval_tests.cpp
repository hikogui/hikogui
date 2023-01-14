// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/interval.hpp"
#include "hikogui/stdint.hpp"
#include <gtest/gtest.h>

static_assert(hi::interval<int>{0, 5}.lower() == 0);
static_assert(hi::interval<int>{0, 5}.upper() == 5);

static_assert(hi::interval<hi::uint128_t>{0, 5}.lower() == 0);
static_assert(hi::interval<hi::uint128_t>{0, 5}.upper() == 5);

static_assert(0 == hi::interval<hi::uint128_t>{0, 5}.lower());
static_assert(5 == hi::interval<hi::uint128_t>{0, 5}.upper());

static_assert(hi::interval<hi::uint128_t>{0, 5}.lower() <= signed char{3});
static_assert(hi::interval<hi::uint128_t>{0, 5}.upper() >= signed char{3});

static_assert(signed char{3} >= hi::interval<hi::uint128_t>{0, 5}.lower());
static_assert(signed char{3} <= hi::interval<hi::uint128_t>{0, 5}.upper());
