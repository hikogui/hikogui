// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/radio_button_widget.hpp Defines radio_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"

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
class radio_button_widget final : public abstract_button_widget<Name / "radio-button"> {
public:
    using super = abstract_button_widget<Name / "radio-button">;
    using delegate_type = typename super::delegate_type;
    constexpr static auto prefix = super::prefix;

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
        this->alignment = alignment::top_left();
        this->set_attributes<0>(hi_forward(attributes)...);
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
        _label_constraints = super::update_constraints();

        // Make room for button and margin.
        _button_size = theme<prefix>.size(this);
        hi_axiom(_button_size.width() == _button_size.height());

        hilet extra_size = extent2{theme<prefix>.spacing_horizontal(this) + _button_size.width(), 0};

        auto constraints = max(_label_constraints + extra_size, _button_size);
        constraints.margins = theme<prefix>.margin(this);
        constraints.alignment = *this->alignment;
        return constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(this->layout, context)) {
            auto alignment_ = os_settings::left_to_right() ? *this->alignment : mirror(*this->alignment);

            if (alignment_ == horizontal_alignment::left or alignment_ == horizontal_alignment::right) {
                _button_rectangle = align(context.rectangle(), _button_size, alignment_);
            } else {
                hi_not_implemented();
            }

            hilet inner_margin = theme<prefix>.spacing_horizontal(this);
            hilet cap_height = theme<prefix>.cap_height(this);

            hilet label_width = context.width() - (_button_rectangle.width() + inner_margin);
            if (alignment_ == horizontal_alignment::left) {
                hilet label_left = _button_rectangle.right() + inner_margin;
                hilet label_rectangle = aarectangle{label_left, 0, label_width, context.height()};
                this->_on_label_shape = this->_off_label_shape = this->_other_label_shape =
                    box_shape{_label_constraints, label_rectangle, cap_height};

            } else if (alignment_ == horizontal_alignment::right) {
                hilet label_rectangle = aarectangle{0, 0, label_width, context.height()};
                this->_on_label_shape = this->_off_label_shape = this->_other_label_shape =
                    box_shape{_label_constraints, label_rectangle, cap_height};

            } else {
                hi_not_implemented();
            }

            _button_circle = circle{narrow_cast<aarectangle>(_button_rectangle)};

            hilet pip_size = theme<prefix / "pip">.size(this);
            hi_axiom(pip_size.width() == pip_size.height());

            _pip_circle =
                align(narrow_cast<aarectangle>(_button_rectangle), circle{narrow_cast<float>(pip_size.width() / 2)}, alignment::middle_center());
        }
        super::set_layout(context);
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*this->mode > widget_mode::invisible and overlaps(context, this->layout)) {
            draw_radio_button(context);
            draw_radio_pip(context);
            this->draw_button(context);
        }
    }
    /// @endprivatesection
private:
    static constexpr std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    box_constraints _label_constraints;

    extent2 _button_size;
    aarectangle _button_rectangle;
    circle _button_circle;

    animator<float> _animated_value = _animation_duration;
    circle _pip_circle;

    void draw_radio_button(widget_draw_context& context) noexcept
    {
        context.draw_circle(
            this->layout,
            _button_circle * 1.02f,
            theme<prefix>.background_color(this),
            theme<prefix>.border_color(this),
            theme<prefix>.border_width(this),
            border_side::inside);
    }

    void draw_radio_pip(widget_draw_context& context) noexcept
    {
        _animated_value.update(this->state != widget_state::off ? 1.0f : 0.0f, context.display_time_point);
        if (_animated_value.is_animating()) {
            this->request_redraw();
        }

        // draw pip
        auto float_value = _animated_value.current_value();
        if (float_value > 0.0f) {
            context.draw_circle(
                this->layout,
                _pip_circle * 1.02f * float_value,
                theme<prefix / "pip">.background_color(this),
                theme<prefix / "pip">.border_color(this),
                theme<prefix / "pip">.border_width(this));
        }
    }
};

}} // namespace hi::v1
