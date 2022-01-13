// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace tt::inline v1 {

/** A button that is part of a menu.
 *
 * A menu-button has two different states with different visual
 * representation:
 *  - **on**: The menu button shows a check mark next to the label.
 *  - **other**: The menu button shows just the label.
 *
 * @image html menu_button_widget.gif
 *
 * Each time a user activates the menu-button it switches its state to 'on'.
 * Most menus will close the menu after the menu button was activated.
 *
 * A menu button cannot itself switch state to 'other', this state may be
 * caused by external factors. The canonical example is another menu button in
 * a set, which is configured with a different `on_value`.
 *
 *
 * @snippet widgets/menu_button_example.cpp Create three menu buttons
 */
class menu_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    /** Construct a menu button widget.
     *
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this menu button widget.
     * @param label The label to show in the menu button.
     * @param delegate The delegate to use to manage the state of the menu button.
     */
    template<typename Label>
    menu_button_widget(gui_window &window, widget *parent, Label &&label, std::weak_ptr<delegate_type> delegate) noexcept :
        menu_button_widget(window, parent, std::forward<Label>(label), weak_or_unique_ptr{std::move(delegate)})
    {
    }

    /** Construct a menu button widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this menu button widget.
     * @param label The label to show in the menu button.
     * @param value The value or `observable` value which represents the state
     *              of the menu button.
     * @param args An optional on-value. This value is used to determine which
     *             value yields an 'on' state.
     */
    template<typename Label, typename Value, typename... Args>
    menu_button_widget(gui_window &window, widget *parent, Label &&label, Value &&value, Args &&...args) noexcept
        requires(not std::is_convertible_v<Value, weak_or_unique_ptr<delegate_type>>) :
        menu_button_widget(
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
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    [[nodiscard]] bool handle_event(command command) noexcept override;
    /// @endprivatesection
private:
    glyph_ids _check_glyph;
    extent2 _check_size;
    aarectangle _check_rectangle;
    aarectangle _check_glyph_rectangle;
    extent2 _short_cut_size;
    aarectangle _short_cut_rectangle;

    template<typename Label>
    menu_button_widget(gui_window &window, widget *parent, Label &&label, weak_or_unique_ptr<delegate_type> delegate) noexcept :
        super(window, parent, std::move(delegate))
    {
        label_alignment = alignment::middle_left();
        set_label(std::forward<Label>(label));
    }

    void draw_menu_button(draw_context const &context) noexcept;
    void draw_check_mark(draw_context const &context) noexcept;
};

} // namespace tt::inline v1
