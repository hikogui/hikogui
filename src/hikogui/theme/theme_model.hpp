// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_length.hpp"
#include "../geometry/module.hpp"
#include "../color/module.hpp"
#include "../text/module.hpp"
#include <array>

namespace hi { inline namespace v1 {

struct theme_model {
    color background_color;
    color fill_color;
    color caret_color_primary;
    color caret_color_secondary;
    color selection_color;

    color border_color;
    theme_length border_bottom_left_radius;
    theme_length border_bottom_right_radius;
    theme_length border_top_left_radius;
    theme_length border_top_right_radius;
    theme_length border_width;

    theme_length width;
    theme_length height;
    theme_length margin_bottom;
    theme_length margin_left;
    theme_length maring_top;
    theme_length margin_right;
    theme_length spacing_vertical;
    theme_length spacing_horizontal;

    text_theme text_style;

    /** The font-x-height in pt/px.
     *
     * This variable is automatically calculated based on the `text_style`.
     */
    theme_length font_x_height;

    /** The font-cap-height in pt/px.
     *
     * This variable is automatically calculated based on the `text_style`.
     */
    theme_length font_cap_height;

    /** The font-line-height in pt/px.
     *
     * This variable is automatically calculated based on the `text_style`.
     */
    theme_length font_line_height;
};

namespace detail {

class tagged_theme_model_base {
public:
    tagged_theme_model_base(std::string_view tag) noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);
        _map[std::string{tag}] = this;
    }

    [[nodiscard]] theme_model& operator[](theme_state state) noexcept
    {
        return _model_by_state[to_underlying(state)];
    }

    [[nodiscard]] static std::vector<std::string> model_keys() noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);
        auto r = std::vector<std::string>{};
        r.reserve(_map.size());

        for (auto& [key, value] : _map) {
            r.push_back(key);
        }

        return r;
    }

    [[nodiscard] static tagged_theme_model_base &model_by_key(std::string const &key) noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);
        auto it = _map.find(key);

        hi_assert(it != _map.end());
        auto *ptr = it->second;

        hi_assert_not_null(ptr);
        return *ptr;
    }

private:
    // Theoretically it possible for global variable initialization to be
    // done from multiple threads. Practically this may happen when loading
    // libraries at run-time.
    inline static unfair_mutex _map_mutex;
    inline static std::map<std::string, tagged_theme_model_base *> _map;

    std::array<theme_model, theme_state_size> _model_by_state;
};

template<fixed_string Tag>
class tagged_theme_model final : public tagged_theme_model_base {
public:
    tagged_theme_model() noexcept : tagged_theme_model_base(Tag) {}
};

template<fixed_string Tag>
inline auto global_theme_model = tagged_theme_model<Tag>{};

} // namespace detail

template<typename Context>
concept theme_delegate = requires(Context const& c) { c.ptpx_and_state(); };

template<fixed_string Tag>
class theme {
public:
    /**
     *
     * In a widget `theme` is expected to be used a temporary value from which
     * only a single member variable is selected. If this constructor does not
     * get inlined it will not be optimized to perform only a single
     * calculation.
     */
    hi_force_inline theme(theme_delegate auto const& delegate) noexcept
    {
        hilet[ptpx, state] = delegate.ptpx_and_state();

        hilet& model = global_theme_model<Tag>;

        // Font sizes can only be in points or pixels, not in Em.
        // The empx-scale is the font's line-height.
        font_x_height = model.font_x_height.to_pixels(ptpx, 1.0);
        font_cap_height = model.font_cap_height.to_pixels(ptpx, 1.0);
        font_line_height = model.font_line_height.to_pixels(ptpx, 1.0);
        hilet empx = font_line_height;

        border_bottom_left_radius = model.border_bottom_left_radius.to_pixels(ptpx, empx);
        border_bottom_right_radius = model.border_bottom_right_radius.to_pixels(ptpx, empx);
        border_top_left_radius = model.border_top_left_radius.to_pixels(ptpx, empx);
        border_top_right_radius = model.border_top_left_radius.to_pixels(ptpx, empx);
        border_radius = {border_bottom_left_radius, border_bottom_right_radius, border_top_left_radius, border_top_right_radius};
        border_width = model.border_width.to_pixels(ptpx, empx);

        width = model.width.to_pixels(ptpx, empx);
        height = model.height.to_pixels(ptpx, empx);
        margin_bottom = model.margin_bottom.to_pixels(ptpx, empx);
        margin_left = model.margin_left.to_pixels(ptpx, empx);
        maring_top = model.maring_top.to_pixels(ptpx, empx);
        margin_right = model.margin_right.to_pixels(ptpx, empx);
        spacing_vertical = model.spacing_vertical.to_pixels(ptpx, empx);
        spacing_horizontal = model.spacing_horizontal.to_pixels(ptpx, empx);

        background_color = model.background_color;
        fill_color = model.background_color;
        caret_color_primary = model.background_color;
        caret_color_secondary = model.background_color;
        selection_color = model.background_color;
        border_color = model.background_color;

        text_style = std::addressof(model.text_style);
    }

    color background_color;
    color fill_color;
    color caret_color_primary;
    color caret_color_secondary;
    color selection_color;

    color border_color;
    double border_bottom_left_radius;
    double border_bottom_right_radius;
    double border_top_left_radius;
    double border_top_right_radius;
    corner_radii border_radius;
    double border_width;

    double width;
    double height;
    double margin_bottom;
    double margin_left;
    double maring_top;
    double margin_right;
    double spacing_vertical;
    double spacing_horizontal;

    text_theme *text_style;

    /** The font-x-height in pt/px.
     *
     * This variable is automatically calculated based on the `text_style`.
     */
    double font_x_height;

    /** The font-cap-height in pt/px.
     *
     * This variable is automatically calculated based on the `text_style`.
     */
    double font_cap_height;

    /** The font-line-height in pt/px.
     *
     * This variable is automatically calculated based on the `text_style`.
     */
    double font_line_height;
};

}} // namespace hi::v1
