// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "style_path.hpp"
#include "style_attributes.hpp"
#include "style_pseudo_class.hpp"
#include "../units/units.hpp"
#include "../dispatch/dispatch.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <functional>
#include <utility>

hi_export_module(hikogui.theme : style);

hi_export namespace hi {
inline namespace v1 {

class style {
public:
    using query_attributes_type = std::function<style_attributes(style_path, style_pseudo_class)>;
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

    hi::margins margins_px;
    hi::margins padding_px;
    hi::corner_radii border_radius_px;

    hi::horizontal_alignment horizontal_alignment;
    hi::vertical_alignment vertical_alignment;

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
        _name = std::move(new_name);
        reload(true);
    }

    [[nodiscard]] std::string const& name() const noexcept
    {
        return _name;
    }

    void set_id(std::string id)
    {
        _id = std::move(id);
        reload(true);
    }

    [[nodiscard]] std::string const& id() const noexcept
    {
        return _id;
    }

    void set_classes(std::vector<std::string> classes)
    {
        _classes = std::move(classes);
        reload(true);
    }

    [[nodiscard]] std::vector<std::string> const& classes() const noexcept
    {
        return _classes;
    }

    void set_parent_path(style_path new_parent_path) noexcept
    {
        _parent_path = new_parent_path;
        reload(true);
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
            std::tie(_override_attributes, _id, _classes) = *optional_style;
            reload(true);
        } else if (optional_style.has_error()) {
            throw parse_error(optional_style.error());
        } else {
            hi_no_default();
        }
        return *this;
    }

    [[nodiscard]] query_attributes_type const& query_attributes() const noexcept
    {
        return _query_attributes;
    }

    void set_query_attributes(query_attributes_type new_query_attributes)
    {
        _query_attributes = std::move(new_query_attributes);
        reload(false);
    }

    [[nodiscard]] unit::pixel_density pixel_density() const noexcept
    {
        return _pixel_density;
    }

    void set_pixel_density(unit::pixel_density new_pixel_density)
    {
        _pixel_density = new_pixel_density;
        update_attributes(style_modify_mask::pixel_density);
        _notifier(style_modify_mask::pixel_density, false);
    }

    void set_pseudo_class(style_pseudo_class new_pseudo_class)
    {
        auto const old_pseudo_class = std::exchange(_pseudo_class, new_pseudo_class);

        auto const i = std::to_underlying(old_pseudo_class);
        auto const j = std::to_underlying(new_pseudo_class);
        auto const mask = _pseudo_class_modifications[i + j * style_pseudo_class_size];

        update_attributes(mask);
        _notifier(mask, false);
    }

    /** Reload the style attributes from the current theme.
     *
     * Reload is called automatically after:
     *  - changing the query_attributes function.
     *  - changing the parent of the style (or of its ancestors).
     *  - changing the name, id, classes of a style (or of its ancestors).
     *
     * But must by called manually for children when the notifier is called
     * with `style_modify_mask::path`.
     */
    void reload(bool path_has_changed = false) noexcept
    {
        if (not _query_attributes) {
            // The query_attributes function may not yet been set when the
            // path is configured or when the widget's tree is being setup.
            _notifier(style_modify_mask::none, path_has_changed);
            return;
        }

        for (auto i = size_t{0}; i != style_pseudo_class_size; ++i) {
            _loaded_attributes[i] = _query_attributes(path(), static_cast<style_pseudo_class>(i));
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

    /** A function to retrieve style attributes from the current selected query_attributes.
     */
    query_attributes_type _query_attributes;

    /** The attributes directly overridden by the developer for this widget's instance.
     */
    style_attributes _override_attributes;

    /** The attributes loaded from the query_attributes, with overriden attributes applied.
     */
    std::array<style_attributes, style_pseudo_class_size> _loaded_attributes;

    /** A table for which attributes are modified when switching between pseudo-classes.
     */
    std::array<style_modify_mask, style_pseudo_class_size * style_pseudo_class_size> _pseudo_class_modifications;

    /** The currently selected attributes from the current pseudo classes.
     */
    style_attributes _attributes;

    notifier_type _notifier;

    void update_attributes(style_modify_mask mask)
    {
        if (to_bool(mask & style_modify_mask::color)) {
            foreground_color = _attributes.foreground_color();
            background_color = _attributes.background_color();
            border_color = _attributes.border_color();
            accent_color = _attributes.accent_color();
        }

        if (to_bool(mask & style_modify_mask::size)) {
            width = _attributes.width() * _pixel_density;
            height = _attributes.height() * _pixel_density;
            width_px = width.in(unit::pixels);
            height_px = height.in(unit::pixels);
        }

        if (to_bool(mask & style_modify_mask::margin)) {
            margin_left = _attributes.margin_left() * _pixel_density;
            margin_bottom = _attributes.margin_bottom() * _pixel_density;
            margin_right = _attributes.margin_right() * _pixel_density;
            margin_top = _attributes.margin_top() * _pixel_density;
            padding_left = _attributes.padding_left() * _pixel_density;
            padding_bottom = _attributes.padding_bottom() * _pixel_density;
            padding_right = _attributes.padding_right() * _pixel_density;
            padding_top = _attributes.padding_top() * _pixel_density;
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
            border_width = _attributes.border_width() * _pixel_density;
            border_bottom_left_radius = _attributes.border_bottom_left_radius() * _pixel_density;
            border_bottom_right_radius = _attributes.border_bottom_right_radius() * _pixel_density;
            border_top_left_radius = _attributes.border_top_left_radius() * _pixel_density;
            border_top_right_radius = _attributes.border_top_right_radius() * _pixel_density;
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
            horizontal_alignment = _attributes.horizontal_alignment();
            vertical_alignment = _attributes.vertical_alignment();
        }
    }
};

} // namespace v1
}