// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "native_i32x4_sse2.hpp"
#include "simd_test_utility.hpp"

hi_warning_push();
// C26474: Don't cast between pointer types when the conversion could be implicit (type.1).
// For the test we need to do this explicit.
hi_warning_ignore_msvc(26474);

using S = hi::native_simd<int32_t, 4>;
using A = S::array_type;

TEST(native_i32x4, construct)
{
    {
        auto expected = A{0, 0, 0, 0};
        ASSERT_EQ(static_cast<A>(S{}), expected);
    }

    {
        auto expected = A{1, 0, 0, 0};
        ASSERT_EQ(static_cast<A>(S{1}), expected);
    }

    {
        auto expected = A{1, 2, 3, 4};
        ASSERT_EQ(static_cast<A>(S{1, 2, 3, 4}), expected);
    }

    {
        auto expected = A{4, 4, 4, 4};
        ASSERT_EQ(static_cast<A>(S::broadcast(4)), expected);
    }

    {
        auto from = A{1, 2, 3, 4};
        auto expected = A{1, 2, 3, 4};
        ASSERT_EQ(static_cast<A>(S{from}), expected);
    }

    {
        auto from = A{1, 2, 3, 4};
        auto expected = A{1, 2, 3, 4};
        ASSERT_EQ(static_cast<A>(S{from.data()}), expected);
    }

    {
        auto from = A{1, 2, 3, 4};
        auto expected = A{1, 2, 3, 4};
        ASSERT_EQ(static_cast<A>(S{static_cast<void *>(from.data())}), expected);
    }

    {
        auto a = A{1, 2, 3, 4};
        auto from = std::span(a.data(), a.size());
        auto expected = A{1, 2, 3, 4};
        ASSERT_EQ(static_cast<A>(S{from}), expected);
    }
}

TEST(native_i32x4, conversion)
{
    auto a = S{1, 2, 3, 4};
    auto expected = A{1, 2, 3, 4};

    {
        auto result = A{};
        a.store(result);
        ASSERT_EQ(result, expected);
    }

    {
        auto result = A{};
        auto result_span = std::span(result.data(), result.size());
        a.store(result_span);
        ASSERT_EQ(result, expected);
    }

    {
        auto result = A{};
        a.store(result.data());
        ASSERT_EQ(result, expected);
    }

    {
        auto result = A{};
        a.store(static_cast<void *>(result.data()));
        ASSERT_EQ(result, expected);
    }
}

TEST(native_i32x4, compare)
{
    HI_ASSERT_SIMD_EQ(S(1, 2, 0, -4), S(1, 2, 0, -4));
    HI_ASSERT_SIMD_NE(S(2, 2, 0, -4), S(1, 2, 0, -4));
    HI_ASSERT_SIMD_NE(S(2, 3, 0, -5), S(1, 2, 0, -4));

    ASSERT_EQ((S(1, 2, 0, -4) == S(1, 2, 42, -4)).mask(), 0b1011);
    ASSERT_EQ((S(2, 2, 0, -4) == S(1, 2, 42, -4)).mask(), 0b1010);

    ASSERT_EQ((S(1, 2, 0, -4) != S(1, 2, 42, -4)).mask(), 0b0100);
    ASSERT_EQ((S(2, 2, 0, -4) != S(1, 2, 42, -4)).mask(), 0b0101);

    ASSERT_EQ((S(1, 2, -3, 4) < S(2, 2, 2, 2)).mask(), 0b0101);
    ASSERT_EQ((S(1, 2, -3, 4) <= S(2, 2, 2, 2)).mask(), 0b0111);
    ASSERT_EQ((S(1, 2, -3, 4) > S(2, 2, 2, 2)).mask(), 0b1000);
    ASSERT_EQ((S(1, 2, -3, 4) >= S(2, 2, 2, 2)).mask(), 0b1010);
}

