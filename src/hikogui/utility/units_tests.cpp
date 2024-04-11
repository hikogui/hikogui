// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "units.hpp"
#include <hikotest/hikotest.hpp>
#include <hikothird/au.hh>
#include <format>

TEST_SUITE(units) {

TEST_CASE(pixels_per_inch)
{
    auto my_ppi = hi::pixels_per_inch(72.0);

    REQUIRE(hi::as_pixels(au::inches(2.0), my_ppi) == hi::pixels(144.0));
}

TEST_CASE(points_per_em)
{
    auto my_font_size = hi::points(static_cast<short>(12));

    REQUIRE(hi::to_length(hi::em_squares(2.0), my_font_size) == hi::points(24.0));
}

};
