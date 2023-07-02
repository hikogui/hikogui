// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gidentity.hpp"
#include "../utility/module.hpp"
#include "../utility/test.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi;

TEST(gidentity, identity_vector)
{
    static_assert(std::is_same_v<decltype(gidentity() * vector2(1.0, 2.0)), vector2>);
    static_assert(std::is_same_v<decltype(gidentity() * vector3(1.0, 2.0, 3.0)), vector3>);

    STATIC_ASSERT_TRUE(gidentity() * vector2(1.0, 2.0) == vector2(1.0, 2.0));
    STATIC_ASSERT_TRUE(gidentity() * vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0));
}

TEST(gidentity, identity_point)
{
    static_assert(std::is_same_v<decltype(gidentity() * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype(gidentity() * point3(1.0, 2.0, 3.0)), point3>);

    STATIC_ASSERT_TRUE(gidentity() * point2(1.0, 2.0) == point2(1.0, 2.0));
    STATIC_ASSERT_TRUE(gidentity() * point3(1.0, 2.0, 3.0) == point3(1.0, 2.0, 3.0));
}
