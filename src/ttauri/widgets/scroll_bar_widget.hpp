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

namespace tt {

template<axis Axis>
class scroll_bar_widget final : public widget {
public:
    using super = widget;

    static constexpr tt::axis axis = Axis;

    template<typename Content, typename Aperture, typename Offset>
    scroll_bar_widget(
        gui_window &window, widget *parent,
        Content &&content,
        Aperture &&aperture,
        Offset &&offset) noexcept :
        widget(window, parent),
        content(std::forward<Content>(content)),
        aperture(std::forward<Aperture>(aperture)),
        offset(std::forward<Offset>(offset))
    {
        _content_callback = this->content.subscribe([this](auto...) {
            request_relayout();
        });
        _aperture_callback = this->aperture.subscribe([this](auto...) {
            request_relayout();
        });
        _offset_callback = this->offset.subscribe([this](auto...) {
            request_relayout();
        });
    }

    ~scroll_bar_widget() {}

    [[nodiscard]] bool constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (super::constrain(display_time_point, need_reconstrain)) {
            if constexpr (axis == axis::vertical) {
                _minimum_size = _preferred_size = {theme().icon_size, theme().large_size};
                _maximum_size = {theme().icon_size, 32767.0f};
            } else {
                _minimum_size = _preferred_size = {theme().large_size, theme().icon_size};
                _maximum_size = {32767.0f, theme().icon_size};
            }
            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
            return true;
        } else {
            return false;
        }
    }

    void layout(layout_context const &context, bool need_layout) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (compare_then_assign(_layout, context) or need_layout) {
            tt_axiom(*content != 0.0f);

            // Calculate the position of the slider.
            ttlet slider_offset = *offset * travel_vs_hidden_content_ratio();

            if constexpr (axis == axis::vertical) {
                slider_rectangle =
                    aarectangle{rectangle().left(), rectangle().bottom() + slider_offset, rectangle().width(), slider_length()};
            } else {
                slider_rectangle =
                    aarectangle{rectangle().left() + slider_offset, rectangle().bottom(), slider_length(), rectangle().height()};
            }
            request_redraw();
        }
    }

    void draw(draw_context const &context) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (visible and overlaps(context, _layout)) {
            draw_rails(context);
            draw_slider(context);
        }
    }

    hitbox hitbox_test(point3 position) const noexcept override
    {
        tt_axiom(is_gui_thread());

        if (visible and _layout.hit_rectangle.contains(position) and slider_rectangle.contains(position)) {
            return hitbox{this, position};
        } else {
            return hitbox{};
        }
    }

    [[nodiscard]] bool handle_event(mouse_event const &event) noexcept
    {
        tt_axiom(is_gui_thread());
        auto handled = super::handle_event(event);

        if (event.cause.leftButton) {
            handled = true;

            switch (event.type) {
                using enum mouse_event::Type;
            case ButtonDown:
                // Record the original scroll-position before the drag starts.
                offset_before_drag = *offset;
                break;

            case Drag: {
                // The distance the slider has to move relative to the slider position at the
                // start of the drag.
                ttlet slider_movement = axis == axis::vertical ? event.delta().y() : event.delta().x();
                ttlet content_movement = slider_movement * hidden_content_vs_travel_ratio();
                offset = offset_before_drag + content_movement;
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
        return theme().color(theme_color::fill, semantic_layer);
    }

    [[nodiscard]] color foreground_color() const noexcept override
    {
        if (hover) {
            return theme().color(theme_color::fill, semantic_layer + 2);
        } else {
            return theme().color(theme_color::fill, semantic_layer + 1);
        }
    }

private:
    observable<float> offset;
    observable<float> aperture;
    observable<float> content;

    typename decltype(offset)::callback_ptr_type _offset_callback;
    typename decltype(aperture)::callback_ptr_type _aperture_callback;
    typename decltype(content)::callback_ptr_type _content_callback;

    aarectangle slider_rectangle;

    float offset_before_drag;

    [[nodiscard]] float rail_length() const noexcept
    {
        tt_axiom(is_gui_thread());
        return axis == axis::vertical ? rectangle().height() : rectangle().width();
    }

    [[nodiscard]] float slider_length() const noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet content_aperture_ratio = *aperture / *content;
        return std::max(rail_length() * content_aperture_ratio, theme().size * 2.0f);
    }

    /** The amount of travel that the slider can make.
     */
    [[nodiscard]] float slider_travel_range() const noexcept
    {
        tt_axiom(is_gui_thread());
        return rail_length() - slider_length();
    }

    /** The amount of content hidden from view.
     */
    [[nodiscard]] float hidden_content() const noexcept
    {
        tt_axiom(is_gui_thread());
        return *content - *aperture;
    }

    /** Get the ratio of the hidden content vs the slider travel range.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float hidden_content_vs_travel_ratio() const noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet _slider_travel_range = slider_travel_range();
        return _slider_travel_range != 0.0f ? hidden_content() / _slider_travel_range : 0.0f;
    }

    /** Get the ratio of the slider travel range vs hidden content.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float travel_vs_hidden_content_ratio() const noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet _hidden_content = hidden_content();
        return _hidden_content != 0.0f ? slider_travel_range() / _hidden_content : 0.0f;
    }

    void draw_rails(draw_context const &context) noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet corner_shapes =
            axis == axis::vertical ? tt::corner_shapes{rectangle().width() * 0.5f} : tt::corner_shapes{rectangle().height() * 0.5f};
        context.draw_box(_layout, rectangle(), background_color(), corner_shapes);
    }

    void draw_slider(draw_context const &context) noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet corner_shapes = axis == axis::vertical ? tt::corner_shapes{slider_rectangle.width() * 0.5f} :
                                            tt::corner_shapes{slider_rectangle.height() * 0.5f};

        context.draw_box(_layout, translate_z(0.1f) * slider_rectangle, foreground_color(), corner_shapes);
    }
};

using horizontal_scroll_bar_widget = scroll_bar_widget<axis::horizontal>;
using vertical_scroll_bar_widget = scroll_bar_widget<axis::vertical>;

} // namespace tt
