// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toggle_widget.hpp Defines toggle_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"
#include "../macros.hpp"

namespace hi { inline namespace v1 {

/** A GUI widget that permits the user to make a binary choice.
 *
 * A toggle is very similar to a `checkbox_widget`. The
 * semantic difference between a checkbox and a toggle is:
 *  - A toggle is immediately active, turning on and off a feature or service at
 *    the moment you toggle it.
 *  - A checkbox determines what happens when another action takes place. Or
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
class toggle_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    /** Construct a toggle widget.
     *
     * @param parent The parent widget that owns this toggle widget.
     * @param delegate The delegate to use to manage the state of the toggle button.
     * @param attributes Different attributes used to configure the label's on the toggle button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    toggle_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        alignment = alignment::top_left();
        set_attributes<0>(hi_forward(attributes)...);
    }

    /** Construct a toggle widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this toggle widget.
     * @param value The value or `observer` value which represents the state of the toggle.
     * @param attributes Different attributes used to configure the label's on the toggle button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    template<different_from<std::shared_ptr<delegate_type>> Value, button_widget_attribute... Attributes>
    toggle_widget(widget *parent, Value&& value, Attributes&&...attributes) noexcept requires requires
    {
        make_default_toggle_button_delegate(hi_forward(value));
    } : toggle_widget(parent, make_default_toggle_button_delegate(hi_forward(value)), hi_forward(attributes)...) {}

    /** Construct a toggle widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this toggle widget.
     * @param value The value or `observer` value which represents the state of the toggle.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     * @param attributes Different attributes used to configure the label's on the toggle button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        button_widget_attribute... Attributes>
    toggle_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires
    {
        make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value));
    } :
        toggle_widget(
            parent,
            make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value)),
            hi_forward(attributes)...)
    {
    }

    /** Construct a toggle widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this toggle widget.
     * @param value The value or `observer` value which represents the state of the toggle.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     * @param off_value The off-value. This value is used to determine which value yields an 'off' state.
     * @param attributes Different attributes used to configure the label's on the toggle button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        forward_of<observer<observer_decay_t<Value>>> OffValue,
        button_widget_attribute... Attributes>
    toggle_widget(
        widget *parent,
        Value&& value,
        OnValue&& on_value,
        OffValue&& off_value,
        Attributes&&...attributes) noexcept requires requires
    {
        make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value), hi_forward(off_value));
    } :
        toggle_widget(
            parent,
            make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value), hi_forward(off_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = super::update_constraints();

        // Make room for button and margin.
        _button_size = {theme().size() * 2.0f, theme().size()};
        hilet extra_size = extent2{theme().margin<float>() + _button_size.width(), 0.0f};

        auto r = max(_label_constraints + extra_size, _button_size);
        r.margins = theme().margin();
        r.alignment = *alignment;
        return r;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            auto alignment_ = os_settings::left_to_right() ? *alignment : mirror(*alignment);

            if (alignment_ == horizontal_alignment::left or alignment_ == horizontal_alignment::right) {
                _button_rectangle = align(context.rectangle(), _button_size, alignment_);
            } else {
                hi_not_implemented();
            }

            hilet label_width = context.width() - (_button_rectangle.width() + theme().margin<float>());
            if (alignment_ == horizontal_alignment::left) {
                hilet label_left = _button_rectangle.right() + theme().margin<float>();
                hilet label_rectangle = aarectangle{label_left, 0.0f, label_width, context.height()};
                _on_label_shape = _off_label_shape = _other_label_shape =
                    box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};

            } else if (alignment_ == horizontal_alignment::right) {
                hilet label_rectangle = aarectangle{0, 0, label_width, context.height()};
                _on_label_shape = _off_label_shape = _other_label_shape =
                    box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};

            } else {
                hi_not_implemented();
            }

            hilet button_square =
                aarectangle{get<0>(_button_rectangle), extent2{_button_rectangle.height(), _button_rectangle.height()}};

            _pip_circle = align(button_square, circle{theme().size() * 0.5f - 3.0f}, alignment::middle_center());

            hilet pip_to_button_margin_x2 = _button_rectangle.height() - _pip_circle.diameter();
            _pip_move_range = _button_rectangle.width() - _pip_circle.diameter() - pip_to_button_margin_x2;
        }
        super::set_layout(context);
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout())) {
            draw_toggle_button(context);
            draw_toggle_pip(context);
            draw_button(context);
        }
    }

    /// @endprivatesection
private:
    constexpr static std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    box_constraints _label_constraints;

    extent2 _button_size;
    aarectangle _button_rectangle;
    animator<float> _animated_value = _animation_duration;
    circle _pip_circle;
    float _pip_move_range;

    void draw_toggle_button(draw_context const& context) noexcept
    {
        context.draw_box(
            layout(),
            _button_rectangle,
            background_color(),
            focus_color(),
            theme().border_width(),
            border_side::inside,
            corner_radii{_button_rectangle.height() * 0.5f});
    }

    void draw_toggle_pip(draw_context const& context) noexcept
    {
        _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, context.display_time_point);
        if (_animated_value.is_animating()) {
            request_redraw();
        }

        hilet positioned_pip_circle = translate3{_pip_move_range * _animated_value.current_value(), 0.0f, 0.1f} * _pip_circle;

        hilet forground_color_ = state() == button_state::on ? accent_color() : foreground_color();
        context.draw_circle(layout(), positioned_pip_circle * 1.02f, forground_color_);
    }
};

}} // namespace hi::v1
