// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "safe_int.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>

using namespace std;
using namespace hi;

TEST(SafeIntTests, Add)
{
    tint64_t r;

    hilet minimum = tint64_t{numeric_limits<int64_t>::min()};

    ASSERT_THROW(r = minimum + minimum, std::overflow_error);
}
