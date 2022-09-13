// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace hi::inline v1 {

/** A button that is part of a menu.
 *
 * A menu-button has two different states with different visual
 * representation:
 *  - **on**: The menu button shows a check mark next to the label.
 *  - **other**: The menu button shows just the label.
 *
 * Each time a user activates the menu-button it switches its state to 'on'.
 * Most menus will close the menu after the menu button was activated.
 *
 * A menu button cannot itself switch state to 'other', this state may be
 * caused by external factors. The canonical example is another menu button in
 * a set, which is configured with a different `on_value`.
 */
class menu_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    /** Construct a menu button widget.
     *
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this menu button widget.
     * @param label The label to show in the menu button.
     * @param delegate The delegate to use to manage the state of the menu button.
     */
    template<forward_of<observer<hi::label>> Label>
    menu_button_widget(gui_window& window, widget *parent, std::shared_ptr<delegate_type> delegate, Label&& label) noexcept :
        super(window, parent, std::move(delegate))
    {
        alignment = alignment::middle_left();
        set_label(std::forward<Label>(label));
    }

    /** Construct a menu button widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this menu button widget.
     * @param label The label to show in the menu button.
     * @param value The value or `observer` value which represents the state
     *              of the menu button.
     * @param args An optional on-value. This value is used to determine which
     *             value yields an 'on' state.
     */
    menu_button_widget(
        gui_window& window,
        widget *parent,
        forward_of<observer<hi::label>> auto&& label,
        different_from<std::shared_ptr<delegate_type>> auto&& value,
        different_from<std::shared_ptr<delegate_type>> auto&&...args) noexcept requires requires
    {
        make_default_button_delegate<button_type::radio>(hi_forward(value), hi_forward(args)...);
    } :
        menu_button_widget(
            window,
            parent,
            make_default_button_delegate<button_type::radio>(hi_forward(value), hi_forward(args)...),
            hi_forward(label))
    {
    }

    /// @privatesection
    widget_constraints const& set_constraints() noexcept override;
    void set_layout(widget_layout const& layout) noexcept override;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    bool handle_event(gui_event const& event) noexcept override;
    /// @endprivatesection
private:
    glyph_ids _check_glyph;
    extent2 _check_size;
    aarectangle _check_rectangle;
    aarectangle _check_glyph_rectangle;
    extent2 _short_cut_size;
    aarectangle _short_cut_rectangle;

    void draw_menu_button(draw_context const& context) noexcept;
    void draw_check_mark(draw_context const& context) noexcept;
};

} // namespace hi::inline v1
