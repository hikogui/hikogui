// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pixels_per_inch.hpp"
#include "pixels.hpp"
#include <hikotest/hikotest.hpp>
#include <hikothird/au.hh>

TEST_SUITE(pixels_per_inch) {

TEST_CASE(inch_to_pixel)
{
    auto my_ppi = hi::unit::pixels_per_inch(72.0);

    REQUIRE(au::inches(2.0) * my_ppi == hi::unit::pixels(144.0));
}

};
