// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace tt {

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
 * @snippet widgets/toggle_example.cpp Create a toggle
 *
 */
class toggle_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    /** Construct a toggle widget.
     *
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this toggle widget.
     * @param delegate The delegate to use to manage the state of the toggle button.
     */
    toggle_widget(gui_window &window, widget *parent, std::unique_ptr<delegate_type> delegate) noexcept;

    /** Construct a toggle widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this toggle widget.
     * @param value The value or `observable` value which represents the state of the toggle.
     * @param args An optional on-value, followed by an optional off-value. These two values
     *             are used to determine which value yields an on/off state.
     */
    template<typename Value, typename... Args>
    toggle_widget(gui_window &window, widget *parent, Value &&value, Args &&...args) noexcept
        requires(not std::is_convertible_v<Value, weak_or_unique_ptr<delegate_type>>) :
        toggle_widget(
            window,
            parent,
            make_unique_default_button_delegate<button_type::toggle>(std::forward<Value>(value), std::forward<Args>(args)...))
    {
    }

    /// @privatesection
    void constrain() noexcept override;
    void set_layout(widget_layout const &context) noexcept override;
    void draw(draw_context const &context) noexcept override;
    /// @endprivatesection
private:
    static constexpr std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    extent2 _button_size;
    aarectangle _button_rectangle;
    animator<float> _animated_value = _animation_duration;
    aarectangle _pip_rectangle;
    float _pip_move_range;

    toggle_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept;
    void draw_toggle_button(draw_context const &context) noexcept;
    void draw_toggle_pip(draw_context const &context) noexcept;
};

} // namespace tt
