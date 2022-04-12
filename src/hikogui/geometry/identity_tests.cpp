// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/geometry/identity.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi::geo;

TEST(geometry, identity_vector)
{
    static_assert(std::is_same_v<decltype(identity() * vector<2>(1.0, 2.0)), vector<2>>);
    static_assert(std::is_same_v<decltype(identity() * vector<3>(1.0, 2.0, 3.0)), vector<3>>);

    static_assert(identity() * vector<2>(1.0, 2.0) == vector<2>(1.0, 2.0));
    static_assert(identity() * vector<3>(1.0, 2.0, 3.0) == vector<3>(1.0, 2.0, 3.0));

    ASSERT_TRUE(identity() * vector<2>(1.0, 2.0) == vector<2>(1.0, 2.0));
    ASSERT_TRUE(identity() * vector<3>(1.0, 2.0, 3.0) == vector<3>(1.0, 2.0, 3.0));
}

TEST(geometry, identity_point)
{
    static_assert(std::is_same_v<decltype(identity() * point<2>(1.0, 2.0)), point<2>>);
    static_assert(std::is_same_v<decltype(identity() * point<3>(1.0, 2.0, 3.0)), point<3>>);

    static_assert(identity() * point<2>(1.0, 2.0) == point<2>(1.0, 2.0));
    static_assert(identity() * point<3>(1.0, 2.0, 3.0) == point<3>(1.0, 2.0, 3.0));

    ASSERT_TRUE(identity() * point<2>(1.0, 2.0) == point<2>(1.0, 2.0));
    ASSERT_TRUE(identity() * point<3>(1.0, 2.0, 3.0) == point<3>(1.0, 2.0, 3.0));
}
