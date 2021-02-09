// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/geometry/mat.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(matrix, identity_vec)
{
    static_assert(std::is_same_v<decltype(matrix::I() * vec2(1.0, 2.0)), vec2>);
    static_assert(std::is_same_v<decltype(matrix::I() * vec3(1.0, 2.0, 3.0)), vec3>);

    static_assert(matrix::I() * vec2(1.0, 2.0) == vec2(1.0, 2.0));
    static_assert(matrix::I() * vec3(1.0, 2.0, 3.0) == vec3(1.0, 2.0, 3.0));

    ASSERT_TRUE(matrix::I() * vec2(1.0, 2.0) == vec2(1.0, 2.0));
    ASSERT_TRUE(matrix::I() * vec3(1.0, 2.0, 3.0) == vec3(1.0, 2.0, 3.0));
}

TEST(matrix, identity_point)
{
    static_assert(std::is_same_v<decltype(matrix::I() * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype(matrix::I() * point3(1.0, 2.0, 3.0)), point3>);

    static_assert(matrix::I() * point2(1.0, 2.0) == point2(1.0, 2.0));
    static_assert(matrix::I() * point3(1.0, 2.0, 3.0) == point3(1.0, 2.0, 3.0));

    ASSERT_TRUE(matrix::I() * point2(1.0, 2.0) == point2(1.0, 2.0));
    ASSERT_TRUE(matrix::I() * point3(1.0, 2.0, 3.0) == point3(1.0, 2.0, 3.0));
}

TEST(matrix, identity_translate)
{
    static_assert(std::is_same_v<decltype(matrix::I() * matrix::T2(1.0, 2.0)), matrix::T2>);
    static_assert(std::is_same_v<decltype(matrix::I() * matrix::T3(1.0, 2.0, 3.0)), matrix::T3>);

    static_assert(matrix::I() * matrix::T2(1.0, 2.0) == matrix::T2(1.0, 2.0));
    static_assert(matrix::I() * matrix::T3(1.0, 2.0, 3.0) == matrix::T3(1.0, 2.0, 3.0));

    ASSERT_TRUE(matrix::I() * matrix::T2(1.0, 2.0) == matrix::T2(1.0, 2.0));
    ASSERT_TRUE(matrix::I() * matrix::T3(1.0, 2.0, 3.0) == matrix::T3(1.0, 2.0, 3.0));
}

TEST(matrix, identity_scale)
{
    static_assert(std::is_same_v<decltype(matrix::I() * matrix::S2(1.0, 2.0)), matrix::S2>);
    static_assert(std::is_same_v<decltype(matrix::I() * matrix::S3(1.0, 2.0, 3.0)), matrix::S3>);

    static_assert(matrix::I() * matrix::S2(1.0, 2.0) == matrix::S2(1.0, 2.0));
    static_assert(matrix::I() * matrix::S3(1.0, 2.0, 3.0) == matrix::S3(1.0, 2.0, 3.0));

    ASSERT_TRUE(matrix::I() * matrix::S2(1.0, 2.0) == matrix::S2(1.0, 2.0));
    ASSERT_TRUE(matrix::I() * matrix::S3(1.0, 2.0, 3.0) == matrix::S3(1.0, 2.0, 3.0));
}

TEST(matrix, translate_vec)
{
    static_assert(std::is_same_v<decltype(matrix::T2(4.0, 6.0) * vec2(1.0, 2.0)), vec2>);
    static_assert(std::is_same_v<decltype(matrix::T2(4.0, 6.0) * vec3(1.0, 2.0, 3.0)), vec3>);
    static_assert(std::is_same_v<decltype(matrix::T3(4.0, 6.0, 8.0) * vec2(1.0, 2.0)), vec2>);
    static_assert(std::is_same_v<decltype(matrix::T3(4.0, 6.0, 8.0) * vec3(1.0, 2.0, 3.0)), vec3>);

    static_assert(matrix::T2(4.0, 6.0) * vec2(1.0, 2.0) == vec2(1.0, 2.0));
    static_assert(matrix::T2(4.0, 6.0) * vec3(1.0, 2.0, 3.0) == vec3(1.0, 2.0, 3.0));
    static_assert(matrix::T3(4.0, 6.0, 8.0) * vec2(1.0, 2.0) == vec2(1.0, 2.0));
    static_assert(matrix::T3(4.0, 6.0, 8.0) * vec3(1.0, 2.0, 3.0) == vec3(1.0, 2.0, 3.0));

    ASSERT_TRUE(matrix::T2(4.0, 6.0) * vec2(1.0, 2.0) == vec2(1.0, 2.0));
    ASSERT_TRUE(matrix::T2(4.0, 6.0) * vec3(1.0, 2.0, 3.0) == vec3(1.0, 2.0, 3.0));
    ASSERT_TRUE(matrix::T3(4.0, 6.0, 8.0) * vec2(1.0, 2.0) == vec2(1.0, 2.0));
    ASSERT_TRUE(matrix::T3(4.0, 6.0, 8.0) * vec3(1.0, 2.0, 3.0) == vec3(1.0, 2.0, 3.0));
}

TEST(matrix, translate_point)
{
    static_assert(std::is_same_v<decltype(matrix::T2(4.0, 6.0) * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype(matrix::T2(4.0, 6.0) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype(matrix::T3(4.0, 6.0, 8.0) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype(matrix::T3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)), point3>);

    static_assert(matrix::T2(4.0, 6.0) * point2(1.0, 2.0) == point2(5.0, 8.0));
    static_assert(matrix::T2(4.0, 6.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 3.0));
    static_assert(matrix::T3(4.0, 6.0, 8.0) * point2(1.0, 2.0) == point3(5.0, 8.0, 8.0));
    static_assert(matrix::T3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 11.0));

    ASSERT_TRUE(matrix::T2(4.0, 6.0) * point2(1.0, 2.0) == point2(5.0, 8.0));
    ASSERT_TRUE(matrix::T2(4.0, 6.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 3.0));
    ASSERT_TRUE(matrix::T3(4.0, 6.0, 8.0) * point2(1.0, 2.0) == point3(5.0, 8.0, 8.0));
    ASSERT_TRUE(matrix::T3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 11.0));
}

TEST(matrix, translate_scale_point)
{
    static_assert(std::is_same_v<decltype(matrix::T2(-3, -4) * (matrix::S2(4.0, 6.0) * point2(1.0, 2.0))), point2>);
    static_assert(std::is_same_v<decltype(matrix::T2(-3, -4) * (matrix::S2(4.0, 6.0) * point3(1.0, 2.0, 3.0))), point3>);
    static_assert(std::is_same_v<decltype(matrix::T2(-3, -4) * (matrix::S3(4.0, 6.0, 8.0) * point2(1.0, 2.0))), point2>);
    static_assert(std::is_same_v<decltype(matrix::T2(-3, -4) * (matrix::S3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0))), point3>);
    static_assert(std::is_same_v<decltype(matrix::T3(-3, -4, -5) * (matrix::S2(4.0, 6.0) * point2(1.0, 2.0))), point3>);
    static_assert(std::is_same_v<decltype(matrix::T3(-3, -4, -5) * (matrix::S2(4.0, 6.0) * point3(1.0, 2.0, 3.0))), point3>);
    static_assert(std::is_same_v<decltype(matrix::T3(-3, -4, -5) * (matrix::S3(4.0, 6.0, 8.0) * point2(1.0, 2.0))), point3>);
    static_assert(std::is_same_v<decltype(matrix::T3(-3, -4, -5) * (matrix::S3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0))), point3>);

    static_assert(matrix::T2(-3, -4) * (matrix::S2(4.0, 6.0) * point2(1.0, 2.0)) == point2(1.0, 8.0));
    static_assert(matrix::T2(-3, -4) * (matrix::S2(4.0, 6.0) * point3(1.0, 2.0, 3.0)) == point3(1.0, 8.0, 3.0));
    static_assert(matrix::T2(-3, -4) * (matrix::S3(4.0, 6.0, 8.0) * point2(1.0, 2.0)) == point2(1.0, 8.0));
    static_assert(matrix::T2(-3, -4) * (matrix::S3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)) == point3(1, 8.0, 24.0));
    static_assert(matrix::T3(-3, -4, -5) * (matrix::S2(4.0, 6.0) * point2(1.0, 2.0)) == point3(1.0, 8.0, -5));
    static_assert(matrix::T3(-3, -4, -5) * (matrix::S2(4.0, 6.0) * point3(1.0, 2.0, 3.0)) == point3(1.0, 8.0, -2.0));
    static_assert(matrix::T3(-3, -4, -5) * (matrix::S3(4.0, 6.0, 8.0) * point2(1.0, 2.0)) == point3(1.0, 8.0, -5));
    static_assert(matrix::T3(-3, -4, -5) * (matrix::S3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)) == point3(1, 8.0, 19.0));

    ASSERT_TRUE(matrix::T2(-3, -4) * (matrix::S2(4.0, 6.0) * point2(1.0, 2.0)) == point2(1.0, 8.0));
    ASSERT_TRUE(matrix::T2(-3, -4) * (matrix::S2(4.0, 6.0) * point3(1.0, 2.0, 3.0)) == point3(1.0, 8.0, 3.0));
    ASSERT_TRUE(matrix::T2(-3, -4) * (matrix::S3(4.0, 6.0, 8.0) * point2(1.0, 2.0)) == point2(1.0, 8.0));
    ASSERT_TRUE(matrix::T2(-3, -4) * (matrix::S3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)) == point3(1, 8.0, 24.0));
    ASSERT_TRUE(matrix::T3(-3, -4, -5) * (matrix::S2(4.0, 6.0) * point2(1.0, 2.0)) == point3(1.0, 8.0, -5));
    ASSERT_TRUE(matrix::T3(-3, -4, -5) * (matrix::S2(4.0, 6.0) * point3(1.0, 2.0, 3.0)) == point3(1.0, 8.0, -2.0));
    ASSERT_TRUE(matrix::T3(-3, -4, -5) * (matrix::S3(4.0, 6.0, 8.0) * point2(1.0, 2.0)) == point3(1.0, 8.0, -5));
    ASSERT_TRUE(matrix::T3(-3, -4, -5) * (matrix::S3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)) == point3(1, 8.0, 19.0));

    static_assert(std::is_same_v<decltype((matrix::T2(-3, -4) * matrix::S2(4.0, 6.0)) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype((matrix::T2(-3, -4) * matrix::S2(4.0, 6.0)) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype((matrix::T2(-3, -4) * matrix::S3(4.0, 6.0, 8.0)) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype((matrix::T2(-3, -4) * matrix::S3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype((matrix::T3(-3, -4, -5) * matrix::S2(4.0, 6.0)) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype((matrix::T3(-3, -4, -5) * matrix::S2(4.0, 6.0)) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype((matrix::T3(-3, -4, -5) * matrix::S3(4.0, 6.0, 8.0)) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype((matrix::T3(-3, -4, -5) * matrix::S3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0)), point3>);

    static_assert((matrix::T2(-3, -4) * matrix::S2(4.0, 6.0)) * point3(1.0, 2.0, 3.0) == point3(1.0, 8.0, 3.0));
    static_assert((matrix::T2(-3, -4) * matrix::S3(4.0, 6.0, 8.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, 0.0));
    static_assert((matrix::T2(-3, -4) * matrix::S3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0) == point3(1, 8.0, 24.0));
    static_assert((matrix::T2(-3, -4) * matrix::S2(4.0, 6.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, 0.0));
    static_assert((matrix::T3(-3, -4, -5) * matrix::S2(4.0, 6.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, -5));
    static_assert((matrix::T3(-3, -4, -5) * matrix::S2(4.0, 6.0)) * point3(1.0, 2.0, 3.0) == point3(1.0, 8.0, -2.0));
    static_assert((matrix::T3(-3, -4, -5) * matrix::S3(4.0, 6.0, 8.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, -5));
    static_assert((matrix::T3(-3, -4, -5) * matrix::S3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0) == point3(1, 8.0, 19.0));

    ASSERT_TRUE((matrix::T2(-3, -4) * matrix::S2(4.0, 6.0)) * point3(1.0, 2.0, 3.0) == point3(1.0, 8.0, 3.0));
    ASSERT_TRUE((matrix::T2(-3, -4) * matrix::S3(4.0, 6.0, 8.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, 0.0));
    ASSERT_TRUE((matrix::T2(-3, -4) * matrix::S3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0) == point3(1, 8.0, 24.0));
    ASSERT_TRUE((matrix::T2(-3, -4) * matrix::S2(4.0, 6.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, 0.0));
    ASSERT_TRUE((matrix::T3(-3, -4, -5) * matrix::S2(4.0, 6.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, -5));
    ASSERT_TRUE((matrix::T3(-3, -4, -5) * matrix::S2(4.0, 6.0)) * point3(1.0, 2.0, 3.0) == point3(1.0, 8.0, -2.0));
    ASSERT_TRUE((matrix::T3(-3, -4, -5) * matrix::S3(4.0, 6.0, 8.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, -5));
    ASSERT_TRUE((matrix::T3(-3, -4, -5) * matrix::S3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0) == point3(1, 8.0, 19.0));
}

TEST(matrix, translate_identity)
{
    static_assert(std::is_same_v<decltype(matrix::T2(1.0, 2.0) * matrix::I()), matrix::T2>);
    static_assert(std::is_same_v<decltype(matrix::T3(1.0, 2.0, 3.0) * matrix::I()), matrix::T3>);

    static_assert(matrix::T2(1.0, 2.0) * matrix::I() == matrix::T2(1.0, 2.0));
    static_assert(matrix::T3(1.0, 2.0, 3.0) * matrix::I() == matrix::T3(1.0, 2.0, 3.0));

    ASSERT_TRUE(matrix::T2(1.0, 2.0) * matrix::I() == matrix::T2(1.0, 2.0));
    ASSERT_TRUE(matrix::T3(1.0, 2.0, 3.0) * matrix::I() == matrix::T3(1.0, 2.0, 3.0));
}

TEST(matrix, translate_translate)
{
    static_assert(std::is_same_v<decltype(matrix::T2(4.0, 6.0) * matrix::T2(1.0, 2.0)), matrix::T2>);
    static_assert(std::is_same_v<decltype(matrix::T2(4.0, 6.0) * matrix::T3(1.0, 2.0, 3.0)), matrix::T3>);
    static_assert(std::is_same_v<decltype(matrix::T3(4.0, 6.0, 8.0) * matrix::T2(1.0, 2.0)), matrix::T3>);
    static_assert(std::is_same_v<decltype(matrix::T3(4.0, 6.0, 8.0) * matrix::T3(1.0, 2.0, 3.0)), matrix::T3>);

    static_assert(matrix::T2(4.0, 6.0) * matrix::T2(1.0, 2.0) == matrix::T2(5.0, 8.0));
    static_assert(matrix::T2(4.0, 6.0) * matrix::T3(1.0, 2.0, 3.0) == matrix::T3(5.0, 8.0, 3.0));
    static_assert(matrix::T3(4.0, 6.0, 8.0) * matrix::T2(1.0, 2.0) == matrix::T3(5.0, 8.0, 8.0));
    static_assert(matrix::T3(4.0, 6.0, 8.0) * matrix::T3(1.0, 2.0, 3.0) == matrix::T3(5.0, 8.0, 11.0));

    ASSERT_TRUE(matrix::T2(4.0, 6.0) * matrix::T2(1.0, 2.0) == matrix::T2(5.0, 8.0));
    ASSERT_TRUE(matrix::T2(4.0, 6.0) * matrix::T3(1.0, 2.0, 3.0) == matrix::T3(5.0, 8.0, 3.0));
    ASSERT_TRUE(matrix::T3(4.0, 6.0, 8.0) * matrix::T2(1.0, 2.0) == matrix::T3(5.0, 8.0, 8.0));
    ASSERT_TRUE(matrix::T3(4.0, 6.0, 8.0) * matrix::T3(1.0, 2.0, 3.0) == matrix::T3(5.0, 8.0, 11.0));
}

TEST(matrix, scale_vec)
{
    static_assert(std::is_same_v<decltype(matrix::S2(4.0, 6.0) * vec2(1.0, 2.0)), vec2>);
    static_assert(std::is_same_v<decltype(matrix::S2(4.0, 6.0) * vec3(1.0, 2.0, 3.0)), vec3>);
    static_assert(std::is_same_v<decltype(matrix::S3(4.0, 6.0, 8.0) * vec2(1.0, 2.0)), vec2>);
    static_assert(std::is_same_v<decltype(matrix::S3(4.0, 6.0, 8.0) * vec3(1.0, 2.0, 3.0)), vec3>);

    static_assert(matrix::S2(4.0, 6.0) * vec2(1.0, 2.0) == vec2(4.0, 12.0));
    static_assert(matrix::S2(4.0, 6.0) * vec3(1.0, 2.0, 3.0) == vec3(4.0, 12.0, 3.0));
    static_assert(matrix::S3(4.0, 6.0, 8.0) * vec2(1.0, 2.0) == vec2(4.0, 12.0));
    static_assert(matrix::S3(4.0, 6.0, 8.0) * vec3(1.0, 2.0, 3.0) == vec3(4, 12.0, 24.0));

    ASSERT_TRUE(matrix::S2(4.0, 6.0) * vec2(1.0, 2.0) == vec2(4.0, 12.0));
    ASSERT_TRUE(matrix::S2(4.0, 6.0) * vec3(1.0, 2.0, 3.0) == vec3(4.0, 12.0, 3.0));
    ASSERT_TRUE(matrix::S3(4.0, 6.0, 8.0) * vec2(1.0, 2.0) == vec2(4.0, 12.0));
    ASSERT_TRUE(matrix::S3(4.0, 6.0, 8.0) * vec3(1.0, 2.0, 3.0) == vec3(4, 12.0, 24.0));
}

TEST(matrix, scale_point)
{
    static_assert(std::is_same_v<decltype(matrix::S2(4.0, 6.0) * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype(matrix::S2(4.0, 6.0) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype(matrix::S3(4.0, 6.0, 8.0) * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype(matrix::S3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)), point3>);

    static_assert(matrix::S2(4.0, 6.0) * point2(1.0, 2.0) == point2(4.0, 12.0));
    static_assert(matrix::S2(4.0, 6.0) * point3(1.0, 2.0, 3.0) == point3(4.0, 12.0, 3.0));
    static_assert(matrix::S3(4.0, 6.0, 8.0) * point2(1.0, 2.0) == point2(4.0, 12.0));
    static_assert(matrix::S3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0) == point3(4, 12.0, 24.0));

    ASSERT_TRUE(matrix::S2(4.0, 6.0) * point2(1.0, 2.0) == point2(4.0, 12.0));
    ASSERT_TRUE(matrix::S2(4.0, 6.0) * point3(1.0, 2.0, 3.0) == point3(4.0, 12.0, 3.0));
    ASSERT_TRUE(matrix::S3(4.0, 6.0, 8.0) * point2(1.0, 2.0) == point2(4.0, 12.0));
    ASSERT_TRUE(matrix::S3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0) == point3(4, 12.0, 24.0));
}

TEST(matrix, scale_identity)
{
    static_assert(std::is_same_v<decltype(matrix::S2(1.0, 2.0) * matrix::I()), matrix::S2>);
    static_assert(std::is_same_v<decltype(matrix::S3(1.0, 2.0, 3.0) * matrix::I()), matrix::S3>);

    static_assert(matrix::S2(1.0, 2.0) * matrix::I() == matrix::S2(1.0, 2.0));
    static_assert(matrix::S3(1.0, 2.0, 3.0) * matrix::I() == matrix::S3(1.0, 2.0, 3.0));

    ASSERT_TRUE(matrix::S2(1.0, 2.0) * matrix::I() == matrix::S2(1.0, 2.0));
    ASSERT_TRUE(matrix::S3(1.0, 2.0, 3.0) * matrix::I() == matrix::S3(1.0, 2.0, 3.0));
}

TEST(matrix, scale_scale)
{
    static_assert(std::is_same_v<decltype(matrix::S2(4.0, 6.0) * matrix::S2(1.0, 2.0)), matrix::S2>);
    static_assert(std::is_same_v<decltype(matrix::S2(4.0, 6.0) * matrix::S3(1.0, 2.0, 3.0)), matrix::S3>);
    static_assert(std::is_same_v<decltype(matrix::S3(4.0, 6.0, 8.0) * matrix::S2(1.0, 2.0)), matrix::S3>);
    static_assert(std::is_same_v<decltype(matrix::S3(4.0, 6.0, 8.0) * matrix::S3(1.0, 2.0, 3.0)), matrix::S3>);

    static_assert(matrix::S2(4.0, 6.0) * matrix::S2(1.0, 2.0) == matrix::S2(4.0, 12.0));
    static_assert(matrix::S2(4.0, 6.0) * matrix::S3(1.0, 2.0, 3.0) == matrix::S3(4.0, 12.0, 3.0));
    static_assert(matrix::S3(4.0, 6.0, 8.0) * matrix::S2(1.0, 2.0) == matrix::S3(4.0, 12.0, 8.0));
    static_assert(matrix::S3(4.0, 6.0, 8.0) * matrix::S3(1.0, 2.0, 3.0) == matrix::S3(4, 12.0, 24.0));

    ASSERT_TRUE(matrix::S2(4.0, 6.0) * matrix::S2(1.0, 2.0) == matrix::S2(4.0, 12.0));
    ASSERT_TRUE(matrix::S2(4.0, 6.0) * matrix::S3(1.0, 2.0, 3.0) == matrix::S3(4.0, 12.0, 3.0));
    ASSERT_TRUE(matrix::S3(4.0, 6.0, 8.0) * matrix::S2(1.0, 2.0) == matrix::S3(4.0, 12.0, 8.0));
    ASSERT_TRUE(matrix::S3(4.0, 6.0, 8.0) * matrix::S3(1.0, 2.0, 3.0) == matrix::S3(4, 12.0, 24.0));
}
