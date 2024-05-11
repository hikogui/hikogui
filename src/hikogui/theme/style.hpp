// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "style_path.hpp"
#include "style_attributes.hpp"
#include "../macros.hpp"
#include <cstdint>

hi_export_module(hikogui.theme : style_attributes);

hi_export namespace hi {
inline namespace v1 {

class style {
public:
    pixels_f width;
    pixels_f height;
    pixels_f margin_left;
    pixels_f margin_bottom;
    pixels_f margin_right;
    pixels_f margin_top;
    pixels_f padding_left;
    pixels_f padding_bottom;
    pixels_f padding_right;
    pixels_f padding_top;
    pixels_f border_width;
    pixels_f left_bottom_corner_radius;
    pixels_f right_bottom_corner_radius;
    pixels_f left_top_corner_radius;
    pixels_f right_top_corner_radius;

    float width_px;
    float height_px;
    float margin_left_px;
    float margin_bottom_px;
    float margin_right_px;
    float margin_top_px;
    float padding_left_px;
    float padding_bottom_px;
    float padding_right_px;
    float padding_top_px;
    float border_width_px;
    float left_bottom_corner_radius_px;
    float right_bottom_corner_radius_px;
    float left_top_corner_radius_px;
    float right_top_corner_radius_px;

    hi::margins margins_px;
    hi::margins padding_px;
    hi::corner_radii corner_radii_px;

    color foreground_color;
    color background_color;
    color border_color;

    hi::horizontal_alignment horizontal_alignment;
    hi::vertical_alignment vertical_alignment;

    constexpr style &operator=(style const&) noexcept = default;
    constexpr style &operator=(style &&) noexcept = default;
    constexpr style &operator=(style const&) noexcept = default;
    constexpr style &operator=(style &&) noexcept = default;
    constexpr explicit style(std::string name) : path({style_path_segment{name}}) {}

    /** Parse the given string to configure this style.
     * 
     * The @a style_string has the following format:
     * ```
     * style_string := (id | class | attribute )*
     * id := '#' annex-31-minus
     * class := '.' annex-31-minus
     * attribute := annex-31-minus '=' value
     * value := color-value | length-value | horizontal-alignment-value | vertical-alignment-value
     * color-value := hex-color-value | rgb-color-value | rgba-color-value | named-color-value
     * hex-color-value := '#' [0-9A-Fa-f]{6,8}
     * rgb-color-value := 'rgb_color(' number ',' number ',' number ')'
     * rgba-color-value := 'rgb_color(' number ',' number ',' number ',' number ')'
     * named-color-value := annex-31-minus
     * length-value := number ('pd' | 'px' | 'pt' | 'in' | 'cm')?
     * horizontal-alignment-value := 'left' | 'right | 'center' | 'justified' | 'natural' | 'opposite'
     * vertical-alignment-value := 'bottom' | 'middle' | 'top'
     * ``` 
     */
    style &operator=(std::string style_string)
    {
        if (auto const optional_style = parse_style(style_string)) {
            auto const &[child_segment, new_attributes] = *optional_style;

            hi_axiom(not path.empty());
            path.back().id = child_segment.id;
            path.back().classes = child_segment.classes;

            override_attributes = new_attributes;
        } else if (optional_style.has_error()) {
            throw parse_error(optional_style.error());
        } else {
            hi_no_default();
        }

        update_from_theme();
        return *this;
    }

    void set_parent_path(style_path const &parent_path)
    {
        hi_axiom(not path.empty());
        auto back = std::move(path.back());
        path = parent_path;
        path.push_back(std::move(back));

        update_from_theme();
    }

    void set_state(style_pseudo_state state)
    {
        handle_state_change();
    }

    void set_pixel_density(hi::pixels_density pixel_density)
    {
        handle_density_change();
        _notifier();
    }

private:
    style_path _path;
    size_t _sibling_index;
    hi::pixel_density _density;
    style_state _state;

    style_attributes override_attributes;
    style_attributes length_attributes;
    std::array<style_attributes, num_style_pseudo_statuses> color_attributes;

    notifier<> _notifier;

    void update_from_theme()
    {
        for (auto i = size_t{0}; i != attributes_array.size(); ++i) {
            color_attributes[i] = theme_get_color_attributes(path, _sibling_index, style_status{i});
            color_attributes[i].apply(override_attributes);
            length_attributes = theme_get_length_attributes(path, _sibling_index);
        }

        handle_density_change();
        handle_state_change();
        _notifier();
    }

    void handle_density_change()
    {
        width = length_attributes.width() * _density;
        height = length_attributes.height() * _density;
        margin_left = length_attributes.margin_left() * _density;
        margin_bottom = length_attributes.margin_bottom() * _density;
        margin_right = length_attributes.margin_right() * _density;
        margin_top = length_attributes.margin_top() * _density;
        padding_left = length_attributes.padding_left() * _density;
        padding_bottom = length_attributes.padding_bottom() * _density;
        padding_right = length_attributes.padding_right() * _density;
        padding_top = length_attributes.padding_top() * _density;
        border_width = length_attributes.border_width() * _density;
        left_bottom_corner_radius = length_attributes.left_bottom_corner_radius() * _density;
        right_bottom_corner_radius = length_attributes.right_bottom_corner_radius() * _density;
        left_top_corner_radius = length_attributes.left_top_corner_radius() * _density;
        right_top_corner_radius = length_attributes.right_top_corner_radius() * _density;

        width_px = width.in(pixels);
        height_px = height.in(pixels);
        margin_left_px = margin_left.in(pixels);
        margin_bottom_px = margin_bottom.in(pixels);
        margin_right_px = margin_right.in(pixels);
        margin_top_px = margin_top.in(pixels);
        padding_left_px = padding_left.in(pixels);
        padding_bottom_px = padding_bottom.in(pixels);
        padding_right_px = padding_right.in(pixels);
        padding_top_px = padding_top.in(pixels);
        border_width_px = border_width.in(pixels);
        left_bottom_corner_radius_px = left_bottom_corner_radius.in(pixels);
        right_bottom_corner_radius_px = right_bottom_corner_radius.in(pixels);
        left_top_corner_radius_px = left_top_corner_radius.in(pixels);
        right_top_corner_radius_px = right_top_corner_radius.in(pixels);

        margins_px = hi::margins{margin_left_px, margin_bottom_px, margin_right_px, margin_top_px};
        padding_px = hi::margins{padding_left_px, padding_bottom_px, padding_right_px, padding_top_px};
        corner_radii_px = hi::corner_radii{left_bottom_corner_radius_px, right_bottom_corner_radius_px, left_top_corner_radius_px, right_top_corner_radius_px};

        horizontal_alignment = length_attributes.horizontal_alignment();
        vertical_alignment = length_attributes.vertical_alignment();
    }

    void handle_state_change()
    {
        background_color = _color_attributes[std::to_underlying(_state)].background_color;
        foreground_color = _color_attributes[std::to_underlying(_state)].foreground_color;
        border_color = _color_attributes[std::to_underlying(_state)].border_color;
    }
};

}}