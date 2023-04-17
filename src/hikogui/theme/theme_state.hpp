// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include <string>
#include <ostream>

namespace hi {
inline namespace v1 {

// clang-format off
enum class theme_state : uint8_t {
    disabled = 0b00'0'0'00,
    enabled  = 0b00'0'0'01,
    hover    = 0b00'0'0'10,
    active   = 0b00'0'0'11,
    no_focus = 0b00'0'0'00,
    focus    = 0b00'0'1'00,
    off      = 0b00'0'0'00,
    on       = 0b00'1'0'00,
    layer_0  = 0b00'0'0'00,
    layer_1  = 0b01'0'0'00,
    layer_2  = 0b10'0'0'00,
    layer_3  = 0b11'0'0'00
};
// clang-format on

constexpr auto theme_state_size = 64_uz;

[[nodiscard]] constexpr theme_state operator|(theme_state const &lhs, theme_state const &rhs) noexcept
{
    return static_cast<theme_state>(to_underlying(lhs) | to_underlying(rhs));
}

constexpr theme_state &operator|=(theme_state &lhs, theme_state const &rhs) noexcept
{
    return lhs = lhs | rhs;
}

// clang-format off
enum class theme_state_mask : uint8_t {
    mouse  = 0b00'0'0'11,
    focus  = 0b00'0'1'00,
    value  = 0b00'1'0'00,
    layers = 0b11'0'0'00,
};
// clang-format on

[[nodiscard]] constexpr theme_state_mask operator|(theme_state_mask const& lhs, theme_state_mask const& rhs) noexcept
{
    return static_cast<theme_state_mask>(to_underlying(lhs) | to_underlying(rhs));
}

constexpr theme_state_mask& operator|=(theme_state_mask& lhs, theme_state_mask const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

}}
