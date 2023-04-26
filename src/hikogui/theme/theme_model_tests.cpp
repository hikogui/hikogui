// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme_model.hpp"
#include <gtest/gtest.h>

TEST(theme_model, pixel_length)
{
    struct delegate_type {
        [[nodiscard]] hi::sub_theme_selector_type sub_theme_selector() const noexcept
        {
            // Scale by 200%.
            return {hi::theme_state::enabled, -8};
        }
    };

    auto &m = hi::theme_model_by_key("pixel-length-test");
    m[hi::theme_state::enabled].width = hi::pixels{42};

    // Pixels aren't scaled
    auto d = delegate_type{};
    ASSERT_FLOAT_EQ(hi::theme<"pixel-length-test">.width(&d), 42.0);
}

TEST(theme_model, dip_length)
{
    struct delegate_type {
        [[nodiscard]] hi::sub_theme_selector_type sub_theme_selector() const noexcept
        {
            // Scale by 200%.
            return {hi::theme_state::enabled, -8};
        }
    };

    auto& m = hi::theme_model_by_key("dip-length-test");
    m[hi::theme_state::enabled].width = hi::dips{42};

    // Device independent pixels are scaled.
    auto d = delegate_type{};
    ASSERT_FLOAT_EQ(hi::theme<"dip-length-test">.width(&d), 84.0);
}
