// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/scroll_bar_widget.hpp Defines scroll_bar_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../GUI/gui_event.hpp"
#include "../geometry/axis.hpp"
#include "../observer.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

/** Scroll bar widget
 * This widget is used in a pair of a vertical and horizontal scrollbar as
 * a child of the `scroll_widget`. The vertical and horizontal scrollbar are
 * displayed next to the `scroll_aperture_widget` and controls what part of
 * the content is displayed in the aperture.
 *
 * @ingroup widgets
 * @tparam Axis which axis (horizontal or vertical) this scroll bar is used for.
 */
template<axis Axis>
class scroll_bar_widget final : public widget {
public:
    using super = widget;

    static constexpr hi::axis axis = Axis;

    observer<float> offset;
    observer<float> aperture;
    observer<float> content;

    scroll_bar_widget(
        widget *parent,
        forward_of<observer<float>> auto&& content,
        forward_of<observer<float>> auto&& aperture,
        forward_of<observer<float>> auto&& offset) noexcept :
        widget(parent), content(hi_forward(content)), aperture(hi_forward(aperture)), offset(hi_forward(offset))
    {
        _content_cbt = this->content.subscribe([&](auto...) {
            ++global_counter<"scroll_bar_widget:content:relayout">;
            process_event({gui_event_type::window_relayout});
        });
        _aperture_cbt = this->aperture.subscribe([&](auto...) {
            ++global_counter<"scroll_bar_widget:aperture:relayout">;
            process_event({gui_event_type::window_relayout});
        });
        _offset_cbt = this->offset.subscribe([&](auto...) {
            ++global_counter<"scroll_bar_widget:offset:relayout">;
            process_event({gui_event_type::window_relayout});
        });
    }

    ~scroll_bar_widget() {}

    widget_constraints const& set_constraints(set_constraints_context const& context) noexcept override
    {
        _layout = {};

        // The minimum size is twice the length of the slider, which is twice the context.theme->size()
        if constexpr (axis == axis::vertical) {
            return _constraints = {
                       {context.theme->icon_size, context.theme->size * 4.0f},
                       {context.theme->icon_size, context.theme->size * 4.0f},
                       {context.theme->icon_size, 32767.0f}};
        } else {
            return _constraints = {
                       {context.theme->size * 4.0f, context.theme->icon_size},
                       {context.theme->size * 4.0f, context.theme->icon_size},
                       {32767.0f, context.theme->icon_size}};
        }
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        _layout = context;

        // Calculate the position of the slider.
        hilet slider_offset = *offset * travel_vs_hidden_content_ratio();
        if constexpr (axis == axis::vertical) {
            _slider_rectangle = aarectangle{0.0f, slider_offset, context.width(), slider_length()};
        } else {
            _slider_rectangle = aarectangle{slider_offset, 0.0f, slider_length(), context.height()};
        }
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout())) {
            draw_rails(context);
            draw_slider(context);
        }
    }

    hitbox hitbox_test(point3 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout().contains(position) and _slider_rectangle.contains(position)) {
            return {this, position, hitbox_type::scroll_bar};
        } else {
            return {};
        }
    }

    bool handle_event(gui_event const& event) noexcept
    {
        switch (event.type()) {
        case gui_event_type::mouse_down:
            if (event.mouse().cause.left_button) {
                // Record the original scroll-position before the drag starts.
                _offset_before_drag = *offset;
                return true;
            }
            break;

        case gui_event_type::mouse_drag:
            if (event.mouse().cause.left_button) {
                // The distance the slider has to move relative to the slider position at the
                // start of the drag.
                hilet slider_movement = axis == axis::vertical ? event.drag_delta().y() : event.drag_delta().x();
                hilet content_movement = slider_movement * hidden_content_vs_travel_ratio();
                hilet new_offset = _offset_before_drag + content_movement;
                offset = clamp_offset(new_offset);
                return true;
            }
            break;

        default:;
        }

        return super::handle_event(event);
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return false;
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        return _layout.theme->color(semantic_color::fill, semantic_layer);
    }

    [[nodiscard]] color foreground_color() const noexcept override
    {
        if (*hover) {
            return _layout.theme->color(semantic_color::fill, semantic_layer + 2);
        } else {
            return _layout.theme->color(semantic_color::fill, semantic_layer + 1);
        }
    }

private:
    aarectangle _slider_rectangle;

    float _offset_before_drag;

    typename decltype(content)::callback_token _content_cbt;
    typename decltype(aperture)::callback_token _aperture_cbt;
    typename decltype(offset)::callback_token _offset_cbt;

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
        hi_axiom(loop::main().on_thread());
        return axis == axis::vertical ? layout().height() : layout().width();
    }

    [[nodiscard]] float slider_length() const noexcept
    {
        hi_axiom(loop::main().on_thread());

        hilet content_aperture_ratio = *content != 0.0f ? *aperture / *content : 1.0f;
        hilet rail_length_ = rail_length();
        return std::clamp(rail_length_ * content_aperture_ratio, _layout.theme->size * 2.0f, rail_length_);
    }

    /** The amount of travel that the slider can make.
     */
    [[nodiscard]] float slider_travel_range() const noexcept
    {
        hi_axiom(loop::main().on_thread());
        return rail_length() - slider_length();
    }

    /** The amount of content hidden from view.
     */
    [[nodiscard]] float hidden_content() const noexcept
    {
        hi_axiom(loop::main().on_thread());
        return *content - *aperture;
    }

    /** Get the ratio of the hidden content vs the slider travel range.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float hidden_content_vs_travel_ratio() const noexcept
    {
        hi_axiom(loop::main().on_thread());

        hilet _slider_travel_range = slider_travel_range();
        return _slider_travel_range != 0.0f ? hidden_content() / _slider_travel_range : 0.0f;
    }

    /** Get the ratio of the slider travel range vs hidden content.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float travel_vs_hidden_content_ratio() const noexcept
    {
        hi_axiom(loop::main().on_thread());

        hilet _hidden_content = hidden_content();
        return _hidden_content != 0.0f ? slider_travel_range() / _hidden_content : 0.0f;
    }

    void draw_rails(draw_context const& context) noexcept
    {
        hilet corner_radii =
            axis == axis::vertical ? hi::corner_radii{layout().width() * 0.5f} : hi::corner_radii{layout().height() * 0.5f};
        context.draw_box(layout(), layout().rectangle(), background_color(), corner_radii);
    }

    void draw_slider(draw_context const& context) noexcept
    {
        hilet corner_radii = axis == axis::vertical ? hi::corner_radii{_slider_rectangle.width() * 0.5f} :
                                                      hi::corner_radii{_slider_rectangle.height() * 0.5f};

        context.draw_box(layout(), translate_z(0.1f) * _slider_rectangle, foreground_color(), corner_radii);
    }
};

using horizontal_scroll_bar_widget = scroll_bar_widget<axis::horizontal>;
using vertical_scroll_bar_widget = scroll_bar_widget<axis::vertical>;

}} // namespace hi::v1
