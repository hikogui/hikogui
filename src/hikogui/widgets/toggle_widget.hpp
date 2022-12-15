// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toggle_widget.hpp Defines toggle_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"

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
    [[nodiscard]] box_constraints update_constraints() noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    /// @endprivatesection
private:
    static constexpr std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    box_constraints _label_constraints;

    extent2i _button_size;
    aarectanglei _button_rectangle;
    animator<float> _animated_value = _animation_duration;
    circle _pip_circle;
    int _pip_move_range;

    void draw_toggle_button(draw_context const& context) noexcept;
    void draw_toggle_pip(draw_context const& context) noexcept;
};

}} // namespace hi::v1
