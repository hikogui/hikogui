// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <emmintrin.h>
#include <immintrin.h>
#include <type_traits>
#include "rapid/numeric_array.hpp"

namespace tt {

constexpr uint32_t float16_bias = 15;
constexpr uint32_t float32_bias = 127;
constexpr uint32_t f32_to_f16_adjustment_exponent = float32_bias - float16_bias;
constexpr uint32_t f32_to_f16_lowest_normal_exponent = 0x01 + f32_to_f16_adjustment_exponent;
constexpr uint32_t f32_to_f16_infinite_exponent = 0x1f + f32_to_f16_adjustment_exponent;
constexpr uint32_t f32_to_f16_adjustment = f32_to_f16_adjustment_exponent << 23;
constexpr uint32_t f32_to_f16_lowest_normal = f32_to_f16_lowest_normal_exponent << 23;
constexpr uint32_t f32_to_f16_infinite = f32_to_f16_infinite_exponent << 23;

// Test with greater or equal is slow, so test with greater than, adjust lowerst_normal.
inline constinit u32x4 f32_to_f16_constants = u32x4{f32_to_f16_lowest_normal - 1, f32_to_f16_infinite, f32_to_f16_adjustment, 0};

constexpr f32x4 f16x8_to_f32x4(i16x8 value) noexcept
{
    // Convert the 16 bit values to 32 bit with leading zeros.
    auto u = bit_cast<u32x4>(i16x8::interleave_lo(value, i16x8{}));

    // Extract the sign bit.
    auto sign = (u >> 15) << 31;

    // Strip the sign bit and align the exponent/mantissa boundary to a float 32.
    u = (u << 17) >> 4;

    // Adjust the bias.
    u = u + f32_to_f16_constants.zzzz();

    // Get a mask of '1' bits when the half-float would be normal or infinite.
    auto is_normal = bit_cast<u32x4>(gt_mask(bit_cast<i32x4>(u), bit_cast<i32x4>(f32_to_f16_constants.xxxx())));

    // Add the sign bit back in.
    u = u | bit_cast<u32x4>(sign);

    // Keep the value if normal, if denormal make it zero.
    u = u & is_normal;

    return bit_cast<f32x4>(u);
}

constexpr i16x8 f32x4_to_f16x8(f32x4 value) noexcept
{
    // Interpret the floating point number as 32 bit-field.
    auto u = bit_cast<u32x4>(value);

    // Get the sign of the floating point number as a bit mask of the upper 17 bits.
    auto sign = (bit_cast<i32x4>(u) >> 31) << 15;

    // Strip sign bit.
    u = (u << 1) >> 1;

    // Get a mask of '1' bits when the half-float would be normal or infinite.
    auto is_normal = bit_cast<u32x4>(gt_mask(bit_cast<i32x4>(u), bit_cast<i32x4>(f32_to_f16_constants.xxxx())));

    // Clamp the floating point number to where the half-float would be infinite.
    u = min(u, f32_to_f16_constants.yyyy());

    // Convert the bias from float to half-float.
    u = u - f32_to_f16_constants.zzzz();

    // Shift the float until it becomes a half-float. This truncates the mantissa.
    u = u >> 13;

    // Keep the value if normal, if denormal make it zero.
    u = u & is_normal;

    // Add the sign bit back in, also set the upper 16 bits so that saturated pack
    // will work correctly when converting to int16.
    u = u | bit_cast<u32x4>(sign);

    // Saturate and pack the 32 bit integers to 16 bit integers.
    auto tmp = bit_cast<i32x4>(u);
    return i16x8{tmp, tmp};
}

class float16 {
    uint16_t v;

public:
    float16() noexcept : v() {}

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    float16(T const &rhs) noexcept {
        ttlet tmp1 = f32x4{narrow_cast<float>(rhs)};
        ttlet tmp2 = f32x4_to_f16x8(tmp1);
        v = tmp2.x();
    }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    float16 &operator=(T const &rhs) noexcept {
        ttlet tmp1 = f32x4{narrow_cast<float>(rhs)};
        ttlet tmp2 = f32x4_to_f16x8(tmp1);
        v = tmp2.x();
        return *this;
    }

    operator float () const noexcept {
        ttlet tmp1 = i16x8{static_cast<int16_t>(v)};
        ttlet tmp2 = f16x8_to_f32x4(tmp1);
        return tmp2.x();
    }

    static float16 from_uint16_t(uint16_t const rhs) noexcept
    {
        auto r = float16{};
        r.v = rhs;
        return r;
    }

    [[nodiscard]] constexpr uint16_t get() const noexcept {
        return v;
    }

    constexpr float16 &set(uint16_t rhs) noexcept {
        v = rhs;
        return *this;
    }

    [[nodiscard]] size_t hash() const noexcept
    {
        return std::hash<uint16_t>{}(v);
    }

    [[nodiscard]] friend bool operator==(float16 const &lhs, float16 const &rhs) noexcept {
        return lhs.v == rhs.v;
    }

    [[nodiscard]] friend bool operator!=(float16 const &lhs, float16 const &rhs) noexcept {
        return lhs.v != rhs.v;
    }
};

}

namespace std {

template<>
struct std::hash<tt::float16> {
    size_t operator()(tt::float16 const &rhs) noexcept
    {
        return rhs.hash();
    }
};

}
