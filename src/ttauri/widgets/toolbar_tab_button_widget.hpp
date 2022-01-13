// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace tt::inline v1 {

/** A graphical control element that allows the user to choose only one of a
 * predefined set of mutually exclusive views of a `tab_widget`.
 *
 * A toolbar tab button generally controls a `tab_widget`, to show one of its
 * child widgets.
 *
 * A toolbar tab button has two different states with different visual
 * representation:
 *  - **on**: The toolbar tab button shows raised among the other tabs.
 *  - **other**: The toolbar tab button is at equal height to other tabs.
 *
 * @image html toolbar_tab_button_widget.gif
 *
 * Each time a user activates the toolbar tab button it switches its state to
 * 'on'.
 *
 * A toolbar tab button cannot itself switch state to 'other', this state may be
 * caused by external factors. The canonical example is another toolbar tab
 * button in a set, which is configured with a different `on_value`.
 *
 * In the following example we create three toolbar tab button widgets on the
 * window which observes the same `value`. Each tab button is configured with a
 * different `on_value`: 0, 1 and 2.
 *
 * @snippet widgets/tab_example.cpp Create three toolbar tab buttons
 *
 * @note A toolbar tab button does not directly control a `tab_widget`. Like
 *       `radio_button_widget` this is accomplished by sharing a delegate or a
 *       observable between the toolbar tab button and the tab widget.
 */
class toolbar_tab_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    /** Construct a toolbar tab button widget.
     *
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this radio button widget.
     * @param label The label to show in the tab button.
     * @param delegate The delegate to use to manage the state of the tab button widget.
     */
    template<typename Label>
    toolbar_tab_button_widget(gui_window &window, widget *parent, Label &&label, std::weak_ptr<delegate_type> delegate) noexcept :
        toolbar_tab_button_widget(window, parent, std::forward<Label>(label), weak_or_unique_ptr{std::move(delegate)})
    {
    }

    /** Construct a toolbar tab button widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this radio button widget.
     * @param label The label to show in the tab button.
     * @param value The value or `observable` value which represents the state
     *              of the tab button.
     * @param args An optional on-value. This value is used to determine which
     *             value yields an 'on' state.
     */
    template<typename Label, typename Value, typename... Args>
    toolbar_tab_button_widget(gui_window &window, widget *parent, Label &&label, Value &&value, Args &&...args) noexcept
        requires(not std::is_convertible_v<Value, weak_or_unique_ptr<delegate_type>>) :
        toolbar_tab_button_widget(
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
    void request_redraw() const noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    [[nodiscard]] bool handle_event(command command) noexcept override;
    // @endprivatesection
private:
    template<typename Label>
    toolbar_tab_button_widget(
        gui_window &window,
        widget *parent,
        Label &&label,
        weak_or_unique_ptr<delegate_type> delegate) noexcept :
        super(window, parent, std::move(delegate))
    {
        label_alignment = alignment::top_center();
        set_label(std::forward<Label>(label));
    }

    void draw_toolbar_tab_button(draw_context const &context) noexcept;
};

} // namespace tt::inline v1
