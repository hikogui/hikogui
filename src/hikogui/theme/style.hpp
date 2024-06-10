// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "hikogui/units/pixels.hpp"
#include "style_cascade.hpp"
#include "style_path.hpp"
#include "style_properties.hpp"
#include "style_computed_properties.hpp"
#include "style_pseudo_class.hpp"
#include "../text/text.hpp"
#include "../units/units.hpp"
#include "../dispatch/dispatch.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <functional>
#include <utility>
#include <cassert>
#include <array>
#include <vector>
#include <string>
#include <memory>

hi_export_module(hikogui.theme : style);

hi_export namespace hi {
inline namespace v1 {

class style : public style_computed_properties {
public:
    using notifier_type = notifier<void(style_modify_mask, bool)>;
    using callback_type = notifier_type::callback_type;
    using callback_proto = notifier_type::callback_proto;
    using properties_array_type = std::array<style_computed_properties, style_pseudo_class_size>;

    float width_px;
    float height_px;
    float font_size_px;
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

    /** The x-height of the primary font.
     */
    unit::pixels_f x_height;

    /** The cap-height of the primary font.
     */
    unit::pixels_f cap_height;

    /** The x-height of the primary font.
     */
    float x_height_px;

    /** The cap-height of the primary font.
     */
    float cap_height_px;

    extent2 size_px;
    hi::margins margins_px;
    hi::margins padding_px;
    hi::corner_radii border_radius_px;

    hi::alignment alignment;

    style(style const&) noexcept = delete;
    style(style&&) noexcept = delete;
    style& operator=(style const&) noexcept = delete;
    style& operator=(style&&) noexcept = delete;
    style() noexcept = default;

    /** Give the style a name.
     *
     * @note The first time this function is called the path of child widgets
     *       will not be updated. This is done so that the path can be set
     *       before child widgets are created; before children() must list
     *       these children.
     * @param name The name of the style.
     */
    void set_name(std::string name)
    {
        _name = name;
        _notifier(style_modify_mask::none, true);
    }

    [[nodiscard]] std::string const& name() const noexcept
    {
        return _name;
    }

    void set_id(std::string id)
    {
        _id = id;
        _notifier(style_modify_mask::none, true);
    }

    [[nodiscard]] std::string const& id() const noexcept
    {
        return _id;
    }

    void set_classes(std::vector<std::string> classes)
    {
        _classes = std::move(classes);
        _notifier(style_modify_mask::none, true);
    }

    [[nodiscard]] std::vector<std::string> const& classes() const noexcept
    {
        return _classes;
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
            std::tie(_override_properties, _id, _classes) = *optional_style;
            _notifier(style_modify_mask::none, true);

        } else if (optional_style.has_error()) {
            throw parse_error(optional_style.error());

        } else {
            std::unreachable();
        }
        return *this;
    }

    void set_pseudo_class(style_pseudo_class new_pseudo_class)
    {
        assert(std::to_underlying(new_pseudo_class) < _loaded_properties.size());

        auto const old_pseudo_class = std::exchange(_pseudo_class, new_pseudo_class);

        if (old_pseudo_class != _pseudo_class) {
            auto const i = std::to_underlying(old_pseudo_class);
            auto const j = std::to_underlying(new_pseudo_class);
            auto const mask = _pseudo_class_modifications[i + j * style_pseudo_class_size];

            update_properties(mask);
            _notifier(mask, false);
        }
    }

    [[nodiscard]] std::pair<style_path, properties_array_type const&> restyle(
        unit::pixel_density pixel_density,
        style_path const& parent_path,
        properties_array_type const& parent_properties) noexcept
    {
        auto path = parent_path;
        path.emplace_back(_name, _id, _classes);

        for (auto i = size_t{0}; i != style_pseudo_class_size; ++i) {
            auto p = get_style_properties(path, static_cast<style_pseudo_class>(i));
            p.apply(_override_properties);

            _loaded_properties[i] = p * pixel_density;
            _loaded_properties[i].inherit(parent_properties[i]);
        }

        for (auto i = size_t{0}; i != style_pseudo_class_size; ++i) {
            auto const& src = _loaded_properties[i];
            for (auto j = size_t{0}; j != style_pseudo_class_size; ++j) {
                auto const& dst = _loaded_properties[j];
                _pseudo_class_modifications[i + j * style_pseudo_class_size] = compare(src, dst);
            }
        }

        update_properties(style_modify_mask::all);
        _notifier(style_modify_mask::all, false);
        return {path, _loaded_properties};
    }

    /** Add a callback to the style.
     *
     * After the call the caller will take ownership of the returned callback
     * object.
     *
     * The `callback` object is a move-only RAII object that will automatically
     * unsubscribe the callback when the token is destroyed.
     *
     * @param flags The callback-flags used to determine how the @a callback is called.
     * @param callback A callable object with prototype void(style_modify_mask, bool) being called when the style changes.
     *                 The first argument is the mask of which properties have changed.
     *                 The second argument is true when the path of the style has changed.
     * @return A RAII object which when destroyed will unsubscribe the callback.
     */
    template<forward_of<callback_proto> Func>
    [[nodiscard]] callback_type subscribe(Func&& func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(std::forward<Func>(func), flags);
    }

private:
    std::string _name;
    std::string _id;
    std::vector<std::string> _classes;

    style_pseudo_class _pseudo_class;

    /** The properties directly overridden by the developer for this widget's instance.
     */
    style_properties _override_properties;

    /** The properties loaded from the query, with overriden properties applied.
     */
    properties_array_type _loaded_properties;

    /** A table for which properties are modified when switching between pseudo-classes.
     */
    std::array<style_modify_mask, style_pseudo_class_size * style_pseudo_class_size> _pseudo_class_modifications;

    notifier_type _notifier;

    void update_properties(style_modify_mask mask)
    {
        this->set_properties(_loaded_properties[std::to_underlying(_pseudo_class)], mask);

        if (to_bool(mask & style_modify_mask::size)) {
            width_px = width.in(unit::pixels);
            height_px = height.in(unit::pixels);
            size_px = extent2{width_px, height_px};
            font_size_px = font_size.in(unit::pixels_per_em);

            auto const &style = text_style.front();
            auto const &primary_font = get_font(style.font_chain().front());

            auto const scaled_font_size = font_size * style.scale();
            x_height = round_as(unit::pixels, primary_font.metrics.x_height * scaled_font_size);
            cap_height = round_as(unit::pixels, primary_font.metrics.cap_height * scaled_font_size);

            x_height_px = x_height.in(unit::pixels);
            cap_height_px = cap_height.in(unit::pixels);
        }

        if (to_bool(mask & style_modify_mask::margin)) {
            margin_left_px = margin_left.in(unit::pixels);
            margin_bottom_px = margin_bottom.in(unit::pixels);
            margin_right_px = margin_right.in(unit::pixels);
            margin_top_px = margin_top.in(unit::pixels);
            padding_left_px = padding_left.in(unit::pixels);
            padding_bottom_px = padding_bottom.in(unit::pixels);
            padding_right_px = padding_right.in(unit::pixels);
            padding_top_px = padding_top.in(unit::pixels);
            margins_px = hi::margins{margin_left_px, margin_bottom_px, margin_right_px, margin_top_px};
            padding_px = hi::margins{padding_left_px, padding_bottom_px, padding_right_px, padding_top_px};
        }

        if (to_bool(mask & style_modify_mask::weight)) {
            border_width_px = border_width.in(unit::pixels);
            border_bottom_left_radius_px = border_bottom_left_radius.in(unit::pixels);
            border_bottom_right_radius_px = border_bottom_right_radius.in(unit::pixels);
            border_top_left_radius_px = border_top_left_radius.in(unit::pixels);
            border_top_right_radius_px = border_top_right_radius.in(unit::pixels);
            border_radius_px = hi::corner_radii{
                border_bottom_left_radius_px,
                border_bottom_right_radius_px,
                border_top_left_radius_px,
                border_top_right_radius_px};
        }

        if (to_bool(mask & style_modify_mask::alignment)) {
            alignment = hi::alignment{horizontal_alignment, vertical_alignment};
        }
    }
};

} // namespace v1
}