TEST(native_i32x4, math)
{
    HI_ASSERT_SIMD_EQ(-S(0, 2, 3, 42), S(0, -2, -3, -42));
    HI_ASSERT_SIMD_EQ(+S(0, 2, 3, 42), S(0, 2, 3, 42));
    HI_ASSERT_SIMD_EQ(S(0, 2, 3, 42) + S(1, 4, -3, 2), S(1, 6, 0, 44));
    HI_ASSERT_SIMD_EQ(S(0, 2, 3, 42) - S(1, 4, -3, 2), S(-1, -2, 6, 40));
    HI_ASSERT_SIMD_EQ(S(0, 2, 3, 42) * S(1, 4, -3, 2), S(0, 8, -9, 84));

    HI_ASSERT_SIMD_EQ(min(S(0, 2, 0, 42), S(1, 0, -3, 1)), S(0, 0, -3, 1));
    HI_ASSERT_SIMD_EQ(max(S(0, 2, 0, 42), S(1, 0, -3, 1)), S(1, 2, 0, 42));
    HI_ASSERT_SIMD_EQ(abs(S(0, 2, -3, -3)), S(0, 2, 3, 3));
}

TEST(native_i32x4, bit_wise)
{
    HI_ASSERT_SIMD_EQ(S(0, 2, -3, 42) >> 1, S(0, 1, -2, 21));
    HI_ASSERT_SIMD_EQ(S(0, 2, -3, 42) << 1, S(0, 4, -6, 84));
    HI_ASSERT_SIMD_EQ(S(0, 2, 0, 42) | S(1, 0, -3, 0), S(1, 2, -3, 42));
    HI_ASSERT_SIMD_EQ(S(1, 2, 3, 42) & S::from_mask(0b1010), S(0, 2, 0, 42));
    HI_ASSERT_SIMD_EQ(S::from_mask(0b0011) ^ S::from_mask(0b1010), S::from_mask(0b1001));
    HI_ASSERT_SIMD_EQ(~S::from_mask(0b1010), S::from_mask(0b0101));

    HI_ASSERT_SIMD_EQ(not_and(S::from_mask(0b1010), S(1, 2, 3, 42)), S(1, 0, 3, 0));
}

TEST(native_i32x4, access)
{
    auto tmp = S(1, 2, 3, 4);

    ASSERT_EQ(get<0>(tmp), 1);
    ASSERT_EQ(get<1>(tmp), 2);
    ASSERT_EQ(get<2>(tmp), 3);
    ASSERT_EQ(get<3>(tmp), 4);

    HI_ASSERT_SIMD_EQ(insert<0>(tmp, 42), S(42, 2, 3, 4));
    HI_ASSERT_SIMD_EQ(insert<1>(tmp, 42), S(1, 42, 3, 4));
    HI_ASSERT_SIMD_EQ(insert<2>(tmp, 42), S(1, 2, 42, 4));
    HI_ASSERT_SIMD_EQ(insert<3>(tmp, 42), S(1, 2, 3, 42));

    HI_ASSERT_SIMD_EQ(set_zero<0b0000>(tmp), S(1, 2, 3, 4));
    HI_ASSERT_SIMD_EQ(set_zero<0b0001>(tmp), S(0, 2, 3, 4));
    HI_ASSERT_SIMD_EQ(set_zero<0b0010>(tmp), S(1, 0, 3, 4));
    HI_ASSERT_SIMD_EQ(set_zero<0b0100>(tmp), S(1, 2, 0, 4));
    HI_ASSERT_SIMD_EQ(set_zero<0b1000>(tmp), S(1, 2, 3, 0));
    HI_ASSERT_SIMD_EQ(set_zero<0b1001>(tmp), S(0, 2, 3, 0));
    HI_ASSERT_SIMD_EQ(set_zero<0b1111>(tmp), S(0, 0, 0, 0));
}

