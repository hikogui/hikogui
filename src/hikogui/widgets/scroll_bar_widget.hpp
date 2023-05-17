// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/scroll_bar_widget.hpp Defines scroll_bar_widget.
 * @ingroup widgets
 */

#pragma once

#include "../GUI/module.hpp"
#include "../geometry/module.hpp"
#include "../observer.hpp"
#include "../utility/module.hpp"
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
template<axis Axis, fixed_string Name = "">
class scroll_bar_widget final : public widget {
public:
    using super = widget;
    constexpr static auto prefix = Name / (Axis == axis::horizontal ? "hbar" : "vbar");

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

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        if (*mode <= widget_mode::collapse) {
            return {};
        }

        // The theme's width and height are the size of the slider.
        // The scroll-bars and therefor the scroll-view and scroll-aperture
        // can never be smaller than the length of the slider of both bars.
        // Unless those bars are disabled by default.
        hi_axiom(theme<prefix>.width(this) >= theme<prefix / "slider">.width(this));
        hi_axiom(theme<prefix>.height(this) >= theme<prefix / "slider">.height(this));
        if constexpr (axis == axis::vertical) {
            return {
                extent2{theme<prefix>.width(this), theme<prefix>.height(this)},
                extent2{theme<prefix>.width(this), theme<prefix>.height(this)},
                extent2{theme<prefix>.width(this), large_number_v<int>}};
        } else {
            return {
                extent2{theme<prefix>.width(this), theme<prefix>.height(this)},
                extent2{theme<prefix>.width(this), theme<prefix>.height(this)},
                extent2{large_number_v<float>, theme<prefix>.height(this)}};
        }
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        layout = context;

        if (*mode <= widget_mode::collapse) {
            _slider_rectangle = {};
            return;
        }

        // Calculate the position of the slider.
        hilet slider_offset = std::round(*offset * travel_vs_hidden_content_ratio());
        if constexpr (axis == axis::vertical) {
            hilet slider_width = theme<prefix / "slider">.width(this);
            hilet x = std::floor((context.width() - slider_width) / 2.0f);

            _slider_rectangle = aarectangle{x, slider_offset, slider_width, slider_length()};

        } else {
            hilet slider_height = theme<prefix / "slider">.height(this);
            hilet y = std::floor((context.height() - slider_height) / 2.0f);

            _slider_rectangle = aarectangle{slider_offset, y, slider_length(), slider_height};
        }
    }

    [[nodiscard]] bool visible() const noexcept
    {
        return *aperture < *content;
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout) and visible()) {
            draw_rails(context);
            draw_slider(context);
        }
    }

    hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout.contains(position) and visible() and _slider_rectangle.contains(position)) {
            return {id, layout.elevation, hitbox_type::scroll_bar};
        } else {
            return {};
        }
    }

    bool handle_event(gui_event const& event) noexcept override
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
                hilet slider_movement =
                    narrow_cast<int>(axis == axis::vertical ? event.drag_delta().y() : event.drag_delta().x());
                hilet content_movement = narrow_cast<int>(std::round(slider_movement * hidden_content_vs_travel_ratio()));
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
        return axis == axis::vertical ? layout.height() : layout.width();
    }

    [[nodiscard]] float slider_length() const noexcept
    {
        hi_axiom(loop::main().on_thread());

        // The minimum length of a slider.
        auto length = axis == axis::vertical ? theme<prefix / "slider">.height(this) : theme<prefix / "slider">.width(this);
        hi_axiom(length <= rail_length());

        // Increase the length of a slider based on the content to aperture ratio.
        if (*content != 0 and *aperture <= *content) {
            inplace_max(length, *aperture * rail_length() / *content);
        }

        return length;
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
        return _slider_travel_range != 0 ? narrow_cast<float>(hidden_content()) / _slider_travel_range : 0.0f;
    }

    /** Get the ratio of the slider travel range vs hidden content.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float travel_vs_hidden_content_ratio() const noexcept
    {
        hi_axiom(loop::main().on_thread());

        hilet _hidden_content = hidden_content();
        return _hidden_content != 0.0f ? narrow_cast<float>(slider_travel_range()) / _hidden_content : 0.0f;
    }

    void draw_rails(widget_draw_context& context) noexcept
    {
        context.draw_box(
            layout,
            layout.rectangle(),
            theme<prefix>.background_color(this),
            theme<prefix>.border_color(this),
            theme<prefix>.border_width(this),
            theme<prefix>.border_radius(this));
    }

    void draw_slider(widget_draw_context& context) noexcept
    {
        context.draw_box(
            layout,
            translate_z(0.1f) * narrow_cast<aarectangle>(_slider_rectangle),
            theme<prefix / "slider">.background_color(this),
            theme<prefix / "slider">.border_color(this),
            theme<prefix / "slider">.border_width(this),
            theme<prefix / "slider">.border_radius(this));
    }
};

}} // namespace hi::v1
