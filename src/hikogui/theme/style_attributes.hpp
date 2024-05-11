// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../units/units.hpp"
#include "../macros.hpp"
#include <cstdint>

hi_export_module(hikogui.theme : style_attributes);

hi_export namespace hi {
inline namespace v1 {

class style_attributes {
public:
    constexpr style_attributes() noexcept = default;
    constexpr style_attributes(style_attributes const&) noexcept = default;
    constexpr style_attributes(style_attributes&&) noexcept = default;
    constexpr style_attributes& operator=(style_attributes const&) noexcept = default;
    constexpr style_attributes& operator=(style_attributes&&) noexcept = default;

#define HIX_GETSET(NAME) \
    decltype(_##NAME) NAME() noexcept \
    { \
        hi_axiom(_##NAME##_valid); \
        return _##NAME; \
    } \
    void set_##NAME(decltype(_##NAME) NAME, bool important = false) noexcept \
    { \
        if (important or not _##NAME##_important) { \
            _##NAME##_important |= static_cast<uint64_t>(important); \
            _##NAME##_valid = 1; \
            _##NAME = NAME; \
        } \
    }

    HIX_GETSET(width)
    HIX_GETSET(height)
    HIX_GETSET(margin_left)
    HIX_GETSET(margin_bottom)
    HIX_GETSET(margin_right)
    HIX_GETSET(margin_top)
    HIX_GETSET(padding_left)
    HIX_GETSET(padding_bottom)
    HIX_GETSET(padding_right)
    HIX_GETSET(padding_top)
    HIX_GETSET(border_width)
    HIX_GETSET(left_bottom_corner_radius)
    HIX_GETSET(right_bottom_corner_radius)
    HIX_GETSET(left_top_corner_radius)
    HIX_GETSET(right_top_corner_radius)
    HIX_GETSET(foreground_color)
    HIX_GETSET(background_color)
    HIX_GETSET(border_color)
    HIX_GETSET(horizontal_alignment)
    HIX_GETSET(vertical_alignment)
#undef HIX_GETSET

    void set_margin(length_f margin, bool important = false) noexcept
    {
        if (important or not _left_margin_important) {
            _left_margin_important |= static_cast<uint64_t>(important);
            _left_margin_valid = 1;
            _left_margin = margin;
        }
        if (important or not _bottom_margin_important) {
            _bottom_margin_important |= static_cast<uint64_t>(important);
            _bottom_margin_valid = 1;
            _bottom_margin = margin;
        }
        if (important or not _right_margin_important) {
            _right_margin_important |= static_cast<uint64_t>(important);
            _right_margin_valid = 1;
            _right_margin = margin;
        }
        if (important or not _top_margin_important) {
            _top_margin_important |= static_cast<uint64_t>(important);
            _top_margin_valid = 1;
            _top_margin = margin;
        }
    }

    void set_padding(length_f padding, bool important = false) noexcept
    {
        if (important or not _left_padding_important) {
            _left_padding_important |= static_cast<uint64_t>(important);
            _left_padding_valid = 1;
            _left_padding = padding;
        }
        if (important or not _bottom_padding_important) {
            _bottom_padding_important |= static_cast<uint64_t>(important);
            _bottom_padding_valid = 1;
            _bottom_padding = padding;
        }
        if (important or not _right_padding_important) {
            _right_padding_important |= static_cast<uint64_t>(important);
            _right_padding_valid = 1;
            _right_padding = padding;
        }
        if (important or not _top_padding_important) {
            _top_padding_important |= static_cast<uint64_t>(important);
            _top_padding_valid = 1;
            _top_padding = padding;
        }
    }

    void set_corner_radius(length_f corner_radius, bool important = false) noexcept
    {
        if (important or not _left_bottom_corner_radius_important) {
            _left_bottom_corner_radius_important |= static_cast<uint64_t>(important);
            _left_bottom_corner_radius_valid = 1;
            _left_bottom_corner_radius = corner_radius;
        }
        if (important or not _right_bottom_corner_radius_important) {
            _right_bottom_corner_radius_important |= static_cast<uint64_t>(important);
            _right_bottom_corner_radius_valid = 1;
            _right_bottom_corner_radius = corner_radius;
        }
        if (important or not _left_top_corner_radius_important) {
            _left_top_corner_radius_important |= static_cast<uint64_t>(important);
            _left_top_corner_radius_valid = 1;
            _left_top_corner_radius = corner_radius;
        }
        if (important or not _right_top_corner_radius_important) {
            _right_top_corner_radius_padding_important |= static_cast<uint64_t>(important);
            _right_top_corner_radius_padding_valid = 1;
            _right_top_corner_radius_padding = corner_radius;
        }
    }

    void clear() noexcept
    {
        *this = style_attributes{};
    }

    void apply(style_attributes const& other) noexcept
    {
#define HIX_APPLY(NAME) \
    if (other._##NAME##_valid) { \
        set_##NAME(other._##NAME, static_cast<bool>(other._##NAME##_important)); \
    }

        HIX_APPLY(width)
        HIX_APPLY(height)
        HIX_APPLY(margin_left)
        HIX_APPLY(margin_bottom)
        HIX_APPLY(margin_right)
        HIX_APPLY(margin_top)
        HIX_APPLY(padding_left)
        HIX_APPLY(padding_bottom)
        HIX_APPLY(padding_right)
        HIX_APPLY(padding_top)
        HIX_APPLY(border_width)
        HIX_APPLY(left_bottom_corner_radius)
        HIX_APPLY(right_bottom_corner_radius)
        HIX_APPLY(left_top_corner_radius)
        HIX_APPLY(right_top_corner_radius)
        HIX_APPLY(foreground_color)
        HIX_APPLY(background_color)
        HIX_APPLY(border_color)
        HIX_APPLY(horizontal_alignment)
        HIX_APPLY(vertical_alignment)
#undef HIX_APPLY
    }

private:
    hi::length_f _width = hi::points{0.0f};
    hi::length_f _height = hi::points{0.0f};
    hi::length_f _margin_left = hi::points{0.0f};
    hi::length_f _margin_bottom = hi::points{0.0f};
    hi::length_f _margin_right = hi::points{0.0f};
    hi::length_f _margin_top = hi::points{0.0f};
    hi::length_f _padding_left = hi::points{0.0f};
    hi::length_f _padding_bottom = hi::points{0.0f};
    hi::length_f _padding_right = hi::points{0.0f};
    hi::length_f _padding_top = hi::points{0.0f};
    hi::length_f _border_width = hi::points{0.0f};
    hi::length_f _left_bottom_corner_radius = hi::points{0.0f};
    hi::length_f _right_bottom_corner_radius = hi::points{0.0f};
    hi::length_f _left_top_corner_radius = hi::points{0.0f};
    hi::length_f _right_top_corner_radius = hi::points{0.0f};
    hi::color _foreground_color = {};
    hi::color _background_color = {};
    hi::color _border_color = 0;
    hi::horizontal_alignment _horizontal_alignment = hi::horizontal_alignment::natural;
    hi::vertical_alignment _vertical_alignment = hi::vertical_alignment::top;

    uint64_t _width_valid : 1 = 0;
    uint64_t _height_valid : 1 = 0;
    uint64_t _margin_left_valid : 1 = 0;
    uint64_t _margin_bottom_valid : 1 = 0;
    uint64_t _margin_right_valid : 1 = 0;
    uint64_t _margin_top_valid : 1 = 0;
    uint64_t _padding_left_valid : 1 = 0;
    uint64_t _padding_bottom_valid : 1 = 0;
    uint64_t _padding_right_valid : 1 = 0;
    uint64_t _padding_top_valid : 1 = 0;
    uint64_t _border_width_valid : 1 = 0;
    uint64_t _left_bottom_corner_radius_valid : 1 = 0;
    uint64_t _right_bottom_corner_radius_valid : 1 = 0;
    uint64_t _left_top_corner_radius_valid : 1 = 0;
    uint64_t _right_top_corner_radius_valid : 1 = 0;
    uint64_t _foreground_color_valid : 1 = 0;
    uint64_t _background_color_valid : 1 = 0;
    uint64_t _border_color_valid : 1 = 0;
    uint64_t _horizontal_alignment_valid : 1 = 0;
    uint64_t _vertical_alignment_valid : 1 = 0;

    uint64_t _width_important : 1 = 0;
    uint64_t _height_important : 1 = 0;
    uint64_t _margin_left_important : 1 = 0;
    uint64_t _margin_bottom_important : 1 = 0;
    uint64_t _margin_right_important : 1 = 0;
    uint64_t _margin_top_important : 1 = 0;
    uint64_t _padding_left_important : 1 = 0;
    uint64_t _padding_bottom_important : 1 = 0;
    uint64_t _padding_right_important : 1 = 0;
    uint64_t _padding_top_important : 1 = 0;
    uint64_t _border_width_important : 1 = 0;
    uint64_t _left_bottom_corner_radius_important : 1 = 0;
    uint64_t _right_bottom_corner_radius_important : 1 = 0;
    uint64_t _left_top_corner_radius_important : 1 = 0;
    uint64_t _right_top_corner_radius_important : 1 = 0;
    uint64_t _foreground_color_important : 1 = 0;
    uint64_t _background_color_important : 1 = 0;
    uint64_t _border_color_important : 1 = 0;
    uint64_t _horizontal_alignment_important : 1 = 0;
    uint64_t _vertical_alignment_important : 1 = 0;
};

} // namespace v1
}
