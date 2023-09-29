// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "interval.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>

static_assert(hi::interval<int>{0, 5}.lower() == 0);
static_assert(hi::interval<int>{0, 5}.upper() == 5);
