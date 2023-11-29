// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "widget_state.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>


TEST(widget_state, iterate)
{
    auto i = uint16_t{};

    // Iteration should iterate over each value.
    ASSERT_NE(widget_state::begin(), widget_state::end());
    for (auto it = widget_state::begin(); it != widget_state::end(); ++it) {
        ASSERT_EQ(i, static_cast<uint16_t>(it)); 
        ++i;
    }
}