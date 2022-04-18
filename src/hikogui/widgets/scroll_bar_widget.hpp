// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GUI/mouse_event.hpp"
#include "../geometry/axis.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi::inline v1 {

template<axis Axis>
class scroll_bar_widget final : public widget {
public:
    using super = widget;

    static constexpr hi::axis axis = Axis;

    observable<float> offset;
    observable<float> aperture;
    observable<float> content;

    template<typename Content, typename Aperture, typename Offset>
    scroll_bar_widget(gui_window &window, widget *parent, Content &&content, Aperture &&aperture, Offset &&offset) noexcept :
        widget(window, parent),
        content(std::forward<Content>(content)),
        aperture(std::forward<Aperture>(aperture)),
        offset(std::forward<Offset>(offset))
    {
        // clang-format off
        _content_cbt = this->content.subscribe([&](auto...){ request_relayout(); });
        _aperture_cbt = this->aperture.subscribe([&](auto...){ request_relayout(); });
        _offset_cbt = this->offset.subscribe([&](auto...){ request_relayout(); });
        // clang-format on
    }

    ~scroll_bar_widget() {}

    widget_constraints const &set_constraints() noexcept override
    {
        _layout = {};

        // The minimum size is twice the length of the slider, which is twice the theme().size()
        if constexpr (axis == axis::vertical) {
            return _constraints = {
                       {theme().icon_size, theme().size * 4.0f},
                       {theme().icon_size, theme().size * 4.0f},
                       {theme().icon_size, 32767.0f}};
        } else {
            return _constraints = {
                       {theme().size * 4.0f, theme().icon_size},
                       {theme().size * 4.0f, theme().icon_size},
                       {32767.0f, theme().icon_size}};
        }
    }

    void set_layout(widget_layout const &layout) noexcept override
    {
        _layout = layout;

        // Calculate the position of the slider.
        hilet slider_offset = *offset * travel_vs_hidden_content_ratio();
        if constexpr (axis == axis::vertical) {
            _slider_rectangle = aarectangle{0.0f, slider_offset, layout.width(), slider_length()};
        } else {
            _slider_rectangle = aarectangle{slider_offset, 0.0f, slider_length(), layout.height()};
        }
    }

    void draw(draw_context const &context) noexcept override
    {
        if (*visible and overlaps(context, layout())) {
            draw_rails(context);
            draw_slider(context);
        }
    }

    hitbox hitbox_test(point3 position) const noexcept override
    {
        hi_axiom(is_gui_thread());

        if (*visible and *enabled and layout().contains(position) and _slider_rectangle.contains(position)) {
            return {this, position};
        } else {
            return {};
        }
    }

    [[nodiscard]] bool handle_event(mouse_event const &event) noexcept
    {
        hi_axiom(is_gui_thread());
        auto handled = super::handle_event(event);

        if (event.cause.leftButton) {
            handled = true;

            switch (event.type) {
                using enum mouse_event::Type;
            case ButtonDown:
                // Record the original scroll-position before the drag starts.
                _offset_before_drag = *offset;
                break;

            case Drag: {
                // The distance the slider has to move relative to the slider position at the
                // start of the drag.
                hilet slider_movement = axis == axis::vertical ? event.delta().y() : event.delta().x();
                hilet content_movement = slider_movement * hidden_content_vs_travel_ratio();
                hilet new_offset = _offset_before_drag + content_movement;
                offset = clamp_offset(new_offset);
            } break;

            default:;
            }
        }
        return handled;
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return false;
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        return theme().color(semantic_color::fill, semantic_layer);
    }

    [[nodiscard]] color foreground_color() const noexcept override
    {
        if (*hover) {
            return theme().color(semantic_color::fill, semantic_layer + 2);
        } else {
            return theme().color(semantic_color::fill, semantic_layer + 1);
        }
    }

private:
    aarectangle _slider_rectangle;

    float _offset_before_drag;

    typename decltype(content)::token_type _content_cbt;
    typename decltype(aperture)::token_type _aperture_cbt;
    typename decltype(offset)::token_type _offset_cbt;

    /** Create a new offset value.
    * 
    * Clamp the new offset value by the amount of scrollable distance.
    */
    [[nodiscard]] float clamp_offset(float new_offset) const noexcept
    {
        hilet scrollable_distance = std::max(0.0f, *content - *aperture);
        return std::clamp(new_offset, 0.0f, scrollable_distance);
    }

    [[nodiscard]] float rail_length() const noexcept
    {
        hi_axiom(is_gui_thread());
        return axis == axis::vertical ? layout().height() : layout().width();
    }

    [[nodiscard]] float slider_length() const noexcept
    {
        hi_axiom(is_gui_thread());

        hilet content_aperture_ratio = *content != 0.0f ? *aperture / *content : 1.0f;
        hilet rail_length_ = rail_length();
        return std::clamp(rail_length_ * content_aperture_ratio, theme().size * 2.0f, rail_length_);
    }

    /** The amount of travel that the slider can make.
     */
    [[nodiscard]] float slider_travel_range() const noexcept
    {
        hi_axiom(is_gui_thread());
        return rail_length() - slider_length();
    }

    /** The amount of content hidden from view.
     */
    [[nodiscard]] float hidden_content() const noexcept
    {
        hi_axiom(is_gui_thread());
        return *content - *aperture;
    }

    /** Get the ratio of the hidden content vs the slider travel range.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float hidden_content_vs_travel_ratio() const noexcept
    {
        hi_axiom(is_gui_thread());

        hilet _slider_travel_range = slider_travel_range();
        return _slider_travel_range != 0.0f ? hidden_content() / _slider_travel_range : 0.0f;
    }

    /** Get the ratio of the slider travel range vs hidden content.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float travel_vs_hidden_content_ratio() const noexcept
    {
        hi_axiom(is_gui_thread());

        hilet _hidden_content = hidden_content();
        return _hidden_content != 0.0f ? slider_travel_range() / _hidden_content : 0.0f;
    }

    void draw_rails(draw_context const &context) noexcept
    {
        hilet corner_radii =
            axis == axis::vertical ? hi::corner_radii{layout().width() * 0.5f} : hi::corner_radii{layout().height() * 0.5f};
        context.draw_box(layout(), layout().rectangle(), background_color(), corner_radii);
    }

    void draw_slider(draw_context const &context) noexcept
    {
        hilet corner_radii = axis == axis::vertical ? hi::corner_radii{_slider_rectangle.width() * 0.5f} :
                                                       hi::corner_radii{_slider_rectangle.height() * 0.5f};

        context.draw_box(layout(), translate_z(0.1f) * _slider_rectangle, foreground_color(), corner_radii);
    }
};

using horizontal_scroll_bar_widget = scroll_bar_widget<axis::horizontal>;
using vertical_scroll_bar_widget = scroll_bar_widget<axis::vertical>;

} // namespace hi::inline v1
