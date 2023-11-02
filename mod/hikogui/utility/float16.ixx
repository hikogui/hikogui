// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <cstdint>
#include <type_traits>
#include <bit>
#include <algorithm>
#include <numeric>
#include <format>

export module hikogui_utility_float16;
import hikogui_utility_misc;

hi_warning_push();
// C26472: Don't use static_cast for arithmetic conversions, Use brace initialization, gsl::narrow_cast or gsl::narrow (type.1).
// static_cast here is used to extract bits and cause sign-extension.
hi_warning_ignore_msvc(26472);

export namespace hi::inline v1 {

constexpr uint32_t float16_bias = 15;
constexpr uint32_t float32_bias = 127;
constexpr uint32_t f32_to_f16_adjustment_exponent = float32_bias - float16_bias;
constexpr uint32_t f32_to_f16_lowest_normal_exponent = 0x01 + f32_to_f16_adjustment_exponent;
constexpr uint32_t f32_to_f16_infinite_exponent = 0x1f + f32_to_f16_adjustment_exponent;
constexpr uint32_t f32_to_f16_adjustment = f32_to_f16_adjustment_exponent << 23;
constexpr uint32_t f32_to_f16_lowest_normal = f32_to_f16_lowest_normal_exponent << 23;
constexpr uint32_t f32_to_f16_infinite = f32_to_f16_infinite_exponent << 23;

constexpr float cvtsh_ss(uint16_t value) noexcept
{
    // Convert the 16 bit values to 32 bit with leading zeros.
    uint32_t u = value;

    // Extract the sign bit.
    hilet sign = (u >> 15) << 31;

    // Strip the sign bit and align the exponent/mantissa boundary to a float 32.
    u = (u << 17) >> 4;

    // Adjust the bias. f32_to_f16_adjustment
    u = u + f32_to_f16_adjustment;

    // Get a mask of '1' bits when the half-float would be normal or infinite.
    hilet is_normal = u > (f32_to_f16_lowest_normal - 1);

    // Add the sign bit back in.
    u = u | sign;

    // Keep the value if normal, if denormal make it zero.
    u = is_normal ? u : 0;

    return std::bit_cast<float>(u);
}

constexpr uint16_t cvtss_sh(float value) noexcept
{
    // Interpret the floating point number as 32 bit-field.
    auto u = std::bit_cast<uint32_t>(value);

    // Get the sign of the floating point number as a bit mask of the upper 17 bits.
    hilet sign = static_cast<uint32_t>(static_cast<int32_t>(u) >> 31) << 15;

    // Strip sign bit.
    u = (u << 1) >> 1;

    // Get a mask of '1' bits when the half-float would be normal or infinite.
    hilet is_normal = u > (f32_to_f16_lowest_normal - 1);

    // Clamp the floating point number to where the half-float would be infinite.
    u = std::min(u, f32_to_f16_infinite); // SSE4.1

    // Convert the bias from float to half-float.
    u = u - f32_to_f16_adjustment;

    // Shift the float until it becomes a half-float. This truncates the mantissa.
    u = u >> 13;

    // Keep the value if normal, if denormal make it zero.
    u = is_normal ? u : 0;

    // Add the sign bit back in, also set the upper 16 bits so that saturated pack
    // will work correctly when converting to int16.
    u = u | sign;

    // Saturate and pack the 32 bit integers to 16 bit integers.
    return static_cast<uint16_t>(u);
}

export struct float16 {
    uint16_t v = 0;

    constexpr float16() noexcept = default;
    ~float16() = default;
    constexpr float16(float16 const&) noexcept = default;
    constexpr float16(float16&&) noexcept = default;
    constexpr float16& operator=(float16 const&) noexcept = default;
    constexpr float16& operator=(float16&&) noexcept = default;

    constexpr explicit float16(float other) noexcept : v(cvtss_sh(other)) {}
    constexpr explicit float16(double other) noexcept : float16(static_cast<float>(other)) {}
    constexpr explicit float16(long double other) noexcept : float16(static_cast<float>(other)) {}

    constexpr float16(intrinsic_t, uint16_t v) noexcept : v(v) {}

    [[nodiscard]] constexpr uint16_t const& intrinsic() const noexcept
    {
        return v;
    }

    [[nodiscard]] constexpr uint16_t& intrinsic() noexcept
    {
        return v;
    }

    constexpr float16& operator=(float other) noexcept
    {
        v = cvtss_sh(other);
        return *this;
    }

    constexpr operator float() const noexcept
    {
        return cvtsh_ss(v);
    }

    [[nodiscard]] std::size_t hash() const noexcept
    {
        return std::hash<uint16_t>{}(v);
    }

    [[nodiscard]] constexpr friend bool operator==(float16 const& lhs, float16 const& rhs) noexcept
    {
        return static_cast<float>(lhs) == static_cast<float>(rhs);
    }

