// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "fixed_string.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(fixed_string, from_string_literal)
{
    constexpr auto s = hi::fixed_string{"Hello World"};
    ASSERT_EQ(s, std::string("Hello World"));
    ASSERT_EQ(s.size(), 11);
}