TEST(native_i32x4, blend)
{
    auto a = S(1, 2, 3, 4);
    auto b = S(42, 43, 44, 45);

    HI_ASSERT_SIMD_EQ(blend<0b0000>(a, b), S(1, 2, 3, 4));
    HI_ASSERT_SIMD_EQ(blend<0b0001>(a, b), S(42, 2, 3, 4));
    HI_ASSERT_SIMD_EQ(blend<0b0010>(a, b), S(1, 43, 3, 4));
    HI_ASSERT_SIMD_EQ(blend<0b0100>(a, b), S(1, 2, 44, 4));
    HI_ASSERT_SIMD_EQ(blend<0b1000>(a, b), S(1, 2, 3, 45));
    HI_ASSERT_SIMD_EQ(blend<0b1001>(a, b), S(42, 2, 3, 45));
    HI_ASSERT_SIMD_EQ(blend<0b1111>(a, b), S(42, 43, 44, 45));
}

TEST(native_i32x4, permute)
{
    auto tmp = S(2, 3, 4, 5);

    HI_ASSERT_SIMD_EQ(permute<"abcd">(tmp), S(2, 3, 4, 5));
    HI_ASSERT_SIMD_EQ(permute<"xyzw">(tmp), S(2, 3, 4, 5));
    HI_ASSERT_SIMD_EQ(permute<"0000">(tmp), S(2, 3, 4, 5));

    HI_ASSERT_SIMD_EQ(permute<"dcba">(tmp), S(5, 4, 3, 2));
    HI_ASSERT_SIMD_EQ(permute<"wzyx">(tmp), S(5, 4, 3, 2));

    HI_ASSERT_SIMD_EQ(permute<"axcd">(tmp), S(2, 2, 4, 5));
    HI_ASSERT_SIMD_EQ(permute<"aycd">(tmp), S(2, 3, 4, 5));
    HI_ASSERT_SIMD_EQ(permute<"azcd">(tmp), S(2, 4, 4, 5));
    HI_ASSERT_SIMD_EQ(permute<"awcd">(tmp), S(2, 5, 4, 5));

    HI_ASSERT_SIMD_EQ(permute<"aaaa">(tmp), S(2, 2, 2, 2));
    HI_ASSERT_SIMD_EQ(permute<"xxxx">(tmp), S(2, 2, 2, 2));
    HI_ASSERT_SIMD_EQ(permute<"bbbb">(tmp), S(3, 3, 3, 3));
    HI_ASSERT_SIMD_EQ(permute<"cccc">(tmp), S(4, 4, 4, 4));
    HI_ASSERT_SIMD_EQ(permute<"dddd">(tmp), S(5, 5, 5, 5));
}

