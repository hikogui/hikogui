// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "style_path.hpp"
#include "style_attributes.hpp"
#include "style_pseudo_class.hpp"
#include "style_query.hpp"
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

class style {
public:
    using notifier_type = notifier<void(style_modify_mask, bool)>;
    using callback_type = notifier_type::callback_type;
    using callback_proto = notifier_type::callback_proto;

    unit::pixels_f width;
    unit::pixels_f height;
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

    extent2 size_px;
    hi::margins margins_px;
    hi::margins padding_px;
    hi::corner_radii border_radius_px;

    hi::horizontal_alignment horizontal_alignment;
    hi::vertical_alignment vertical_alignment;
    hi::alignment alignment;

    color foreground_color;
    color background_color;
    color border_color;
    color accent_color;

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
    void set_name(std::string new_name)
    {
        auto const old_name = std::exchange(_name, std::move(new_name));
        if (old_name != _name) {
            reload(true);
        }
    }

    [[nodiscard]] std::string const& name() const noexcept
    {
        return _name;
    }

    void set_id(std::string id)
    {
        auto const old_id = std::exchange(_id, std::move(id));
        if (old_id != _id) {
            reload(true);
        }
    }

    [[nodiscard]] std::string const& id() const noexcept
    {
        return _id;
    }

    void set_classes(std::vector<std::string> classes)
    {
        auto const old_classes = std::exchange(_classes, std::move(classes));
        if (old_classes != _classes) {
            reload(true);
        }
    }

    [[nodiscard]] std::vector<std::string> const& classes() const noexcept
    {
        return _classes;
    }

    void set_parent_path(style_path new_parent_path) noexcept
    {
        auto const old_parent_path = std::exchange(_parent_path, std::move(new_parent_path));
        if (old_parent_path != _parent_path) {
            reload(true);
        }
    }

    [[nodiscard]] style_path const &parent_path() const noexcept
    {
        return _parent_path;
    }

    /** The path without checks for validity.
     * 
     * @note This function is used to propagate the path to children.
     */
    [[nodiscard]] style_path unsafe_path() const
    {
        auto r = parent_path();
        r.emplace_back(_name, _id, _classes);
        return r;
    }

