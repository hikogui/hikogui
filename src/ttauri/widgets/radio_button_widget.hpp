// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace tt::inline v1 {

/** A graphical control element that allows the user to choose only one of a
 * predefined set of mutually exclusive options.
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
 *       Multiple radio buttons may share a delegate or an observable which
 *       allows radio buttons to act as a set.
 */
class radio_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    /** Construct a radio button widget.
     *
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this radio button widget.
     * @param label The label to show next to the radio button.
     * @param delegate The delegate to use to manage the state of the radio button.
     */
    template<typename Label>
    radio_button_widget(gui_window &window, widget *parent, Label &&label, std::weak_ptr<delegate_type> delegate) noexcept :
        radio_button_widget(window, parent, std::forward<Label>(label), weak_or_unique_ptr{std::move(delegate)})
    {
    }

    /** Construct a radio button widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this radio button widget.
     * @param label The label to show next to the radio button.
     * @param value The value or `observable` value which represents the state
     *              of the radio button.
     * @param args An optional on-value. This value is used to determine which
     *             value yields an 'on' state.
     */
    template<typename Label, typename Value, typename... Args>
    radio_button_widget(gui_window &window, widget *parent, Label &&label, Value &&value, Args &&...args) noexcept
        requires(not std::is_convertible_v<Value, weak_or_unique_ptr<delegate_type>>) :
        radio_button_widget(
            window,
            parent,
            std::forward<Label>(label),
            make_unique_default_button_delegate<button_type::radio>(std::forward<Value>(value), std::forward<Args>(args)...))
    {
    }

    /// @privatesection
    widget_constraints const &set_constraints() noexcept override;
    void set_layout(widget_layout const &layout) noexcept override;
    void draw(draw_context const &context) noexcept override;
    /// @endprivatesection
private:
    static constexpr std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    extent2 _button_size;
    aarectangle _button_rectangle;
    animator<float> _animated_value = _animation_duration;
    aarectangle _pip_rectangle;

    template<typename Label>
    radio_button_widget(gui_window &window, widget *parent, Label &&label, weak_or_unique_ptr<delegate_type> delegate) noexcept :
        super(window, parent, std::move(delegate))
    {
        label_alignment = alignment::middle_left();
        set_label(std::forward<Label>(label));
    }

    void draw_radio_button(draw_context const &context) noexcept;
    void draw_radio_pip(draw_context const &context) noexcept;
};

} // namespace tt::inline v1
