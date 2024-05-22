// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "style_path.hpp"
#include "style_attributes.hpp"
#include "style_pseudo_class.hpp"
#include "../units/units.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <functional>
#include <utility>

hi_export_module(hikogui.theme : style_attributes);

hi_export namespace hi {
inline namespace v1 {

class style {
public:
    using theme_type = std::function<style_attributes(style_path, style_pseudo_class)>;

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
    pixels_f border_bottom_left_radius;
    pixels_f border_bottom_right_radius;
    pixels_f border_top_left_radius;
    pixels_f border_top_right_radius;

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
    float border_bottom_left_radius_px;
    float border_bottom_right_radius_px;
    float border_top_left_radius_px;
    float border_top_right_radius_px;

    hi::margins margins_px;
    hi::margins padding_px;
    hi::corner_radii border_radius_px;

    hi::horizontal_alignment horizontal_alignment;
    hi::vertical_alignment vertical_alignment;

    color foreground_color;
    color background_color;
    color border_color;

    style(style const&) noexcept = delete;
    style(style&&) noexcept = delete;
    style& operator=(style const&) noexcept = delete;
    style& operator=(style&&) noexcept = delete;
    style() noexcept = default;

    constexpr void set_parent(style const* new_parent) noexcept
    {
        if (new_parent) {
            _parent = const_cast<style*>(new_parent);
            _pixel_density = new_parent->_pixel_density;
            _theme = new_parent->_theme;
        } else {
            _parent = nullptr;
            _pixel_density = {};
            _theme = {};
        }
        reload();
        _notifier(style_modify_mask::path | style_modify_mask::layout | style_modify_mask::color);
    }

    [[nodiscard]] style* parent() const noexcept
    {
        return _parent;
    }

    constexpr void set_name(std::string name)
    {
        _name = name;
        reload();
        _notifier(style_modify_mask::path | style_modify_mask::layout | style_modify_mask::color);
    }

    [[nodiscard]] std::string const& name() const noexcept
    {
        return _name;
    }

    constexpr void set_id(std::string id)
    {
        _id = std::move(id);
        reload();
        _notifier(style_modify_mask::path | style_modify_mask::layout | style_modify_mask::color);
    }

    [[nodiscard]] std::string const& id() const noexcept
    {
        return _id;
    }

    constexpr void set_classes(std::vector<std::string> classes)
    {
        _classes = std::move(classes);
        reload();
        _notifier(style_modify_mask::path | style_modify_mask::layout | style_modify_mask::color);
    }

    [[nodiscard]] std::vector<std::string> const& classes() const noexcept
    {
        return _classes;
    }

    [[nodiscard]] constexpr style_path path() const noexcept
    {
        auto r = parent() ? parent()->path() : style_path{};
        r.emplace_back(_name, _id, _classes);
        return r;
    }

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
    style& operator=(std::string style_string)
    {
        if (auto const optional_style = parse_style(style_string)) {
            std::tie(_override_attributes, _id, _classes) = *optional_style;
        } else if (optional_style.has_error()) {
            throw parse_error(optional_style.error());
        } else {
            hi_no_default();
        }
        reload();
        _notifier(style_modify_mask::path | style_modify_mask::layout | style_modify_mask::color);
        return *this;
    }

    void set_theme(theme_type new_theme)
    {
        _theme = std::move(new_theme);
        reload();
        _notifier(style_modify_mask::layout | style_modify_mask::color);
    }

    void set_pixel_density(hi::pixel_density new_pixel_density)
    {
        _pixel_density = new_pixel_density;
        update_layout_values();
        _notifier(style_modify_mask::layout);
    }

    void set_pseudo_class(style_pseudo_class new_pseudo_class)
    {
        _pseudo_class = new_pseudo_class;
        auto const mask = _attributes.set(_loaded_attributes[std::to_underlying(new_pseudo_class)]);
        if (to_bool(mask & style_modify_mask::layout)) {
            update_layout_values();
        }
        if (to_bool(mask & style_modify_mask::color)) {
            update_color_values();
        }
        _notifier(mask);
    }

    /** Reload the style attributes from the current theme.
     *
     * Reload is called automatically after:
     *  - changing the theme.
     *  - changing the parent of the style (or of its ancestors).
     *  - changing the name, id, classes of a style (or of its ancestors).
     * 
     * But must by called manually for children when the notifier is called
     * with `style_modify_mask::path`.
     */
    void reload() noexcept
    {
        if (not _theme) {
            // The theme may not yet been set when the path is configured
            // or when the widget's tree is being setup.
            return;
        }

        for (auto i = size_t{0}; i != _loaded_attributes.size(); ++i) {
            _loaded_attributes[i] = _theme(path(), static_cast<style_pseudo_class>(i));
            _loaded_attributes[i].apply(_override_attributes);
        }

        _attributes.set(_loaded_attributes[std::to_underlying(_pseudo_class)]);
        update_layout_values();
        update_color_values();
    }

private:
    std::string _name;
    std::string _id;
    std::vector<std::string> _classes;

    style* _parent;
    hi::pixel_density _pixel_density;
    style_pseudo_class _pseudo_class;

    /** A function to retrieve style attributes from the current selected theme.
    */
    theme_type _theme;

    /** The attributes directly overridden by the developer for this widget's instance.
     */
    style_attributes _override_attributes;

    /** The attributes loaded from the theme, with overriden attributes applied.
     */
    std::array<style_attributes, style_pseudo_class_size> _loaded_attributes;

    /** The currently selected attributes from the current pseudo classes.
     */
    style_attributes _attributes;

    notifier<void(style_modify_mask)> _notifier;

    void update_color_values()
    {
        foreground_color = _attributes.foreground_color();
        background_color = _attributes.background_color();
        border_color = _attributes.border_color();
    }

    void update_layout_values()
    {
        width = _attributes.width() * _pixel_density;
        height = _attributes.height() * _pixel_density;
        margin_left = _attributes.margin_left() * _pixel_density;
        margin_bottom = _attributes.margin_bottom() * _pixel_density;
        margin_right = _attributes.margin_right() * _pixel_density;
        margin_top = _attributes.margin_top() * _pixel_density;
        padding_left = _attributes.padding_left() * _pixel_density;
        padding_bottom = _attributes.padding_bottom() * _pixel_density;
        padding_right = _attributes.padding_right() * _pixel_density;
        padding_top = _attributes.padding_top() * _pixel_density;
        border_width = _attributes.border_width() * _pixel_density;
        border_bottom_left_radius = _attributes.border_bottom_left_radius() * _pixel_density;
        border_bottom_right_radius = _attributes.border_bottom_right_radius() * _pixel_density;
        border_top_left_radius = _attributes.border_top_left_radius() * _pixel_density;
        border_top_right_radius = _attributes.border_top_right_radius() * _pixel_density;

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
        border_bottom_left_radius_px = border_bottom_left_radius.in(pixels);
        border_bottom_right_radius_px = border_bottom_right_radius.in(pixels);
        border_top_left_radius_px = border_top_left_radius.in(pixels);
        border_top_right_radius_px = border_top_right_radius.in(pixels);

        margins_px = hi::margins{margin_left_px, margin_bottom_px, margin_right_px, margin_top_px};
        padding_px = hi::margins{padding_left_px, padding_bottom_px, padding_right_px, padding_top_px};
        border_radius_px = hi::corner_radii{
            border_bottom_left_radius_px, border_bottom_right_radius_px, border_top_left_radius_px, border_top_right_radius_px};

        horizontal_alignment = _attributes.horizontal_alignment();
        vertical_alignment = _attributes.vertical_alignment();
    }

    void handle_state_change()
    {
        // background_color = _color_attributes[std::to_underlying(_state)].background_color;
        // foreground_color = _color_attributes[std::to_underlying(_state)].foreground_color;
        // border_color = _color_attributes[std::to_underlying(_state)].border_color;
    }
};

} // namespace v1
}