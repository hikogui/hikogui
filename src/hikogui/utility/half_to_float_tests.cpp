// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "half_to_float.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include <algorithm>

namespace half_to_float_tests {

[[nodiscard]] float half_to_float_avx2(uint16_t v) noexcept
{
    auto v_ = std::array<uint16_t,4>{v, v, v, v};
    auto r = hi::half_to_float_avx2(v_);
    return std::get<0>(r);
}

}

TEST(half_to_float, generic)
{
    ASSERT_EQ(hi::half_to_float_generic(0x7C00), std::numeric_limits<float>::infinity());

    // Largest normal.
    ASSERT_EQ(hi::half_to_float_generic(0x7bff), 65504.0f);

    // (+) 2^15
    ASSERT_EQ(hi::half_to_float_generic(0x7800), 32768.0f);

    // (+) 1/2 max normal
    ASSERT_EQ(hi::half_to_float_generic(0x77FF), 32752.0f);

    // (+) approx pi
    ASSERT_EQ(hi::half_to_float_generic(0x4248), 3.140625f);

    // (+) approx e
    ASSERT_EQ(hi::half_to_float_generic(0x4170), 2.71875f);

    // (+) two
    ASSERT_EQ(hi::half_to_float_generic(0x4000), 2.0f);

    // (+) smallest > 1.0
    ASSERT_EQ(hi::half_to_float_generic(0x3C01), 1.0009766f);

    // (+) one
    ASSERT_EQ(hi::half_to_float_generic(0x3C00), 1.0f);

    // (+) largest < 1.0
    ASSERT_EQ(hi::half_to_float_generic(0x3BFF), 0.9995117f);

    // (+) approx 2/3
    ASSERT_EQ(hi::half_to_float_generic(0x3956), 0.6669922f);

    // (+) approx 1/3
    ASSERT_EQ(hi::half_to_float_generic(0x3555), 0.33325195f);

    // (+) smallest normal
    ASSERT_EQ(hi::half_to_float_generic(0x0400), 6.1035156E-05f);

    // (+) largest subnormal
    ASSERT_EQ(hi::half_to_float_generic(0x03FF), 6.09755516e-05f);

    // (+) middle subnormal
    ASSERT_EQ(hi::half_to_float_generic(0x0200), 3.05175781e-05f);

    // (+) just below mid-subnormal
    ASSERT_EQ(hi::half_to_float_generic(0x01FF), 3.04579735e-05f);

    // (+) smallest subnormal
    ASSERT_EQ(hi::half_to_float_generic(0x0001), 5.96046448e-08f);

    // (+) positive zero
    ASSERT_EQ(hi::half_to_float_generic(0x0000), 0.0f);

    // (-) negative zero
    ASSERT_EQ(hi::half_to_float_generic(0x8000), -0.0f);

    // (-) highest subnormal
    ASSERT_EQ(hi::half_to_float_generic(0x8001), -5.96046448e-08f);

    // (-) just above mid-subnormal
    ASSERT_EQ(hi::half_to_float_generic(0x81FF), -3.04579735e-05f);

    // (-) middle subnormal
    ASSERT_EQ(hi::half_to_float_generic(0x8200), -3.05175781e-05f);

    // (-) lowest subnormal
    ASSERT_EQ(hi::half_to_float_generic(0x83FF), -6.09755516e-05f);

    // (-) highest normal
    ASSERT_EQ(hi::half_to_float_generic(0x8400), -6.1035156E-05f);

    // (-) approx -1/3
    ASSERT_EQ(hi::half_to_float_generic(0xB555), -0.33325195f);

    // (-) approx -2/3
    ASSERT_EQ(hi::half_to_float_generic(0xB956), -0.6669922f);

    // (-) lowest > -1.0
    ASSERT_EQ(hi::half_to_float_generic(0xBBFF), -0.9995117f);

    // (-) minus one
    ASSERT_EQ(hi::half_to_float_generic(0xBC00), -1.0f);

    // (-) highest < -1.0
    ASSERT_EQ(hi::half_to_float_generic(0xBC01), -1.0009766f);

    // (-) minus two
    ASSERT_EQ(hi::half_to_float_generic(0xC000), -2.0f);

    // (-) approx e
    ASSERT_EQ(hi::half_to_float_generic(0xC170), -2.71875f);

    // (-) approx pi
    ASSERT_EQ(hi::half_to_float_generic(0xC248), -3.140625f);

    // (-) 1/2 lowest normal
    ASSERT_EQ(hi::half_to_float_generic(0xF7FF), -32752.0f);

    // (-) 2^15
    ASSERT_EQ(hi::half_to_float_generic(0xF800), -32768.0f);

    // (-) lowest normal
    ASSERT_EQ(hi::half_to_float_generic(0xFBFF), -65504.0f);
}

