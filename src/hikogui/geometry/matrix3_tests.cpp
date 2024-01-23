// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "matrix3.hpp"
#include "transform.hpp"
#include <hikotest/hikotest.hpp>

using namespace hi;

TEST_SUITE(matrix3) {

TEST_CASE(invert)
{
    // clang-format off
    auto test_XYZ_to_sRGB = matrix3{
         3.24096994f, -1.53738318f, -0.49861076f, 0.0f,
        -0.96924364f,  1.87596750f,  0.04155506f, 0.0f,
         0.05563008f, -0.20397696f,  1.05697151f, 0.0f,
         0.0f,         0.0f,         0.0f,        1.0f};
    // clang-format on

    auto result_sRGB_to_XYZ = ~test_XYZ_to_sRGB;

    REQUIRE(get<0>(result_sRGB_to_XYZ) == f32x4(0.41239080f, 0.21263901f, 0.01933082f, 0.0f), 0.001f);
    REQUIRE(get<1>(result_sRGB_to_XYZ) == f32x4(0.35758434f, 0.71516868f, 0.11919478f, 0.0f), 0.001f);
    REQUIRE(get<2>(result_sRGB_to_XYZ) == f32x4(0.18048079f, 0.07219232f, 0.95053215f, 0.0f), 0.001f);
    REQUIRE(get<3>(result_sRGB_to_XYZ) == f32x4(0.0f, 0.0f, 0.0f, 1.0f), 0.001f);
    return {};
}

};