// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace tt {

/** A GUI widget that permits the user to make a binary choice.
 *
 * A checkbox is a button with three different states with different visual
 * representation:
 *  - **on**: A check-mark is shown inside the box, and the `checkbox_widget::on_label` is shown.
 *  - **off**: An empty box is shown, and the `checkbox_widget::off_label` is shown.
 *  - **other**: A dash is shown inside the box, and the `checkbox_widget::other_label` is shown.
 *
 * @image html checkbox_widget.gif
 *
 * Each time a user activates the checkbox-button it toggles between the 'on' and 'off' states.
 * If the checkbox is in the 'other' state an activation will switch it to
 * the 'off' state.
 *
 * A checkbox cannot itself switch state to 'other', this state may be
 * caused by external factors. The canonical example is a tree structure
 * of checkboxes; when child checkboxes have different values from each other
 * the parent checkbox state is set to 'other'.
 *
 * In the following example we create a checkbox widget on the window
 * which observes `value`. When the value is 1 the checkbox is 'on',
 * when the value is 2 the checkbox is 'off'.
 *
 * @snippet widgets/checkbox_example.cpp Create a checkbox
 *
 */
class checkbox_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    /** Construct a checkbox widget.
     *
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this checkbox widget.
     * @param delegate The delegate to use to manage the state of the checkbox button.
     */
    checkbox_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept;

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observable` value which represents the state of the checkbox.
     * @param args An optional on-value, followed by an optional off-value. These two values
     *             are used to determine which value yields an on/off state.
     */
    template<typename Value, typename... Args>
    checkbox_widget(gui_window &window, widget *parent, Value &&value, Args &&...args) noexcept
        requires(not std::is_convertible_v<Value, weak_or_unique_ptr<delegate_type>>) :
        checkbox_widget(
            window,
            parent,
            make_unique_default_button_delegate<button_type::toggle>(std::forward<Value>(value), std::forward<Args>(args)...))
    {
    }

    /// @privatesection
    [[nodiscard]] bool constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override;
    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override;
    /// @endprivatesection
private:
    extent2 _button_size;
    aarectangle _button_rectangle;
    font_glyph_ids _check_glyph;
    aarectangle _check_glyph_rectangle;
    font_glyph_ids _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    checkbox_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept;

    void draw_check_box(draw_context const &context) noexcept;
    void draw_check_mark(draw_context context) noexcept;
};

} // namespace tt
