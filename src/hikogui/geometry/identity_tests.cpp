// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "identity.hpp"
#include "../utility/module.hpp"
#include "../utility/test.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi;
using namespace hi::geo;

TEST(geometry, identity_vector)
{
    static_assert(std::is_same_v<decltype(identity() * vector2(1.0, 2.0)), vector2>);
    static_assert(std::is_same_v<decltype(identity() * vector3(1.0, 2.0, 3.0)), vector3>);

    STATIC_ASSERT_TRUE(identity() * vector2(1.0, 2.0) == vector2(1.0, 2.0));
    STATIC_ASSERT_TRUE(identity() * vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0));
}

TEST(geometry, identity_point)
{
    static_assert(std::is_same_v<decltype(identity() * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype(identity() * point3(1.0, 2.0, 3.0)), point3>);

    STATIC_ASSERT_TRUE(identity() * point2(1.0, 2.0) == point2(1.0, 2.0));
    STATIC_ASSERT_TRUE(identity() * point3(1.0, 2.0, 3.0) == point3(1.0, 2.0, 3.0));
}