#if HI_HAS_F16C
TEST(half_to_float, f16c)
{
    ASSERT_EQ(hi::half_to_float_f16c(0x7C00), std::numeric_limits<float>::infinity());

    // Largest normal.
    ASSERT_EQ(hi::half_to_float_f16c(0x7bff), 65504.0f);

    // (+) 2^15
    ASSERT_EQ(hi::half_to_float_f16c(0x7800), 32768.0f);

    // (+) 1/2 max normal
    ASSERT_EQ(hi::half_to_float_f16c(0x77FF), 32752.0f);

    // (+) approx pi
    ASSERT_EQ(hi::half_to_float_f16c(0x4248), 3.140625f);

    // (+) approx e
    ASSERT_EQ(hi::half_to_float_f16c(0x4170), 2.71875f);

    // (+) two
    ASSERT_EQ(hi::half_to_float_f16c(0x4000), 2.0f);

    // (+) smallest > 1.0
    ASSERT_EQ(hi::half_to_float_f16c(0x3C01), 1.0009766f);

    // (+) one
    ASSERT_EQ(hi::half_to_float_f16c(0x3C00), 1.0f);

    // (+) largest < 1.0
    ASSERT_EQ(hi::half_to_float_f16c(0x3BFF), 0.9995117f);

    // (+) approx 2/3
    ASSERT_EQ(hi::half_to_float_f16c(0x3956), 0.6669922f);

    // (+) approx 1/3
    ASSERT_EQ(hi::half_to_float_f16c(0x3555), 0.33325195f);

    // (+) smallest normal
    ASSERT_EQ(hi::half_to_float_f16c(0x0400), 6.1035156E-05f);

    // (+) largest subnormal
    ASSERT_EQ(hi::half_to_float_f16c(0x03FF), 6.09755516e-05f);

    // (+) middle subnormal
    ASSERT_EQ(hi::half_to_float_f16c(0x0200), 3.05175781e-05f);

    // (+) just below mid-subnormal
    ASSERT_EQ(hi::half_to_float_f16c(0x01FF), 3.04579735e-05f);

    // (+) smallest subnormal
    ASSERT_EQ(hi::half_to_float_f16c(0x0001), 5.96046448e-08f);

    // (+) positive zero
    ASSERT_EQ(hi::half_to_float_f16c(0x0000), 0.0f);

    // (-) negative zero
    ASSERT_EQ(hi::half_to_float_f16c(0x8000), -0.0f);

    // (-) highest subnormal
    ASSERT_EQ(hi::half_to_float_f16c(0x8001), -5.96046448e-08f);

    // (-) just above mid-subnormal
    ASSERT_EQ(hi::half_to_float_f16c(0x81FF), -3.04579735e-05f);

    // (-) middle subnormal
    ASSERT_EQ(hi::half_to_float_f16c(0x8200), -3.05175781e-05f);

    // (-) lowest subnormal
    ASSERT_EQ(hi::half_to_float_f16c(0x83FF), -6.09755516e-05f);

    // (-) highest normal
    ASSERT_EQ(hi::half_to_float_f16c(0x8400), -6.1035156E-05f);

    // (-) approx -1/3
    ASSERT_EQ(hi::half_to_float_f16c(0xB555), -0.33325195f);

    // (-) approx -2/3
    ASSERT_EQ(hi::half_to_float_f16c(0xB956), -0.6669922f);

    // (-) lowest > -1.0
    ASSERT_EQ(hi::half_to_float_f16c(0xBBFF), -0.9995117f);

    // (-) minus one
    ASSERT_EQ(hi::half_to_float_f16c(0xBC00), -1.0f);

    // (-) highest < -1.0
    ASSERT_EQ(hi::half_to_float_f16c(0xBC01), -1.0009766f);

    // (-) minus two
    ASSERT_EQ(hi::half_to_float_f16c(0xC000), -2.0f);

    // (-) approx e
    ASSERT_EQ(hi::half_to_float_f16c(0xC170), -2.71875f);

    // (-) approx pi
    ASSERT_EQ(hi::half_to_float_f16c(0xC248), -3.140625f);

    // (-) 1/2 lowest normal
    ASSERT_EQ(hi::half_to_float_f16c(0xF7FF), -32752.0f);

    // (-) 2^15
    ASSERT_EQ(hi::half_to_float_f16c(0xF800), -32768.0f);

    // (-) lowest normal
    ASSERT_EQ(hi::half_to_float_f16c(0xFBFF), -65504.0f);
}
#endif

