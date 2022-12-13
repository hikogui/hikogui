// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/menu_button_widget.hpp Defines menu_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"

namespace hi { inline namespace v1 {

/** A button that is part of a menu.
 * @ingroup widgets
 *
 * A menu-button has two different states with different visual
 * representation:
 *  - **on**: The menu button shows a check mark next to the label.
 *  - **off**: The menu button shows just the label.
 *
 * Each time a user activates the menu-button it switches its state to 'on'.
 * Most menus will close the menu after the menu button was activated.
 *
 * A menu button cannot itself switch state to 'off', this state may be
 * caused by external factors. The canonical example is another menu button in
 * a set, which is configured with a different `on_value`.
 */
class menu_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    /** Construct a menu button widget.
     *
     * @param parent The parent widget that owns this menu button widget.
     * @param delegate The delegate to use to manage the state of the menu button.
     * @param attributes Different attributes used to configure the label's on the menu button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    menu_button_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        alignment = alignment::middle_flush();
        set_attributes<0>(hi_forward(attributes)...);
    }

    /** Construct a menu button widget with a default button delegate.
     *
     * @param parent The parent widget that owns this menu button widget.
     * @param value The value or `observer` value which represents the state
     *              of the menu button.
     * @param on_value An optional on-value. This value is used to determine which
     *             value yields an 'on' state.
     * @param attributes Different attributes used to configure the label's on the menu button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        button_widget_attribute... Attributes>
    menu_button_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires
    {
        make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value));
    } :
        menu_button_widget(
            parent,
            make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    bool handle_event(gui_event const& event) noexcept override;
    /// @endprivatesection
private:
    box_constraints _label_constraints;

    glyph_ids _check_glyph;
    extent2i _check_size;
    aarectanglei _check_rectangle;
    aarectanglei _check_glyph_rectangle;
    extent2i _short_cut_size;
    aarectanglei _short_cut_rectangle;

    void draw_menu_button(draw_context const& context) noexcept;
    void draw_check_mark(draw_context const& context) noexcept;
};

}} // namespace hi::v1
