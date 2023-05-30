// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/radio_button_widget.hpp Defines radio_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "radio_button_delegate.hpp"

namespace hi { inline namespace v1 {

/** A graphical control element that allows the user to choose only one of a
 * predefined set of mutually exclusive options.
 * @ingroup widgets
 *
 * A radio-button has two different states with different visual representation:
 *  - **on**: The radio button shows a solid circle inside it
 *  - **other**: The radio button shows empty.
 *
 * @image html radio_button_widget.gif
 *
 * Each time a user activates the radio-button it switches its state to 'on'.
 *
 * A radio button cannot itself switch state to 'other', this state may be
 * caused by external factors. The canonical example is another radio button in
 * a set, which is configured with a different `on_value`.
 *
 * In the following example we create three radio button widgets on the window
 * which observes the same `value`. Each radio button is configured with a
 * different `on_value`: 1, 2 and 3. Initially the value is 0, and therefor none
 * of the radio buttons is selected when the application is started.
 *
 * @snippet widgets/radio_button_example_impl.cpp Create three radio buttons
 *
 * @note Unlike some other GUI toolkits a radio button is a singular widget.
 *       Multiple radio buttons may share a delegate or an observer which
 *       allows radio buttons to act as a set.
 */
template<fixed_string Name = "">
class radio_button_widget final : public widget {
public:
    using super = widget;
    using delegate_type = radio_button_delegate;
    constexpr static auto prefix = Name / "radio";

    /** The delegate that controls the button widget.
     */
    std::shared_ptr<delegate_type> delegate;

    /** The label to show when the button is in the 'on' state.
     */
    observer<label> label = tr("<not set>");

    /** The alignment of the button and on/off/other label.
     */
    observer<hi::alignment> alignment = alignment::top_left();

    /** Construct a radio button widget.
     *
     * @param parent The parent widget that owns this radio button widget.
     * @param delegate The delegate to use to manage the state of the radio button.
     * @param attributes Different attributes used to configure the label's on the radio button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    radio_button_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        hi_assert_not_null(this->delegate);
        this->set_attributes<0>(hi_forward(attributes)...);

        _label_widget = std::make_unique<label_widget<prefix>>(this, on_label, alignment);

        _grid.add_cell(0, 0, cell_type::button);
        _grid.add_cell(1, 0, cell_type::label);

        _delegate_cbt = this->delegate->subscribe([&] {
            ++global_counter<"radio_button_widget:delegate:redraw">;
            hi_assert_not_null(this->delegate);
            state = this->delegate->state(this);

            process_event({gui_event_type::window_redraw});
        });
        this->delegate->init(*this);
        (*_delegate_cbt)();
    }

    /** Construct a radio button widget with a default button delegate.
     *
     * @param parent The parent widget that owns this radio button widget.
     * @param value The value or `observer` value which represents the state
     *              of the radio button.
     * @param on_value An optional on-value. This value is used to determine which
     *             value yields an 'on' state.
     * @param attributes Different attributes used to configure the label's on the radio button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        button_widget_attribute... Attributes>
    radio_button_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires { make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)); }
        :
        radio_button_widget(
            parent,
            make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        for (auto& cell : _grid) {
            if (cell.value == cell_type::button) {
                cell.set_constraints(box_constraints{
                    theme<prefix>.size(this),
                    theme<prefix>.size(this),
                    theme<prefix>.size(this),
                    *alignment,
                    theme<prefix>.margin(this),
                    -vector2::infinity()});

            } else if (cell.value == cell_type::label) {
                cell.set_constraints(max(_label_widget->update_constraints()));

            } else {
                hi_no_default();
            }
        }

        return _grid.constraints(os_settings::left_to_right());
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(layout, context)) {
            _grid.set_layout(context.shape, theme<prefix>.cap_height(this));
        }

        for (hilet& cell : _grid) {
            if (cell.value == cell_type::button) {
                _button_rectangle = align(cell.shape.rectangle, theme<prefix>.size(this), *alignment);
                _pip_rectangle = align(_button_rectangle, theme<prefix / "pip">.size(this), alignment::middle_center());

            } else if (cell.value == cell_type::label) {
                _label_widget->set_layout(context.transform(cell.shape, 0.0f));

            } else {
                hi_no_default();
            }
        }
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout)) {
            for (hilet& cell : _grid) {
                if (cell.value == cell_type::button) {
                    draw_button(context);
                    draw_pip(context);

                } else if (cell.value == cell_type::label) {
                    _label_widget->draw(context);

                } else {
                    hi_no_default();
                }
            }
        }
    }

    [[nodiscard]] generator<widget const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_label_widget;
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept final
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout.contains(position)) {
            return {id, layout.elevation, hitbox_type::button};
        } else {
            return {};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::normal);
    }

    void activate() noexcept
    {
        hi_assert_not_null(delegate);
        delegate->activate(*this);
        this->_state_changed();
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
        case gui_event_type::gui_activate:
            if (*mode >= widget_mode::partial) {
                activate();
                return true;
            }
            break;

        case gui_event_type::mouse_down:
            if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
                clicked = true;
                request_redraw();
                return true;
            }
            break;

        case gui_event_type::mouse_up:
            if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
                clicked = false;

                if (layout.rectangle().contains(event.mouse().position)) {
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
    enum class cell_type { button, label };

    static constexpr std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    grid_layout<cell_type> _grid;
    std::unique_ptr<label_widget<prefix>> _label_widget;

    notifier<>::callback_token _delegate_cbt;

    aarectangle _button_rectangle;
    aarectangle _pip_rectangle;

    animator<float> _animated_value = _animation_duration;

    void draw_button(widget_draw_context& context) noexcept
    {
        context.draw_box(
            this->layout,
            _button_rectangle,
            theme<prefix>.background_color(this),
            theme<prefix>.border_color(this),
            theme<prefix>.border_width(this),
            border_side::outside,
            theme<prefix>.border_radius(this));
    }

    void draw_pip(widget_draw_context& context) noexcept
    {
        _animated_value.update(this->state != widget_state::off ? 1.0f : 0.0f, context.display_time_point);
        if (_animated_value.is_animating()) {
            this->request_redraw();
        }

        // draw pip
        auto float_value = _animated_value.current_value();
        if (float_value > 0.0f) {
            context.draw_box(
                this->layout,
                _pip_rectangle * float_value,
                theme<prefix / "pip">.background_color(this),
                theme<prefix / "pip">.border_color(this),
                theme<prefix / "pip">.border_width(this),
                border_side::inside,
                theme<prefix / "pip">.border_radius(this) * float_value);
        }
    }
};

}} // namespace hi::v1