#if HI_HAS_AVX2
TEST(half_to_float, avx2)
{
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x7C00), std::numeric_limits<float>::infinity());

    // Largest normal.
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x7bff), 65504.0f);

    // (+) 2^15
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x7800), 32768.0f);

    // (+) 1/2 max normal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x77FF), 32752.0f);

    // (+) approx pi
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x4248), 3.140625f);

    // (+) approx e
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x4170), 2.71875f);

    // (+) two
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x4000), 2.0f);

    // (+) smallest > 1.0
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x3C01), 1.0009766f);

    // (+) one
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x3C00), 1.0f);

    // (+) largest < 1.0
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x3BFF), 0.9995117f);

    // (+) approx 2/3
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x3956), 0.6669922f);

    // (+) approx 1/3
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x3555), 0.33325195f);

    // (+) smallest normal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x0400), 6.1035156E-05f);

    // (+) largest subnormal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x03FF), 6.09755516e-05f);

    // (+) middle subnormal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x0200), 3.05175781e-05f);

    // (+) just below mid-subnormal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x01FF), 3.04579735e-05f);

    // (+) smallest subnormal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x0001), 5.96046448e-08f);

    // (+) positive zero
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x0000), 0.0f);

    // (-) negative zero
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x8000), -0.0f);

    // (-) highest subnormal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x8001), -5.96046448e-08f);

    // (-) just above mid-subnormal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x81FF), -3.04579735e-05f);

    // (-) middle subnormal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x8200), -3.05175781e-05f);

    // (-) lowest subnormal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x83FF), -6.09755516e-05f);

    // (-) highest normal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0x8400), -6.1035156E-05f);

    // (-) approx -1/3
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xB555), -0.33325195f);

    // (-) approx -2/3
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xB956), -0.6669922f);

    // (-) lowest > -1.0
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xBBFF), -0.9995117f);

    // (-) minus one
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xBC00), -1.0f);

    // (-) highest < -1.0
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xBC01), -1.0009766f);

    // (-) minus two
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xC000), -2.0f);

    // (-) approx e
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xC170), -2.71875f);

    // (-) approx pi
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xC248), -3.140625f);

    // (-) 1/2 lowest normal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xF7FF), -32752.0f);

    // (-) 2^15
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xF800), -32768.0f);

    // (-) lowest normal
    ASSERT_EQ(half_to_float_tests::half_to_float_avx2(0xFBFF), -65504.0f);
}
#endif
