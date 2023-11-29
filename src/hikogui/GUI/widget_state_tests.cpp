// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "widget_state.hpp"
#include <gtest/gtest.h>
#include <cstddef>

TEST(widget_state, iterate)
{
    // Iteration should iterate over each state once, and the value should match the index..
    ASSERT_NE(hi::widget_state::begin(), hi::widget_state::end());

    auto i = size_t{0};
    for (auto it = hi::widget_state::begin(); it != hi::widget_state::end(); ++it) {
        ASSERT_EQ(i, static_cast<size_t>(it)); 
        ++i;
    }
    
    ASSERT_EQ(i, hi::widget_state::size());
}
