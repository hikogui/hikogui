// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/interval.hpp"
#include "ttauri/bigint.hpp"
#include <gtest/gtest.h>

static_assert(tt::interval<int>{0, 5}.lower() == 0);
static_assert(tt::interval<int>{0, 5}.upper() == 5);

static_assert(tt::interval<tt::big128>{0, 5}.lower() == 0);
static_assert(tt::interval<tt::big128>{0, 5}.upper() == 5);

static_assert(0 == tt::interval<tt::big128>{0, 5}.lower());
static_assert(5 == tt::interval<tt::big128>{0, 5}.upper());

static_assert(tt::interval<tt::big128>{0, 5}.lower() <= signed char{3});
static_assert(tt::interval<tt::big128>{0, 5}.upper() >= signed char{3});

static_assert(signed char{3} >= tt::interval<tt::big128>{0, 5}.lower());
static_assert(signed char{3} <= tt::interval<tt::big128>{0, 5}.upper());
