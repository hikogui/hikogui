// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/radio_widget.hpp Defines radio_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "with_label_widget.hpp"
#include "menu_button_widget.hpp"
#include "radio_delegate.hpp"
#include "../telemetry/telemetry.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.widgets.radio_widget);

hi_export namespace hi {
inline namespace v1 {

/** A GUI widget that permits the user to make a binary choice.
 *
 * A radio is a button with three different states with different visual
 * representation:
 *  - **on**: A pip is shown inside the circle.
 *  - **off**: An empty circle is shown.
 *
 * @image html radio_widget.gif
 *
 * Each time a user activates the radio-button it toggles between the 'on' and 'off' states.
 * If the radio is in the 'other' state an activation will switch it to
 * the 'off' state.
 *
 * In the following example we create a radio widget on the window
 * which observes `value`. When the value is 1 the radio is 'on',
 * when the value is 2 the radio is 'off'.
 *
 * @snippet widgets/radio_example_impl.cpp Create a radio
 */
class radio_widget : public widget {
public:
    using super = widget;
    using delegate_type = radio_delegate;

    /** The delegate that controls the button widget.
     */
    std::shared_ptr<delegate_type> delegate;

    keyboard_focus_group focus_group = keyboard_focus_group::normal;

    template<typename... Args>
    [[nodiscard]] static std::shared_ptr<delegate_type> make_default_delegate(Args&&... args) noexcept
    {
        return make_shared_ctad<default_radio_delegate>(std::forward<Args>(args)...);
    }

    ~radio_widget()
    {
        this->delegate->deinit(*this);
    }

    /** Construct a radio widget.
     *
     * @param parent The parent widget that owns this radio widget.
     * @param delegate The delegate to use to manage the state of the radio button.
     */
    template<std::derived_from<delegate_type> Delegate>
    radio_widget(std::shared_ptr<Delegate> delegate) noexcept : super(), delegate(std::move(delegate))
    {
        hi_axiom_not_null(this->delegate);

        this->delegate->init(*this);
        _delegate_cbt = this->delegate->subscribe([&] {
            set_value(this->delegate->state(*this));
        });
        _delegate_cbt();

        style.set_name("radio");
    }

    /** Construct a radio widget with a default button delegate.
     *
     * @param parent The parent widget that owns this toggle widget.
     * @param args The arguments to the `default_radio_delegate`
     *                followed by arguments to `attributes_type`
     */
    template<typename... Args>
    radio_widget(Args&&... args) : radio_widget(make_default_delegate(std::forward<Args>(args)...))
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        return box_constraints{style.size_px, style.size_px, style.size_px, style.alignment, style.margins_px};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _button_rectangle = align(context.rectangle(), style.size_px, os_settings::alignment(style.alignment));

            _button_circle = circle{_button_rectangle};

            _pip_circle = align(_button_rectangle, circle{style.height_px * 0.5f - 3.0f}, alignment::middle_center());
        }
        super::set_layout(context);
    }

    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            if (focus_group != keyboard_focus_group::menu) {
                context.draw_circle(
                    layout(),
                    _button_circle * 1.02f,
                    style.background_color,
                    style.border_color,
                    style.border_width_px,
                    border_side::inside);
            }

            switch (_animated_value.update(value() == widget_value::on ? 1.0f : 0.0f, context.display_time_point)) {
            case animator_state::idle:
                break;
            case animator_state::running:
                request_redraw();
                break;
            case animator_state::end:
                notifier();
                break;
            default:
                hi_no_default();
            }

            // draw pip
            auto float_value = _animated_value.current_value();
            if (float_value > 0.0) {
                context.draw_circle(layout(), _pip_circle * 1.02f * float_value, style.accent_color);
            }
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (mode() >= widget_mode::partial and layout().contains(position)) {
            return {id, _layout.elevation, hitbox_type::button};
        } else {
            return {};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        return mode() >= widget_mode::partial and to_bool(group & this->focus_group);
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
        case gui_event_type::gui_activate:
            if (mode() >= widget_mode::partial) {
                delegate->activate(*this);
                request_redraw();
                return true;
            }
            break;

        case gui_event_type::mouse_down:
            if (mode() >= widget_mode::partial and event.mouse().cause.left_button) {
                set_pressed(true);
                return true;
            }
            break;

        case gui_event_type::mouse_up:
            if (mode() >= widget_mode::partial and event.mouse().cause.left_button) {
                set_pressed(false);

                // with_label_widget or other widgets may have accepted the hitbox
                // for this widget. Which means the widget_id in the mouse-event
                // may match up with the radio.
                if (event.mouse().hitbox.widget_id == id) {
                    // By staying we can give focus to the parent widget.
                    handle_event(gui_event_type::gui_activate_stay);
                }
                request_redraw();
                return true;
            }
            break;

        default:;
        }

        return super::handle_event(event);
    }
    /// @endprivatesection

private:
    constexpr static std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    aarectangle _button_rectangle;

    circle _button_circle;

    animator<float> _animated_value = _animation_duration;
    circle _pip_circle;

    callback<void()> _delegate_cbt;
};

using radio_with_label_widget = with_label_widget<radio_widget>;
using radio_menu_button_widget = menu_button_widget<radio_widget>;

} // namespace v1
} // namespace hi::v1