    /** The path of the style used to look up attributes from a theme.
     * 
     * @note It is undefined behavior to call this function before calling
     *       style::set_name().
    */
    [[nodiscard]] style_path path() const
    {
        hi_assert(not _name.empty(), "style::set_name() must be called before calling style::path()");
        return unsafe_path();
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
            auto [new_override_attributes, new_id, new_classes] = *optional_style;
            auto const old_override_attributes = std::exchange(_override_attributes, std::move(new_override_attributes));
            auto const old_id = std::exchange(_id, std::move(new_id));
            auto const old_classes = std::exchange(_classes, std::move(new_classes));

            auto const path_has_changed = (old_id != _id) or (old_classes != _classes);
            reload(path_has_changed);

        } else if (optional_style.has_error()) {
            throw parse_error(optional_style.error());

        } else {
            std::unreachable();
        }
        return *this;
    }

    [[nodiscard]] std::shared_ptr<style_query> const &query() const noexcept
    {
        return _query;
    }

    void set_query(std::shared_ptr<style_query> const &new_query)
    {
        // If either are a nullptr, reload() will be fast.
        if (_query == nullptr or new_query == nullptr or *_query != *new_query) {
            _query = new_query;
            reload(false);
        }
    }

    [[nodiscard]] unit::pixel_density pixel_density() const noexcept
    {
        return _pixel_density;
    }

    void set_pixel_density(unit::pixel_density new_pixel_density)
    {
        auto const old_pixel_density = std::exchange(_pixel_density, new_pixel_density);
        if (old_pixel_density != _pixel_density) {
            update_attributes(style_modify_mask::pixel_density);
            _notifier(style_modify_mask::pixel_density, false);
        }        
    }

    void set_pseudo_class(style_pseudo_class new_pseudo_class)
    {
        assert(std::to_underlying(new_pseudo_class) < _loaded_attributes.size());

        auto const old_pseudo_class = std::exchange(_pseudo_class, new_pseudo_class);

        if (old_pseudo_class != _pseudo_class) {
            auto const i = std::to_underlying(old_pseudo_class);
            auto const j = std::to_underlying(new_pseudo_class);
            auto const mask = _pseudo_class_modifications[i + j * style_pseudo_class_size];

            update_attributes(mask);
            _notifier(mask, false);
        }
    }

    [[nodiscard]] style_attributes const& attributes() const noexcept
    {
        return _loaded_attributes[std::to_underlying(_pseudo_class)];
    }

    /** Reload the style attributes from the current theme.
     *
     * Reload is called automatically after:
     *  - changing the query function.
     *  - changing the parent of the style (or of its ancestors).
     *  - changing the name, id, classes of a style (or of its ancestors).
     *
     * But must by called manually for children when the notifier is called
     * with `style_modify_mask::path`.
     */
    void reload(bool path_has_changed = false) noexcept
    {
        if (not _query) {
            // The query function may not yet been set when the
            // path is configured or when the widget's tree is being setup.
            _notifier(style_modify_mask::none, path_has_changed);
            return;
        }

        for (auto i = size_t{0}; i != style_pseudo_class_size; ++i) {
            _loaded_attributes[i] = _query->get_attributes(path(), static_cast<style_pseudo_class>(i));
            _loaded_attributes[i].apply(_override_attributes);
        }

        for (auto i = size_t{0}; i != style_pseudo_class_size; ++i) {
            auto const& src = _loaded_attributes[i];
            for (auto j = size_t{0}; j != style_pseudo_class_size; ++j) {
                auto const& dst = _loaded_attributes[j];
                _pseudo_class_modifications[i + j * style_pseudo_class_size] = compare(src, dst);
            }
        }

        update_attributes(style_modify_mask::all);
        _notifier(style_modify_mask::all, path_has_changed);
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
     *                 The first argument is the mask of which attributes have changed.
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

    style_path _parent_path;
    unit::pixel_density _pixel_density;
    style_pseudo_class _pseudo_class;

    /** A function to retrieve style attributes from the current selected query.
     */
    std::shared_ptr<style_query> _query;

    /** The attributes directly overridden by the developer for this widget's instance.
     */
    style_attributes _override_attributes;

    /** The attributes loaded from the query, with overriden attributes applied.
     */
    std::array<style_attributes, style_pseudo_class_size> _loaded_attributes;

    /** A table for which attributes are modified when switching between pseudo-classes.
     */
    std::array<style_modify_mask, style_pseudo_class_size * style_pseudo_class_size> _pseudo_class_modifications;

    notifier_type _notifier;

    void update_attributes(style_modify_mask mask)
    {
        if (to_bool(mask & style_modify_mask::color)) {
            foreground_color = attributes().foreground_color();
            background_color = attributes().background_color();
            border_color = attributes().border_color();
            accent_color = attributes().accent_color();
        }

        if (to_bool(mask & style_modify_mask::size)) {
            width = ceil_as(unit::pixels, attributes().width() * _pixel_density);
            height = ceil_as(unit::pixels, attributes().height() * _pixel_density);
            width_px = width.in(unit::pixels);
            height_px = height.in(unit::pixels);
            size_px = extent2{width_px, height_px};
        }

        if (to_bool(mask & style_modify_mask::margin)) {
            margin_left = round_as(unit::pixels, attributes().margin_left() * _pixel_density);
            margin_bottom = round_as(unit::pixels, attributes().margin_bottom() * _pixel_density);
            margin_right = round_as(unit::pixels, attributes().margin_right() * _pixel_density);
            margin_top = round_as(unit::pixels, attributes().margin_top() * _pixel_density);
            padding_left = round_as(unit::pixels, attributes().padding_left() * _pixel_density);
            padding_bottom = round_as(unit::pixels, attributes().padding_bottom() * _pixel_density);
            padding_right = round_as(unit::pixels, attributes().padding_right() * _pixel_density);
            padding_top = round_as(unit::pixels, attributes().padding_top() * _pixel_density);
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
            border_width = std::max(floor_as(unit::pixels, attributes().border_width() * _pixel_density), unit::pixels(1.0f));
            border_bottom_left_radius = round_as(unit::pixels, attributes().border_bottom_left_radius() * _pixel_density);
            border_bottom_right_radius = round_as(unit::pixels, attributes().border_bottom_right_radius() * _pixel_density);
            border_top_left_radius = round_as(unit::pixels, attributes().border_top_left_radius() * _pixel_density);
            border_top_right_radius = round_as(unit::pixels, attributes().border_top_right_radius() * _pixel_density);
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
            horizontal_alignment = attributes().horizontal_alignment();
            vertical_alignment = attributes().vertical_alignment();
            alignment = hi::alignment{horizontal_alignment, vertical_alignment};
        }
    }
};

} // namespace v1
}