// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/small_map.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(SmallMap, Default)
{
    small_map<int, int, 4> items;
    ASSERT_EQ(items.size(), 0);

    ASSERT_EQ(items.set(10, 100), true);
    ASSERT_EQ(items.size(), 1);

    ASSERT_EQ(items.set(20, 200), true);
    ASSERT_EQ(items.size(), 2);

    ASSERT_EQ(items.set(10, 1000), true);
    ASSERT_EQ(items.size(), 2);

    ASSERT_EQ(items.set(30, 300), true);
    ASSERT_EQ(items.size(), 3);

    ASSERT_EQ(items.set(40, 400), true);
    ASSERT_EQ(items.size(), 4);

    ASSERT_EQ(items.set(50, 500), false);
    ASSERT_EQ(items.size(), 4);

    ASSERT_EQ(items.get(10), std::optional<int>{1000});
    ASSERT_EQ(items.get(20), std::optional<int>{200});
    ASSERT_EQ(items.get(30), std::optional<int>{300});
    ASSERT_EQ(items.get(40), std::optional<int>{400});
    ASSERT_EQ(items.get(50), std::optional<int>{});

    ASSERT_EQ(items.get(10, 42), 1000);
    ASSERT_EQ(items.get(20, 42), 200);
    ASSERT_EQ(items.get(30, 42), 300);
    ASSERT_EQ(items.get(40, 42), 400);
    ASSERT_EQ(items.get(50, 42), 42);
}
