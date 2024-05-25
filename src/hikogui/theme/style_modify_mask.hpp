// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include <utility>
#include <cstdint>

hi_export_module(hikogui.theme : style_modify_mask);

hi_export namespace hi {
inline namespace v1 {

enum class style_modify_mask : uint8_t {
    /** No values were modified.
     */
    none = 0b0000'0000,

    /** All values were modified (except the theme).
     */
    all = 0b1'11111'11,

    /** A color value was modified.
     */
    color = 0b0'00000'01,

    /** A border-width or border-radius was modified.
     */
    weight = 0b0'00000'10,

    /** A size value was modified.
     */
    size = 0b0'00001'00,

    /** A margin or padding value was modified.
     */
    margin = 0b0'00010'00,

    /** A alignment was changed.
     */
    alignment = 0b0'00100'00,

    /** A font or font-size has changed.
     */
    font = 0b0'10000'00,

    /** Only visual changes.
     */
    redraw = color | weight,

    /** A layout (size, alignment) value was modified.
     */
    layout = size | margin | alignment | font,

    /** The attributes that need to be modified when the pixel density changes.
    */
    pixel_density = weight | size | margin | font, 
};

[[nodiscard]] constexpr style_modify_mask operator|(style_modify_mask const& lhs, style_modify_mask const& rhs) noexcept
{
    return static_cast<style_modify_mask>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr style_modify_mask operator&(style_modify_mask const& lhs, style_modify_mask const& rhs) noexcept
{
    return static_cast<style_modify_mask>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

constexpr style_modify_mask& operator|=(style_modify_mask& lhs, style_modify_mask const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

constexpr style_modify_mask& operator&=(style_modify_mask& lhs, style_modify_mask const& rhs) noexcept
{
    return lhs = lhs & rhs;
}

[[nodiscard]] constexpr bool to_bool(style_modify_mask const& rhs) noexcept
{
    return static_cast<bool>(std::to_underlying(rhs));
}

} // namespace v1
}
