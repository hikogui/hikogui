// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toggle_widget.hpp Defines toggle_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "with_label_widget.hpp"
#include "toggle_delegate.hpp"
#include "../telemetry/telemetry.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.widgets.toggle_widget);

hi_export namespace hi {
inline namespace v1 {

template<typename Context>
concept toggle_widget_attribute = forward_of<Context, observer<hi::alignment>> or forward_of<Context, keyboard_focus_group>;

/** A GUI widget that permits the user to make a binary choice.
 *
 * A toggle is very similar to a `toggle_widget`. The
 * semantic difference between a toggle and a toggle is:
 *  - A toggle is immediately active, turning on and off a feature or service at
 *    the moment you toggle it.
 *  - A toggle determines what happens when another action takes place. Or
 *    only becomes active after pressing the "Apply" or "Save" button on a form.
 *    Or becomes part of a record together with other information to be stored
 *    together in a database of some sort.
 *
 * A toggle is a button with three different states with different visual
 * representation:
 *  - **on**: The switch is thrown to the right and is highlighted, and the
 *    `toggle_widget::on_label` is shown.
 *  - **off**: The switch is thrown to the left and is not highlighted, and the
 *    `toggle_widget::off_label` is shown.
 *  - **other**: The switch is thrown to the left and is not highlighted, and
 *    the `toggle_widget::other_label` is shown.
 *
 * @image html toggle_widget.gif
 *
 * Each time a user activates the toggle-button it toggles between the 'on' and
 * 'off' states. If the toggle is in the 'other' state an activation will switch
 * it to the 'off' state.
 *
 * A toggle cannot itself switch state to 'other', this state may be caused by
 * external factors.
 *
 * In the following example we create a toggle widget on the window which
 * observes `value`. When the value is 1 the toggle is 'on', when the value is 2
 * the toggle is 'off'.
 *
 * @snippet widgets/toggle_example_impl.cpp Create a toggle
 *
 * @ingroup widgets
 */
class toggle_widget : public widget {
public:
    using super = widget;
    using delegate_type = toggle_delegate;

    struct attributes_type {
        observer<alignment> alignment = alignment::top_left();
        keyboard_focus_group focus_group = keyboard_focus_group::normal;

        attributes_type(attributes_type const&) noexcept = default;
        attributes_type(attributes_type&&) noexcept = default;
        attributes_type& operator=(attributes_type const&) noexcept = default;
        attributes_type& operator=(attributes_type&&) noexcept = default;

        template<toggle_widget_attribute... Attributes>
        explicit attributes_type(Attributes&&...attributes) noexcept
        {
            set_attributes(std::forward<Attributes>(attributes)...);
        }

        void set_attributes() noexcept {}

        template<toggle_widget_attribute First, toggle_widget_attribute... Rest>
        void set_attributes(First&& first, Rest&&...rest) noexcept
        {
            if constexpr (forward_of<First, observer<hi::alignment>>) {
                alignment = std::forward<First>(first);

            } else if constexpr (forward_of<First, keyboard_focus_group>) {
                focus_group = std::forward<First>(first);

            } else {
                hi_static_no_default();
            }

            set_attributes(std::forward<Rest>(rest)...);
        }
    };

    attributes_type attributes;

    /** The delegate that controls the button widget.
     */
    std::shared_ptr<delegate_type> delegate;

    hi_num_valid_arguments(consteval static, num_default_delegate_arguments, default_toggle_delegate);
    hi_call_left_arguments(static, make_default_delegate, make_shared_ctad<default_toggle_delegate>);
    hi_call_right_arguments(static, make_attributes, attributes_type);

    ~toggle_widget()
    {
        this->delegate->deinit(*this);
    }

    /** Construct a toggle widget.
     *
     * @param parent The parent widget that owns this toggle widget.
     * @param delegate The delegate to use to manage the state of the toggle button.
     */
    toggle_widget(
        attributes_type attributes,
        std::shared_ptr<delegate_type> delegate) noexcept :
        super(), attributes(std::move(attributes)), delegate(std::move(delegate))
    {
        hi_axiom_not_null(this->delegate);
        
        this->delegate->init(*this);
        _delegate_cbt = this->delegate->subscribe([&] {
            set_value(this->delegate->state(*this));
        });
        _delegate_cbt();

        style.set_name("toggle");
    }

    /** Construct a toggle widget with a default button delegate.
     *
     * @param parent The parent widget that owns this toggle widget.
     * @param args The arguments to the `default_toggle_delegate`
     *                followed by arguments to `attributes_type`
     */
    template<typename... Args>
    toggle_widget(Args&&...args)
        requires(num_default_delegate_arguments<Args...>() != 0)
        :
        toggle_widget(
            make_attributes<num_default_delegate_arguments<Args...>()>(std::forward<Args>(args)...),
            make_default_delegate<num_default_delegate_arguments<Args...>()>(std::forward<Args>(args)...))
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _button_size = {theme().size() * 2.0f, theme().size()};
        return box_constraints{_button_size, _button_size, _button_size, *attributes.alignment, theme().margin()};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _button_rectangle = align(context.rectangle(), _button_size, os_settings::alignment(*attributes.alignment));

            auto const button_square =
                aarectangle{get<0>(_button_rectangle), extent2{_button_rectangle.height(), _button_rectangle.height()}};

            _pip_circle = align(button_square, circle{theme().size() * 0.5f - 3.0f}, alignment::middle_center());

            auto const pip_to_button_margin_x2 = _button_rectangle.height() - _pip_circle.diameter();
            _pip_move_range = _button_rectangle.width() - _pip_circle.diameter() - pip_to_button_margin_x2;
        }
        super::set_layout(context);
    }

    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            context.draw_box(
                layout(),
                _button_rectangle,
                background_color(),
                focus_color(),
                theme().border_width(),
                border_side::inside,
                corner_radii{_button_rectangle.height() * 0.5f});

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

            auto const positioned_pip_circle = translate3{_pip_move_range * _animated_value.current_value(), 0.0f, 0.1f} * _pip_circle;

            auto const foreground_color_ = value() == widget_value::on ? accent_color() : foreground_color();
            context.draw_circle(layout(), positioned_pip_circle * 1.02f, foreground_color_);
        }
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        if (phase() == widget_phase::pressed) {
            return theme().fill_color(_layout.layer + 2);
        } else {
            return super::background_color();
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
        return mode() >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::normal);
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
        case gui_event_type::gui_activate:
            if (mode() >= widget_mode::partial) {
                delegate->activate(*this);
                ++global_counter<"toggle_widget:handle_event:relayout">;
                process_event({gui_event_type::window_relayout});
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
                // may match up with the toggle.
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
    animator<float> _animated_value = _animation_duration;
    circle _pip_circle;
    float _pip_move_range;

    callback<void()> _delegate_cbt;

    template<size_t I>
    void set_attributes() noexcept
    {
    }

    template<size_t I, button_widget_attribute First, button_widget_attribute... Rest>
    void set_attributes(First&& first, Rest&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = std::forward<First>(first);
            set_attributes<I>(std::forward<Rest>(rest)...);

        } else {
            hi_static_no_default();
        }
    }
};

using toggle_with_label_widget = with_label_widget<toggle_widget>;

} // namespace v1
} // namespace hi::v1
