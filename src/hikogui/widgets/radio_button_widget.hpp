// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/radio_button_widget.hpp Defines radio_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "with_label_widget.hpp"
#include "menu_button_widget.hpp"
#include "button_delegate.hpp"
#include "../telemetry/telemetry.hpp"
#include "../macros.hpp"

namespace hi { inline namespace v1 {

template<typename Context>
concept radio_button_widget_attribute =
    forward_of<Context, observer<hi::alignment>> or forward_of<Context, keyboard_focus_group>;

/** A GUI widget that permits the user to make a binary choice.
 * @ingroup widgets
 *
 * A radio_button is a button with three different states with different visual
 * representation:
 *  - **on**: A check-mark is shown inside the box, and the `radio_button_widget::on_label` is shown.
 *  - **off**: An empty box is shown, and the `radio_button_widget::off_label` is shown.
 *  - **other**: A dash is shown inside the box, and the `radio_button_widget::other_label` is shown.
 *
 * @image html radio_button_widget.gif
 *
 * Each time a user activates the radio_button-button it toggles between the 'on' and 'off' states.
 * If the radio_button is in the 'other' state an activation will switch it to
 * the 'off' state.
 *
 * A radio_button cannot itself switch state to 'other', this state may be
 * caused by external factors. The canonical example is a tree structure
 * of radio_buttones; when child radio_buttones have different values from each other
 * the parent radio_button state is set to 'other'.
 *
 * In the following example we create a radio_button widget on the window
 * which observes `value`. When the value is 1 the radio_button is 'on',
 * when the value is 2 the radio_button is 'off'.
 *
 * @snippet widgets/radio_button_example_impl.cpp Create a radio_button
 */
class radio_button_widget final : public widget {
public:
    using super = widget;
    using delegate_type = button_delegate;
    template<typename T>
    using default_delegate_type = default_radio_button_delegate<T>;

    /** The delegate that controls the button widget.
     */
    not_null<std::shared_ptr<delegate_type>> delegate;

    /** The alignment of the button and on/off/other label.
     */
    observer<alignment> alignment;

    /** Notifier to await or callback on when the button was activated.
     */
    notifier<> activated;

    ~radio_button_widget()
    {
        this->delegate->deinit(*this);
    }

    /** Construct a radio_button widget.
     *
     * @param parent The parent widget that owns this radio_button widget.
     * @param delegate The delegate to use to manage the state of the radio_button button.
     */
    template<radio_button_widget_attribute... Attributes>
    radio_button_widget(
        not_null<widget *> parent,
        not_null<std::shared_ptr<delegate_type>> delegate,
        Attributes &&...attributes) noexcept :
        super(parent), delegate(std::move(delegate))
    {
        alignment = alignment::top_left();
        set_attributes<0>(std::forward<Attributes>(attributes)...);

        _delegate_cbt = this->delegate->subscribe([&]{
            this->request_redraw();
            activated();
        });

        this->delegate->init(*this);
    }

    template<typename... Args>
    [[nodiscard]] static std::shared_ptr<delegate_type> make_default_delegate(Args &&...args)
        requires requires { default_radio_button_delegate{std::forward<Args>(args)...}; }
    {
        return make_shared_ctad<default_radio_button_delegate>(std::forward<Args>(args)...);
    }

    /** Construct a radio_button widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this radio_button widget.
     * @param value The value or `observer` value which represents the state of the radio_button.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        radio_button_widget_attribute... Attributes>
    radio_button_widget(
        not_null<widget *> parent,
        Value&& value,
        OnValue&& on_value,
        Attributes &&...attributes) noexcept
        requires requires
    {
        make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value));
    } :
        radio_button_widget(
            parent,
            make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value)),
            std::forward<Attributes>(attributes)...)
    {
    }

    /** Get the current state of the button.
     * @return The state of the button: on / off / other.
     */
    [[nodiscard]] button_state state() const noexcept
    {
        hi_axiom(loop::main().on_thread());
        return delegate->state(*this);
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _button_size = {theme().size(), theme().size()};
        return box_constraints{_button_size, _button_size, _button_size, *alignment, theme().margin(), {}};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _button_rectangle = align(context.rectangle(), _button_size, os_settings::alignment(*alignment));

            _button_circle = circle{_button_rectangle};

            _pip_circle = align(_button_rectangle, circle{theme().size() * 0.5f - 3.0f}, alignment::middle_center());
        }
        super::set_layout(context);
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout())) {
            if (_focus_group != keyboard_focus_group::menu) {
                context.draw_circle(
                    layout(), _button_circle * 1.02f, background_color(), focus_color(), theme().border_width(), border_side::inside);
            }

            _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, context.display_time_point);
            if (_animated_value.is_animating()) {
                request_redraw();
            }

            // draw pip
            auto float_value = _animated_value.current_value();
            if (float_value > 0.0) {
                context.draw_circle(layout(), _pip_circle * 1.02f * float_value, accent_color());
            }
        }
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        if (_pressed) {
            return theme().color(semantic_color::fill, semantic_layer + 2);
        } else {
            return super::background_color();
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept final
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout().contains(position)) {
            return {id, _layout.elevation, hitbox_type::button};
        } else {
            return {};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        return *mode >= widget_mode::partial and to_bool(group & _focus_group);
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
        case gui_event_type::gui_activate:
            if (*mode >= widget_mode::partial) {
                delegate->activate(*this);
                ++global_counter<"radio_button_widget:handle_event:relayout">;
                process_event({gui_event_type::window_relayout});
                return true;
            }
            break;

        case gui_event_type::mouse_down:
            if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
                _pressed = true;
                request_redraw();
                return true;
            }
            break;

        case gui_event_type::mouse_up:
            if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
                _pressed = false;

                // with_label_widget or other widgets may have accepted the hitbox
                // for this widget. Which means the widget_id in the mouse-event
                // may match up with the radio_button.
                if (event.mouse().hitbox.widget_id == id) {
                    handle_event(gui_event_type::gui_activate);
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

    extent2 _button_size;
    aarectangle _button_rectangle;

    circle _button_circle;

    animator<float> _animated_value = _animation_duration;
    circle _pip_circle;

    keyboard_focus_group _focus_group = keyboard_focus_group::normal;

    bool _pressed = false;

    callback<void()> _delegate_cbt;

    template<size_t I>
    void set_attributes() noexcept
    {
    }

    template<size_t I, radio_button_widget_attribute First, radio_button_widget_attribute... Rest>
    void set_attributes(First&& first, Rest&&...rest) noexcept
    {
        if constexpr (forward_of<First, observer<hi::alignment>>) {
            alignment = std::forward<First>(first);

        } else if constexpr (forward_of<First, keyboard_focus_group>) {
            _focus_group = std::forward<First>(first);

        } else {
            hi_static_no_default();
        }

        set_attributes<I>(std::forward<Rest>(rest)...);
    }
};

using radio_button_with_label_widget = with_label_widget<radio_button_widget>;
using radio_menu_button_widget = menu_button_widget<radio_button_widget>;

}} // namespace hi::v1