TEST(native_i32x4, swizzle)
{
    auto tmp = S(2, 3, 4, 5);

    HI_ASSERT_SIMD_EQ(swizzle<"abcd">(tmp), S(2, 3, 4, 5));
    HI_ASSERT_SIMD_EQ(swizzle<"xyzw">(tmp), S(2, 3, 4, 5));
    HI_ASSERT_SIMD_EQ(swizzle<"0000">(tmp), S(0, 0, 0, 0));

    HI_ASSERT_SIMD_EQ(swizzle<"dcba">(tmp), S(5, 4, 3, 2));
    HI_ASSERT_SIMD_EQ(swizzle<"wzyx">(tmp), S(5, 4, 3, 2));

    HI_ASSERT_SIMD_EQ(swizzle<"axcd">(tmp), S(2, 2, 4, 5));
    HI_ASSERT_SIMD_EQ(swizzle<"aycd">(tmp), S(2, 3, 4, 5));
    HI_ASSERT_SIMD_EQ(swizzle<"azcd">(tmp), S(2, 4, 4, 5));
    HI_ASSERT_SIMD_EQ(swizzle<"awcd">(tmp), S(2, 5, 4, 5));

    HI_ASSERT_SIMD_EQ(swizzle<"aaaa">(tmp), S(2, 2, 2, 2));
    HI_ASSERT_SIMD_EQ(swizzle<"xxxx">(tmp), S(2, 2, 2, 2));
    HI_ASSERT_SIMD_EQ(swizzle<"bbbb">(tmp), S(3, 3, 3, 3));
    HI_ASSERT_SIMD_EQ(swizzle<"cccc">(tmp), S(4, 4, 4, 4));
    HI_ASSERT_SIMD_EQ(swizzle<"dddd">(tmp), S(5, 5, 5, 5));

    HI_ASSERT_SIMD_EQ(swizzle<"0000">(tmp), S(0, 0, 0, 0));
    HI_ASSERT_SIMD_EQ(swizzle<"1000">(tmp), S(1, 0, 0, 0));
    HI_ASSERT_SIMD_EQ(swizzle<"0100">(tmp), S(0, 1, 0, 0));
    HI_ASSERT_SIMD_EQ(swizzle<"0010">(tmp), S(0, 0, 1, 0));
    HI_ASSERT_SIMD_EQ(swizzle<"0001">(tmp), S(0, 0, 0, 1));
    HI_ASSERT_SIMD_EQ(swizzle<"1001">(tmp), S(1, 0, 0, 1));
    HI_ASSERT_SIMD_EQ(swizzle<"1111">(tmp), S(1, 1, 1, 1));

    HI_ASSERT_SIMD_EQ(swizzle<"00b0">(tmp), S(0, 0, 3, 0));
    HI_ASSERT_SIMD_EQ(swizzle<"1b00">(tmp), S(1, 3, 0, 0));
    HI_ASSERT_SIMD_EQ(swizzle<"010b">(tmp), S(0, 1, 0, 3));
    HI_ASSERT_SIMD_EQ(swizzle<"0b10">(tmp), S(0, 3, 1, 0));
    HI_ASSERT_SIMD_EQ(swizzle<"b001">(tmp), S(3, 0, 0, 1));
    HI_ASSERT_SIMD_EQ(swizzle<"1b01">(tmp), S(1, 3, 0, 1));
    HI_ASSERT_SIMD_EQ(swizzle<"11b1">(tmp), S(1, 1, 3, 1));
    HI_ASSERT_SIMD_EQ(swizzle<"1111">(tmp), S(1, 1, 1, 1));
}

TEST(native_i32x4, horizontal)
{
#ifdef HI_HAS_SSE3
    HI_ASSERT_SIMD_EQ(horizontal_add(S(2, 3, 4, 5), S(12, 13, 14, 15)), S(5, 9, 25, 29));
    HI_ASSERT_SIMD_EQ(horizontal_sub(S(42, 3, 34, 5), S(2, 13, 24, 15)), S(39, 29, -11, 9));
#endif

    HI_ASSERT_SIMD_EQ(horizontal_sum(S(1, 2, 3, 4)), S::broadcast(10));

    HI_ASSERT_SIMD_EQ(dot_product<0b0000>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(0));
    HI_ASSERT_SIMD_EQ(dot_product<0b0001>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(3));
    HI_ASSERT_SIMD_EQ(dot_product<0b0010>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(10));
    HI_ASSERT_SIMD_EQ(dot_product<0b0011>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(13));
    HI_ASSERT_SIMD_EQ(dot_product<0b0100>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(-9));
    HI_ASSERT_SIMD_EQ(dot_product<0b0101>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(-6));
    HI_ASSERT_SIMD_EQ(dot_product<0b0110>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(1));
    HI_ASSERT_SIMD_EQ(dot_product<0b0111>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(4));
    HI_ASSERT_SIMD_EQ(dot_product<0b1000>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(-4));
    HI_ASSERT_SIMD_EQ(dot_product<0b1001>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(-1));
    HI_ASSERT_SIMD_EQ(dot_product<0b1010>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(6));
    HI_ASSERT_SIMD_EQ(dot_product<0b1011>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(9));
    HI_ASSERT_SIMD_EQ(dot_product<0b1100>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(-13));
    HI_ASSERT_SIMD_EQ(dot_product<0b1101>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(-10));
    HI_ASSERT_SIMD_EQ(dot_product<0b1110>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(-3));
    HI_ASSERT_SIMD_EQ(dot_product<0b1111>(S(1, 2, 3, 4), S(3, 5, -3, -1)), S::broadcast(0));
}

hi_warning_pop();
