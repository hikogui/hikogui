// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"
#include "../log.hpp"

namespace hi::inline v1 {

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

    /** Construct a checkbox widget.
     *
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this checkbox widget.
     * @param delegate The delegate to use to manage the state of the checkbox button.
     */
    checkbox_widget(gui_window& window, widget *parent, std::shared_ptr<delegate_type> delegate) noexcept;

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observable` value which represents the state of the checkbox.
     * @param args An optional on-value, followed by an optional off-value. These two values
     *             are used to determine which value yields an on/off state.
     */
    checkbox_widget(
        gui_window& window,
        widget *parent,
        different_from<std::shared_ptr<delegate_type>> auto&& value,
        different_from<std::shared_ptr<delegate_type>> auto&&...args) noexcept requires requires
    {
        make_default_button_delegate<button_type::toggle>(hi_forward(value), hi_forward(args)...);
    } : checkbox_widget(window, parent, make_default_button_delegate<button_type::toggle>(hi_forward(value), hi_forward(args)...))
    {
    }

    /// @privatesection
    widget_constraints const& set_constraints() noexcept override;
    void set_layout(widget_layout const& layout) noexcept override;
    void draw(draw_context const& context) noexcept override;
    /// @endprivatesection
private:
    extent2 _button_size;
    aarectangle _button_rectangle;
    glyph_ids _check_glyph;
    aarectangle _check_glyph_rectangle;
    glyph_ids _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    void draw_check_box(draw_context const& context) noexcept;
    void draw_check_mark(draw_context const& context) noexcept;
};

} // namespace hi::inline v1
