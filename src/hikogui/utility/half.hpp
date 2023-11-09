// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "half_to_float.hpp"
#include "float_to_half.hpp"
#include "misc.hpp"
#include <cstdint>
#include <type_traits>
#include <bit>
#include <algorithm>
#include <numeric>
#include <format>

hi_export_module(hikogui.utility.half);

hi_warning_push();
// C26472: Don't use static_cast for arithmetic conversions, Use brace initialization, gsl::narrow_cast or gsl::narrow (type.1).
// static_cast here is used to extract bits and cause sign-extension.
hi_warning_ignore_msvc(26472);

hi_export namespace hi::inline v1 {

hi_export struct half {
    uint16_t v = 0;

    constexpr half() noexcept = default;
    ~half() = default;
    constexpr half(half const&) noexcept = default;
    constexpr half(half&&) noexcept = default;
    constexpr half& operator=(half const&) noexcept = default;
    constexpr half& operator=(half&&) noexcept = default;

    constexpr explicit half(float other) noexcept : v(float_to_half(other)) {}
    constexpr explicit half(double other) noexcept : half(static_cast<float>(other)) {}
    constexpr explicit half(long double other) noexcept : half(static_cast<float>(other)) {}

    constexpr half(intrinsic_t, uint16_t v) noexcept : v(v) {}

    [[nodiscard]] constexpr uint16_t const& intrinsic() const noexcept
    {
        return v;
    }

    [[nodiscard]] constexpr uint16_t& intrinsic() noexcept
    {
        return v;
    }

    constexpr half& operator=(float other) noexcept
    {
        v = float_to_half(other);
        return *this;
    }

    constexpr operator float() const noexcept
    {
        return half_to_float(v);
    }

    [[nodiscard]] std::size_t hash() const noexcept
    {
        return std::hash<uint16_t>{}(v);
    }

    [[nodiscard]] constexpr friend bool operator==(half const& lhs, half const& rhs) noexcept
    {
        return static_cast<float>(lhs) == static_cast<float>(rhs);
    }

    [[nodiscard]] constexpr friend auto operator<=>(half const& lhs, half const& rhs) noexcept
    {
        return static_cast<float>(lhs) <=> static_cast<float>(rhs);
    }

#define HI_X_binary_math_op(op) \
    [[nodiscard]] constexpr friend half operator op(half const& lhs, half const& rhs) noexcept \
    { \
        return half{static_cast<float>(lhs) op static_cast<float>(rhs)}; \
    }

    // clang-format off
    HI_X_binary_math_op(+)
    HI_X_binary_math_op(-)
    HI_X_binary_math_op(*)
    HI_X_binary_math_op(/)
    // clang-format on
#undef HI_X_binary_math_op

    //[[nodiscard]] constexpr friend half operator%(half const& lhs, half const& rhs) noexcept
    //{
    //    hilet lhs_ = static_cast<float>(lhs);
    //    hilet rhs_ = static_cast<float>(rhs);
    //    hilet div_result = std::floor(lhs_ / rhs_);
    //    return half{lhs_ - (div_result * rhs_)};
    //}

#define HI_X_binary_bit_op(op) \
    [[nodiscard]] constexpr friend half operator op(half const& lhs, half const& rhs) noexcept \
    { \
        return half(hi::intrinsic, lhs.v op rhs.v); \
    }

        // clang-format off
    HI_X_binary_bit_op(|)
    HI_X_binary_bit_op(&)
    HI_X_binary_bit_op(^)
    // clang-format on
#undef HI_X_binary_bit_op
};

// Check if half can be std::bit_cast<uint16_t>().
static_assert(sizeof(half) == sizeof(uint16_t));
static_assert(std::is_trivially_copy_constructible_v<half>);
static_assert(std::is_trivially_move_constructible_v<half>);
static_assert(std::is_trivially_copy_assignable_v<half>);
static_assert(std::is_trivially_move_assignable_v<half>);
static_assert(std::is_trivially_destructible_v<half>);

static_assert(requires(half a) { std::bit_cast<uint16_t>(a); });
static_assert(requires(uint16_t a) { std::bit_cast<half>(a); });

} // namespace hi::inline v1

hi_export template<>
struct std::hash<hi::half> {
    std::size_t operator()(hi::half const& rhs) noexcept
    {
        return rhs.hash();
    }
};

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::half, char> : std::formatter<float, char> {
    constexpr auto format(hi::half const& t, auto& fc) const
    {
        return std::formatter<float, char>::format(static_cast<float>(t), fc);
    }
};

hi_export template<>
struct std::numeric_limits<hi::half> {
    using value_type = hi::half;

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
        return hi::half(hi::intrinsic, 0x0400);
    }

    constexpr static value_type lowest() noexcept
    {
        return hi::half(hi::intrinsic, 0xfbff);
    }

    constexpr static value_type max() noexcept
    {
        return hi::half(hi::intrinsic, 0x7bff);
    }

    constexpr static value_type epsilon() noexcept
    {
        return hi::half(hi::intrinsic, 0xfbff);
    }

    constexpr static value_type round_error() noexcept
    {
        return hi::half(hi::intrinsic, 0x3800); // 0.5
    }

    constexpr static value_type infinity() noexcept
    {
        return hi::half(hi::intrinsic, 0x7c00);
    }

    constexpr static value_type quiet_NaN() noexcept
    {
        return hi::half(hi::intrinsic, 0x7c01);
    }

    constexpr static value_type signaling_NaN() noexcept
    {
        return hi::half(hi::intrinsic, 0x7e01);
    }

    constexpr static value_type denorm_min() noexcept
    {
        return hi::half(hi::intrinsic, 0x0001);
    }
};

hi_warning_pop();
