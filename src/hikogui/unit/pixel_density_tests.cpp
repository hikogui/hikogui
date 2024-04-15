// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pixel_density.hpp"
#include "pixels.hpp"
#include "dips.hpp"
#include <hikotest/hikotest.hpp>
#include <hikothird/au.hh>

TEST_SUITE(pixels_per_inch) {

TEST_CASE(inch_to_pixel)
{
    auto density = pixel_density{hi::pixels_per_inch(170.0), hi::device_type::phone};

    REQUIRE(au::inches(2.0) * density == hi::pixels(340.0));
}

TEST_CASE(dips_to_pixel_medium)
{
    auto density = pixel_density{hi::pixels_per_inch(170.0), hi::device_type::phone};

    REQUIRE(au::dips(2.0) * density == hi::pixels(2.0));
}

TEST_CASE(dips_to_pixel_high)
{
    auto density = pixel_density{hi::pixels_per_inch(250.0), hi::device_type::phone};

    REQUIRE(au::dips(2.0) * density == hi::pixels(3.0));
}

};