    [[nodiscard]] constexpr friend auto operator<=>(float16 const& lhs, float16 const& rhs) noexcept
    {
        return static_cast<float>(lhs) <=> static_cast<float>(rhs);
    }

#define HI_X_binary_math_op(op) \
    [[nodiscard]] constexpr friend float16 operator op(float16 const& lhs, float16 const& rhs) noexcept \
    { \
        return float16{static_cast<float>(lhs) op static_cast<float>(rhs)}; \
    }

    // clang-format off
    HI_X_binary_math_op(+)
    HI_X_binary_math_op(-)
    HI_X_binary_math_op(*)
    HI_X_binary_math_op(/)
    // clang-format on
#undef HI_X_binary_math_op

    //[[nodiscard]] constexpr friend float16 operator%(float16 const& lhs, float16 const& rhs) noexcept
    //{
    //    hilet lhs_ = static_cast<float>(lhs);
    //    hilet rhs_ = static_cast<float>(rhs);
    //    hilet div_result = std::floor(lhs_ / rhs_);
    //    return float16{lhs_ - (div_result * rhs_)};
    //}

#define HI_X_binary_bit_op(op) \
    [[nodiscard]] constexpr friend float16 operator op(float16 const& lhs, float16 const& rhs) noexcept \
    { \
        return float16(hi::intrinsic, lhs.v op rhs.v); \
    }

        // clang-format off
    HI_X_binary_bit_op(|)
    HI_X_binary_bit_op(&)
    HI_X_binary_bit_op(^)
    // clang-format on
#undef HI_X_binary_bit_op
};

// Check if float16 can be std::bit_cast<uint16_t>().
static_assert(sizeof(float16) == sizeof(uint16_t));
static_assert(std::is_trivially_copy_constructible_v<float16>);
static_assert(std::is_trivially_move_constructible_v<float16>);
static_assert(std::is_trivially_copy_assignable_v<float16>);
static_assert(std::is_trivially_move_assignable_v<float16>);
static_assert(std::is_trivially_destructible_v<float16>);

static_assert(requires(float16 a) { std::bit_cast<uint16_t>(a); });
static_assert(requires(uint16_t a) { std::bit_cast<float16>(a); });

} // namespace hi::inline v1

export template<>
struct std::hash<hi::float16> {
    std::size_t operator()(hi::float16 const& rhs) noexcept
    {
        return rhs.hash();
    }
};

// XXX #617 MSVC bug does not handle partial specialization in modules.
export template<>
struct std::formatter<hi::float16, char> : std::formatter<float, char> {
    constexpr auto format(hi::float16 const& t, auto& fc) const
    {
        return std::formatter<float, char>::format(static_cast<float>(t), fc);
    }
};

export template<>
struct std::numeric_limits<hi::float16> {
    using value_type = hi::float16;

    constexpr static bool is_specialized = true;
    constexpr static bool is_signed = true;
    constexpr static bool is_integer = false;
    constexpr static bool is_exact = false;
    constexpr static bool has_infinity = true;
    constexpr static bool has_quiet_NaN = true;
    constexpr static bool has_signaling_NaN = false;
    constexpr static float_round_style round_style = std::round_to_nearest;
    constexpr static bool is_iec559 = true;
    constexpr static bool is_bounded = true;
    constexpr static bool is_modulo = false;
    constexpr static int digits = 10;
    constexpr static int digits10 = 4;
    constexpr static int max_digits10 = 4;
    constexpr static int min_exponent = -14;
    constexpr static int min_exponent10 = -3;
    constexpr static int max_exponent = 15;
    constexpr static int max_exponent10 = 3;
    constexpr static bool traps = false;
    constexpr static bool tinyness_before = false;

    constexpr static value_type min() noexcept
    {
        return hi::float16(hi::intrinsic, 0x0400);
    }

    constexpr static value_type lowest() noexcept
    {
        return hi::float16(hi::intrinsic, 0xfbff);
    }

    constexpr static value_type max() noexcept
    {
        return hi::float16(hi::intrinsic, 0x7bff);
    }

    constexpr static value_type epsilon() noexcept
    {
        return hi::float16(hi::intrinsic, 0xfbff);
    }

    constexpr static value_type round_error() noexcept
    {
        return hi::float16(hi::intrinsic, 0x3800); // 0.5
    }

    constexpr static value_type infinity() noexcept
    {
        return hi::float16(hi::intrinsic, 0x7c00);
    }

    constexpr static value_type quiet_NaN() noexcept
    {
        return hi::float16(hi::intrinsic, 0x7c01);
    }

    constexpr static value_type signaling_NaN() noexcept
    {
        return hi::float16(hi::intrinsic, 0x7e01);
    }

    constexpr static value_type denorm_min() noexcept
    {
        return hi::float16(hi::intrinsic, 0x0001);
    }
};

hi_warning_pop();
