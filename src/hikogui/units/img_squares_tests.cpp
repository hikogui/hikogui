// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "img_squares.hpp"
#include "pixels.hpp"
#include <hikotest/hikotest.hpp>
#include <hikothird/au.hh>

TEST_SUITE(img_squares) {

TEST_CASE(scale_image)
{
    auto const image_width = hi::unit::pixels(640.0f);

    REQUIRE(hi::unit::img_squares(2.0) * image_width == hi::unit::pixels(1280.0f));
}

};
