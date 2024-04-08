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
    auto my_ppi = 72.0 * hi::ppi;

    REQUIRE(hi::to_pixel(2.0 * mp_units::international::inch, my_ppi) == 144.0 * hi::pixel);
}

TEST_CASE(points_per_em)
{
    //auto my_font_size = 12.0 * hi::font_size[mp_units::international::point];
    mp_units::quantity<hi::font_size[mp_units::international::point], short> my_font_size = 12 * mp_units::international::point;

    REQUIRE(hi::to_length(2.0 * hi::em, my_font_size) == 24.0 * mp_units::international::point);
}

};
