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
 * @snippet widgets/radio_button_example.cpp Create three radio buttons
 *
 * @note Unlike some other GUI toolkits a radio button is a singular widget.
 *       Multiple radio buttons may share a delegate or an observer which
 *       allows radio buttons to act as a set.
 */
class radio_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    /** Construct a radio button widget.
     *
     * @param parent The parent widget that owns this radio button widget.
     * @param delegate The delegate to use to manage the state of the radio button.
     * @param attributes Different attributes used to configure the label's on the radio button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    radio_button_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        alignment = alignment::top_left();
        set_attributes<0>(hi_forward(attributes)...);
    }

    /** Construct a radio button widget with a default button delegate.
     *
     * @param parent The parent widget that owns this radio button widget.
     * @param value The value or `observer` value which represents the state
     *              of the radio button.
     * @param on_value An optional on-value. This value is used to determine which
     *             value yields an 'on' state.
     * @param attributes Different attributes used to configure the label's on the radio button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        button_widget_attribute... Attributes>
    radio_button_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires
    {
        make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value));
    } :
        radio_button_widget(
            parent,
            make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    widget_constraints const& set_constraints(set_constraints_context const& context) noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    /// @endprivatesection
private:
    static constexpr std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    extent2 _button_size;
    aarectangle _button_rectangle;
    animator<float> _animated_value = _animation_duration;
    aarectangle _pip_rectangle;

    void draw_radio_button(draw_context const& context) noexcept;
    void draw_radio_pip(draw_context const& context) noexcept;
};

}} // namespace hi::v1
