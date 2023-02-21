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
template<fixed_string Name = "">
class menu_button_widget final : public abstract_button_widget<Name ^ "menu-button"> {
public:
    using super = abstract_button_widget<Name ^ "menu-button">;
    using delegate_type = typename super::delegate_type;

    /** Construct a menu button widget.
     *
     * @param parent The parent widget that owns this menu button widget.
     * @param delegate The delegate to use to manage the state of the menu button.
     * @param attributes Different attributes used to configure the label's on the menu button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
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
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        button_widget_attribute... Attributes>
    menu_button_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires { make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)); }
        :
        menu_button_widget(
            parent,
            make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = super::update_constraints();

        // Make room for button and margin.
        _check_size = theme<prefix ^ "icon.size", extent2i>{}(this);
        _short_cut_size = theme<prefix ^ "icon.size", extent2i>{}(this);

        // On left side a check mark, on right side short-cut. Around the label extra margin.
        hilet extra_size = extent2i{
            theme<prefix ^ "margin", int>{}(this) * 4 + _check_size.width() + _short_cut_size.width(),
            theme<prefix ^ "margin", int>{}(this) * 2};

        auto constraints = _label_constraints + extra_size;
        constraints.margins = 0;
        return constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(layout, context)) {
            hilet inside_rectangle = context.rectangle() - theme().margin<int>();

            if (os_settings::left_to_right()) {
                _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_left());
                _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_right());
                hilet label_rectangle = aarectanglei{
                    point2i{_check_rectangle.right() + theme().margin<int>(), 0},
                    point2i{_short_cut_rectangle.left() - theme().margin<int>(), context.height()}};
                _on_label_shape = _off_label_shape = _other_label_shape =
                    box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};

            } else {
                _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_left());
                _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_right());
                hilet label_rectangle = aarectanglei{
                    point2i{_short_cut_rectangle.right() + theme().margin<int>(), 0},
                    point2i{_check_rectangle.left() - theme().margin<int>(), context.height()}};
                _on_label_shape = _off_label_shape = _other_label_shape =
                    box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};
            }

            _check_glyph = find_glyph(elusive_icon::Ok);
            hilet check_glyph_bb = narrow_cast<aarectanglei>(_check_glyph.get_bounding_box() * theme().icon_size<float>());
            _check_glyph_rectangle = align(_check_rectangle, check_glyph_bb, alignment::middle_center());
        }

        super::set_layout(context);
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout)) {
            draw_menu_button(context);
            draw_check_mark(context);
            draw_button(context);
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::menu);
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        using enum gui_event_type;

        switch (event.type()) {
        case gui_menu_next:
            if (*mode >= widget_mode::partial and not is_last(keyboard_focus_group::menu)) {
                process_event(gui_event::window_set_keyboard_target(
                    nullptr, keyboard_focus_group::menu, keyboard_focus_direction::forward));
                return true;
            }
            break;

        case gui_menu_prev:
            if (*mode >= widget_mode::partial and not is_first(keyboard_focus_group::menu)) {
                process_event(gui_event::window_set_keyboard_target(
                    nullptr, keyboard_focus_group::menu, keyboard_focus_direction::backward));
                return true;
            }
            break;

        case gui_activate:
            if (*mode >= widget_mode::partial) {
                activate();
                process_event(gui_event::window_set_keyboard_target(
                    nullptr, keyboard_focus_group::normal, keyboard_focus_direction::forward));
                process_event(gui_event::window_set_keyboard_target(
                    nullptr, keyboard_focus_group::normal, keyboard_focus_direction::backward));
                return true;
            }
            break;

        default:;
        }

        return super::handle_event(event);
    }
    /// @endprivatesection
private:
    box_constraints _label_constraints;

    glyph_ids _check_glyph;
    extent2i _check_size;
    aarectanglei _check_rectangle;
    aarectanglei _check_glyph_rectangle;
    extent2i _short_cut_size;
    aarectanglei _short_cut_rectangle;

    void draw_menu_button(draw_context const& context) noexcept
    {
        hilet border_color = *focus ? focus_color() : color::transparent();
        context.draw_box(
            layout,
            layout.rectangle(),
            theme<prefix ^ "fill.color", color>{}(this),
            theme<prefix ^ "outline.color", color>{}(this),
            theme<prefix ^ "outline.width", int>{}(this),
            border_side::inside);
    }

    void draw_check_mark(draw_context const& context) noexcept
    {
        auto state_ = state();

        // Checkmark or tristate.
        if (state_ == hi::button_state::on) {
            context.draw_glyph(
                layout,
                translate_z(0.1f) * narrow_cast<aarectangle>(_check_glyph_rectangle),
                _check_glyph,
                theme<prefix ^ "accent.color", color>{}(this));
        }
    }
};

}} // namespace hi::v1
