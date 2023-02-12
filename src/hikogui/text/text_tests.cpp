// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text.hpp"
#include <gtest/gtest.h>

TEST(text, construct)
{
    auto r = text{};
    ASSERT_TRUE(r.empty());
}
