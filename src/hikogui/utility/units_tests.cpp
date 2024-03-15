// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "units.hpp"
#include <hikotest/hikotest.hpp>
#include <mp-units/systems/international/international.h>
#include <format>

TEST_SUITE(units) {

TEST_CASE(pixels_per_inch)
{
    mp_units::quantity my_ppi = 72.0 * hi::ppi;

    REQUIRE(2.0 * mp_units::international::inch * my_ppi == 144.0 * hi::pixel);
}

TEST_CASE(points_per_em)
{
    auto my_font_size = 12.0 * mp_units::international::point;

    REQUIRE(2.0 * hi::em_square * my_font_size == 24.0 * mp_units::international::point);
}

};
