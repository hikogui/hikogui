// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme_model.hpp"
#include <gtest/gtest.h>

TEST(theme_model, pixel_length)
{
    struct delegate_type {
        [[nodiscard]] std::pair<hi::theme_state, int> state_and_scale() const noexcept {
            return {hi::theme_state::enabled, -2};
        }
    };

    auto &m = hi::theme_model_by_key("pixel-length-test");
    m[hi::theme_state::enabled].width = hi::pixels{42};

    auto d = delegate_type{};
    ASSERT_FLOAT_EQ(hi::theme<"pixel-length-test">.width(&d), 42);
}
