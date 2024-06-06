// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "style_modify_mask.hpp"
#include "../text/text.hpp"
#include "../units/units.hpp"
#include "../color/color.hpp"
#include "../geometry/geometry.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <cassert>

hi_export_module(hikogui.theme : style_computed_properties);

hi_export namespace hi::inline v1 {
struct style_computed_properties {
    unit::pixels_f width;
    unit::pixels_f height;
    unit::pixels_per_em_f font_size;
    unit::pixels_f margin_left;
    unit::pixels_f margin_bottom;
    unit::pixels_f margin_right;
    unit::pixels_f margin_top;
    unit::pixels_f padding_left;
    unit::pixels_f padding_bottom;
    unit::pixels_f padding_right;
    unit::pixels_f padding_top;
    unit::pixels_f border_width;
    unit::pixels_f border_bottom_left_radius;
    unit::pixels_f border_bottom_right_radius;
    unit::pixels_f border_top_left_radius;
    unit::pixels_f border_top_right_radius;
    unit::pixels_f x_height;
    hi::horizontal_alignment horizontal_alignment;
    hi::vertical_alignment vertical_alignment;
    color foreground_color;
    color background_color;
    color border_color;
    color accent_color;
    text_style_set text_style;

    size_t _width_inherit : 1 = 0;
    size_t _height_inherit : 1 = 0;
    size_t _font_size_inherit : 1 = 0;
    size_t _margin_left_inherit : 1 = 0;
    size_t _margin_bottom_inherit : 1 = 0;
    size_t _margin_right_inherit : 1 = 0;
    size_t _margin_top_inherit : 1 = 0;
    size_t _padding_left_inherit : 1 = 0;
    size_t _padding_bottom_inherit : 1 = 0;
    size_t _padding_right_inherit : 1 = 0;
    size_t _padding_top_inherit : 1 = 0;
    size_t _border_width_inherit : 1 = 0;
    size_t _border_bottom_left_radius_inherit : 1 = 0;
    size_t _border_bottom_right_radius_inherit : 1 = 0;
    size_t _border_top_left_radius_inherit : 1 = 0;
    size_t _border_top_right_radius_inherit : 1 = 0;
    size_t _x_height_inherit : 1 = 0;
    size_t _horizontal_alignment_inherit : 1 = 0;
    size_t _vertical_alignment_inherit : 1 = 0;
    size_t _foreground_color_inherit : 1 = 0;
    size_t _background_color_inherit : 1 = 0;
    size_t _border_color_inherit : 1 = 0;
    size_t _accent_color_inherit : 1 = 0;
    size_t _text_style_inherit : 1 = 0;

    void inherit(style_computed_properties const& rhs) noexcept
    {
#define HIX_INHERIT(NAME) \
    if (_##NAME##_inherit) { \
        NAME = rhs.NAME; \
        assert(rhs._##NAME##_inherit == 0); \
        _##NAME##_inherit = 0; \
    }

        HIX_INHERIT(width);
        HIX_INHERIT(height);
        HIX_INHERIT(font_size);
        HIX_INHERIT(margin_left);
        HIX_INHERIT(margin_bottom);
        HIX_INHERIT(margin_right);
        HIX_INHERIT(margin_top);
        HIX_INHERIT(padding_left);
        HIX_INHERIT(padding_bottom);
        HIX_INHERIT(padding_right);
        HIX_INHERIT(padding_top);
        HIX_INHERIT(border_width);
        HIX_INHERIT(border_bottom_left_radius);
        HIX_INHERIT(border_bottom_right_radius);
        HIX_INHERIT(border_top_left_radius);
        HIX_INHERIT(border_top_right_radius);
        HIX_INHERIT(x_height);
        HIX_INHERIT(horizontal_alignment);
        HIX_INHERIT(vertical_alignment);
        HIX_INHERIT(foreground_color);
        HIX_INHERIT(background_color);
        HIX_INHERIT(border_color);
        HIX_INHERIT(accent_color);
        HIX_INHERIT(text_style);

#undef HIX_INHERIT
    }

    void set_properties(style_computed_properties const& rhs, style_modify_mask mask = style_modify_mask::all) noexcept
    {
        if (to_bool(mask & style_modify_mask::color)) {
            foreground_color = rhs.foreground_color;
            background_color = rhs.background_color;
            border_color = rhs.border_color;
            accent_color = rhs.accent_color;
        }

        if (to_bool(mask & style_modify_mask::size)) {
            width = rhs.width;
            height = rhs.height;
            font_size = rhs.font_size;
            text_style = rhs.text_style;
            x_height = rhs.x_height;
        }

        if (to_bool(mask & style_modify_mask::margin)) {
            margin_left = rhs.margin_left;
            margin_bottom = rhs.margin_bottom;
            margin_right = rhs.margin_right;
            margin_top = rhs.margin_top;
            padding_left = rhs.padding_left;
            padding_bottom = rhs.padding_bottom;
            padding_right = rhs.padding_right;
            padding_top = rhs.padding_top;
        }

        if (to_bool(mask & style_modify_mask::weight)) {
            border_width = rhs.border_width;
            border_bottom_left_radius = rhs.border_bottom_left_radius;
            border_bottom_right_radius = rhs.border_bottom_right_radius;
            border_top_left_radius = rhs.border_top_left_radius;
            border_top_right_radius = rhs.border_top_right_radius;
        }

        if (to_bool(mask & style_modify_mask::alignment)) {
            horizontal_alignment = rhs.horizontal_alignment;
            vertical_alignment = rhs.vertical_alignment;
        }
    }

    /** Set all attributes in other as-if they are important.
     * 
     * @param other The attributes used to overwrite the current attributes.
     * @return A mask for what kind of values where changed.
     */
    [[nodiscard]] friend style_modify_mask compare(style_computed_properties const& lhs, style_computed_properties const& rhs) noexcept
    {
#define HIX_COMPARE(NAME, MODIFY_MASK) \
        r |= lhs.NAME != rhs.NAME ? MODIFY_MASK : style_modify_mask{};

        auto r = style_modify_mask{};
        HIX_COMPARE(width, style_modify_mask::size)
        HIX_COMPARE(height, style_modify_mask::size)
        HIX_COMPARE(font_size, style_modify_mask::size)
        HIX_COMPARE(margin_left, style_modify_mask::margin)
        HIX_COMPARE(margin_bottom, style_modify_mask::margin)
        HIX_COMPARE(margin_right, style_modify_mask::margin)
        HIX_COMPARE(margin_top, style_modify_mask::margin)
        HIX_COMPARE(padding_left, style_modify_mask::margin)
        HIX_COMPARE(padding_bottom, style_modify_mask::margin)
        HIX_COMPARE(padding_right, style_modify_mask::margin)
        HIX_COMPARE(padding_top, style_modify_mask::margin)
        HIX_COMPARE(border_width, style_modify_mask::weight)
        HIX_COMPARE(border_bottom_left_radius, style_modify_mask::weight)
        HIX_COMPARE(border_bottom_right_radius, style_modify_mask::weight)
        HIX_COMPARE(border_top_left_radius, style_modify_mask::weight)
        HIX_COMPARE(border_top_right_radius, style_modify_mask::weight)
        HIX_COMPARE(foreground_color, style_modify_mask::color)
        HIX_COMPARE(background_color, style_modify_mask::color)
        HIX_COMPARE(border_color, style_modify_mask::color)
        HIX_COMPARE(accent_color, style_modify_mask::color)
        HIX_COMPARE(horizontal_alignment, style_modify_mask::alignment)
        HIX_COMPARE(vertical_alignment, style_modify_mask::alignment)
        HIX_COMPARE(x_height, style_modify_mask::alignment)
        HIX_COMPARE(text_style, style_modify_mask::size)
#undef HIX_COMPARE
        return r;
    }
};

}