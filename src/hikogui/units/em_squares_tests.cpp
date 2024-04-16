// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "em_squares.hpp"
#include "points.hpp"
#include <hikotest/hikotest.hpp>
#include <hikothird/au.hh>

TEST_SUITE(em_squares) {

TEST_CASE(points_per_em)
{
    auto my_font_size = hi::points(static_cast<short>(12));

    REQUIRE(hi::em_squares(2.0) * my_font_size == hi::points(24.0));
}

};
