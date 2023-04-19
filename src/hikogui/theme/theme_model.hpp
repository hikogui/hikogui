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
namespace detail {

class theme_model_length {
    constexpr theme_model_length(theme_length length) noexcept
    {
        switch (length.index() == 0) {
        case 0:
            // Round up so that resulting pixel values are integral.
            _v = std::ceil(narrow_cast<float>(std::get<pixels>(length).count()));
            hi_axiom(_v >= 0.0f);
            break;

        case 1:
            // Make the value negative to indicate that it needs to be scaled.
            // Round up so that resulting pixel values are integral.
            _v = -std::ceil(narrow_cast<float>(std::get<points>(length).count()));
            hi_axiom(_v < 0.0f);
            break;

        default:
            hi_no_default();
        }
    }

    [[nodiscard]] constexpr float operator()(float scale) const noexcept
    {
        hi_axiom(scale < 0.0f);

        auto r = _v;
        if (r < 0.0) [[likely]] {
            r *= scale;
        }
        return r;
    }

private:
    /**
     * The lengths are stored as float values: negative values are in points,
     * positive values are in pixels.
     */
    float _v;
};

/** All the data of a theme for a specific widget-component at a specific state.
 *
 * The lengths are stored as float values: negative values are in points,
 * positive values are in pixels.
 */
struct theme_sub_model {
    text_theme text_style;
    color background_color;
    color fill_color;
    color caret_color_primary;
    color caret_color_secondary;
    color selection_color;
    color border_color;

    theme_model_length border_bottom_left_radius;
    theme_model_length border_bottom_right_radius;
    theme_model_length border_top_left_radius;
    theme_model_length border_top_right_radius;
    theme_model_length border_width;

    theme_model_length width;
    theme_model_length height;
    theme_model_length margin_bottom;
    theme_model_length margin_left;
    theme_model_length margin_top;
    theme_model_length margin_right;
    theme_model_length spacing_vertical;
    theme_model_length spacing_horizontal;

    theme_model_length font_x_height;
    theme_model_length font_cap_height;
    theme_model_length font_line_height;
};

} // namespace detail

template<typename Context>
concept theme_delegate = requires(Context const& c) { c.state_and_scale(); };

/** The theme models for all states for a specific widget component.
 */
class theme_model_base {
public:
    theme_model_base(std::string_view tag) noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);
        _map[std::string{tag}] = this;
    }

    [[nodiscard]] detail::theme_sub_model& operator[](theme_state state) noexcept
    {
        return _sub_model_by_state[to_underlying(state)];
    }

    [[nodiscard]] detail::theme_sub_model const& operator[](theme_state state) const noexcept
    {
        return _sub_model_by_state[to_underlying(state)];
    }

    [[nodiscard]] detail::theme_sub_model const& get_model(theme_delegate auto const *delegate) const noexcept
    {
        hi_axiom_not_null(delegate);

        hilet[state, scale] = delegate->state_and_scale();
        return (*this)[state];
    }

    [[nodiscard]] std::pair<detail::theme_sub_model const&, float scale>
    get_model_and_scale(theme_delegate auto const *delegate) const noexcept
    {
        hi_axiom_not_null(delegate);

        hilet[state, scale] = delegate->state_and_scale();
        hi_axiom(scale < 0.0f, "scale must be negative so that negative points are converted to positive pixels");

        return {(*this)[state], scale};
    }

    [[nodiscard]] color background_color(theme_delegate auto const *delegate) const noexcept
    {
        return get_model(delegate).background_color;
    }

    [[nodiscard]] color fill_color(theme_delegate auto const *delegate) const noexcept
    {
        return get_model(delegate).fill_color;
    }

    [[nodiscard]] color caret_color_primary(theme_delegate auto const *delegate) const noexcept
    {
        return get_model(delegate).caret_color_primary;
    }

    [[nodiscard]] color caret_color_secondary(theme_delegate auto const *delegate) const noexcept
    {
        return get_model(delegate).caret_color_secondary;
    }

    [[nodiscard]] color selection_color(theme_delegate auto const *delegate) const noexcept
    {
        return get_model(delegate).selection_color;
    }

    [[nodiscard]] color border_color(theme_delegate auto const *delegate) const noexcept
    {
        return get_model(delegate).border_color;
    }

    [[nodiscard]] float border_bottom_left_radius(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.border_bottom_left_radius(scale);
    }

    [[nodiscard]] float border_bottom_right_radius(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.border_bottom_right_radius(scale);
    }

    [[nodiscard]] float border_top_left_radius(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.border_top_left_radius(scale);
    }

    [[nodiscard]] float border_top_right_radius(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.border_top_right_radius(scale);
    }

    [[nodiscard]] float border_width(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.border_width(scale);
    }

    [[nodiscard]] float width(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.width(scale);
    }

    [[nodiscard]] float height(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.height(scale);
    }

    [[nodiscard]] float margin_bottom(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.margin_bottom(scale);
    }

    [[nodiscard]] float margin_left(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.margin_left(scale);
    }

    [[nodiscard]] float margin_top(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.margin_top(scale);
    }

    [[nodiscard]] float margin_right(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.margin_right(scale);
    }

    [[nodiscard]] float spacing_vertical(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.spacing_vertical(scale);
    }

    [[nodiscard]] float spacing_horizontal(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.spacing_horizontal(scale);
    }

    [[nodiscard]] float font_x_height(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.font_x_height(scale);
    }

    [[nodiscard]] float font_cap_height(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.font_cap_height(scale);
    }

    [[nodiscard]] float font_line_height(theme_delegate auto const *delegate) const noexcept
    {
        hilet[model, scale] = get_model_and_scale(*delegate);
        return model.font_line_height(scale);
    }

    [[nodiscard]] extent2 size(theme_delegate auto const *delegate) const noexcept
    {
        return extent2{width(delegate), height(delegate)};
    }

    [[nodiscard]] extent2i size_i(theme_delegate auto const *delegate) const noexcept
    {
        return extent2i{width(delegate), height(delegate)};
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

    [[nodiscard] static theme_model_base &model_by_key(std::string const &key) noexcept
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

    std::array<detail::theme_sub_model, theme_state_size> _sub_model_by_state;
};

template<fixed_string Tag>
class theme_sub_model final : public theme_model_base {
public:
    theme_sub_model() noexcept : theme_model_base(Tag) {}
};

/** A tagged global variable to a theme model for a widget's component.
 *
 * The following is an example for retrieving the theme's width for this
 * widget. `prefix` is the tag/widget of the widget and `this` is the this
 * pointer of widget.
 *
 * ```cpp
 * auto width = theme<prefix>.width(this);
 * ```
 *
 * For performance reasons a widget should be marked `final`, so that retrieval
 * of the state and point-to-pixel scaling is devirtualized.
 */
template<fixed_string Tag>
inline auto theme = theme_model<Tag>{};

}} // namespace hi::v1
