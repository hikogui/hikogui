// Copyright 2019 Pokitec
// All rights reserved.

#include "gap_buffer.hpp"
#include "required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;

TEST(gap_buffer, default_constructor)
{
    auto tmp = gap_buffer<int>{};
    ASSERT_EQ(tmp, gap_buffer<int>{});
}

TEST(gap_buffer, list_initialization)
{
    auto tmp1 = gap_buffer<int>{1, 2, 3};
    auto tmp2 = gap_buffer<int>{1, 2, 3};
    ASSERT_EQ(tmp1.size(), 3);
    ASSERT_EQ(tmp2.size(), 3);
    ASSERT_EQ(tmp2[0], 1);
    ASSERT_EQ(tmp2[1], 2);
    ASSERT_EQ(tmp2[2], 3);
    ASSERT_EQ(tmp1, tmp2);
}
