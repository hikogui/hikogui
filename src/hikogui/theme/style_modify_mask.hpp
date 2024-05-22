// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include <utility>

hi_export_module(hikogui.theme : style_modify_mask);

hi_export namespace hi {
inline namespace v1 {

enum class style_modify_mask {
    /** No values were modified.
     */
    none = 0b000,

    /** A color value was modified.
     */
    color = 0b001,

    /** A layout (size, alignment) value was modified.
     */
    layout = 0b010,

    /** The path of the style was modified.
     */
    path = 0b100,
};

[[nodiscard]] constexpr style_modify_mask operator|(style_modify_mask const& lhs, style_modify_mask const& rhs) noexcept
{
    return static_cast<style_modify_mask>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr style_modify_mask operator&(style_modify_mask const& lhs, style_modify_mask const& rhs) noexcept
{
    return static_cast<style_modify_mask>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

constexpr style_modify_mask& operator|=(style_modify_mask &lhs, style_modify_mask const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

constexpr style_modify_mask& operator&=(style_modify_mask &lhs, style_modify_mask const& rhs) noexcept
{
    return lhs = lhs & rhs;
}

[[nodiscard]] constexpr bool to_bool(style_modify_mask const& rhs) noexcept
{
    return static_cast<bool>(std::to_underlying(rhs));
}

}}
