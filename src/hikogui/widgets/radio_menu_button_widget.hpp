// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/radio_menu_button_widget.hpp Defines radio_menu_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "radio_button_delegate.hpp"

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
class radio_menu_button_widget final : public widget {
public:
    using super = widget;
    using delegate_type = typename radio_button_delegate;
    constexpr static auto prefix = Name / "radio-menu";

    /** Construct a menu button widget.
     *
     * @param parent The parent widget that owns this menu button widget.
     * @param delegate The delegate to use to manage the state of the menu button.
     * @param attributes Different attributes used to configure the label's on the menu button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    radio_menu_button_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        this->alignment = alignment::middle_flush();
        this->set_attributes<0>(hi_forward(attributes)...);
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
    radio_menu_button_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires { make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)); }
        :
        radio_menu_button_widget(
            parent,
            make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = _label_widget.update_constraints();
        _mark_label_constraints = _mark_label_widget.update_constraints();
        _shortcut_label_constraints = _shortcut_label_widget.update_constraints();

        auto constraints = max(_label_constraints, _mark_label_constraints, _shortcut_label_constraints);
        inplace_max(constraints.margins, theme<prefix>.margin(this));
        _padding = constraints.margins;

        // clang-format off
        hilet extra_width =
            constraints.margins.left() +
            theme<prefix / "mark">.width(this) +
            _label_constraints.margins.left() +
            // The label is here.
            _label_constraints.margins.right() +
            theme<prefix / "short-cut">.width(this) +
            constraints.margins.right();
        // clang-format on

        // Internalize the margins inside the widget, as menu items are flush
        // with each other and their container.
        constraints.minimum.width() = _label_constraints.minimum.width() + extra_width;
        constraints.preferred.width() = _label_constraints.preferred.width() + extra_width;
        constraints.maximum.width() = _label_constraints.maximum.width() + extra_width;
        constraints.minimum.height() += constraints.margins.top() + constraints.margins.bottom();
        constraints.preferred.height() += constraints.margins.top() + constraints.margins.bottom();
        constraints.maximum.height() += constraints.margins.top() + constraints.margins.bottom();
        constraints.margins = {};
        return constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(this->layout, context)) {
            hilet outline = context.rectangle() - _padding;

            if (os_settings::left_to_right()) {
                hilet mark_outline_size = extent2{theme<prefix / "mark">.width(), outline.height()};
                hilet shortcut_outline_size = extent2{theme<prefix / "short-cut">.width(), outline.height()};

                hilet mark_shape = aarectangle{get<0>(outline), get<0>(outline) + mark_outline_size};
                hilet shortcut_shape = aarectangle{get<3>(outline) - shortcut_outline_size, get<3>(outline)};
                hilet label_shape = aarectangle{
                    point2{get<1>(mark_outline).x() + _label_constraint.margin.left(), get<1>(mark_outline).y()},
                    point2{get<2>(shortcut_outline).x() - _label_constraint.margin.right(), get<2>(shortcut_outline.y())}};

                _mark_widget->set_layout(context.transform(mark_shape, 0.1f));
                _shortcut_widget->set_layout(context.transform(shortcut_shape, 0.1f));
                _label_widget->set_layout(context.transform(label_shape, 0.1f));

            } else {
                _short_cut_rectangle = align(inside_rectangle, _short_cut_size, alignment::middle_left());
                _check_rectangle = align(inside_rectangle, _check_size, alignment::middle_right());
                hilet label_rectangle = aarectangle{
                    point2{_short_cut_rectangle.right() + spacing, 0},
                    point2{_check_rectangle.left() - spacing, context.height()}};
                this->_on_label_shape = this->_off_label_shape = this->_other_label_shape =
                    box_shape{_label_constraints, label_rectangle, cap_height};
            }
        }
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*this->mode > widget_mode::invisible and overlaps(context, this->layout)) {
            draw_button(context);
            draw_check(context);
        }
        _label_widget->draw(context);
    }

    [[nodiscard]] generator<widget const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_label_widget;
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept final
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout.contains(position)) {
            return {id, layout.elevation, hitbox_type::button};
        } else {
            return {};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return *this->mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::menu);
    }

    void activate() noexcept
    {
        hi_assert_not_null(delegate);
        delegate->activate(*this);
        this->_state_changed();
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        using enum gui_event_type;

        switch (event.type()) {
        case gui_menu_next:
            if (*this->mode >= widget_mode::partial and not this->is_last(keyboard_focus_group::menu)) {
                this->process_event(gui_event::window_set_keyboard_target(
                    nullptr, keyboard_focus_group::menu, keyboard_focus_direction::forward));
                return true;
            }
            break;

        case gui_menu_prev:
            if (*this->mode >= widget_mode::partial and not this->is_first(keyboard_focus_group::menu)) {
                this->process_event(gui_event::window_set_keyboard_target(
                    nullptr, keyboard_focus_group::menu, keyboard_focus_direction::backward));
                return true;
            }
            break;

        case gui_activate:
            if (*this->mode >= widget_mode::partial) {
                this->activate();
                this->process_event(gui_event::window_set_keyboard_target(
                    nullptr, keyboard_focus_group::normal, keyboard_focus_direction::forward));
                this->process_event(gui_event::window_set_keyboard_target(
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
    std::unique_ptr<label_widget<prefix>> _label_widget;
    box_constraints _label_constraints;

    notifier<>::callback_token _delegate_cbt;

    font_book::font_glyph_type _check_glyph;
    aarectangle _check_glyph_rectangle;

    void draw_button(widget_draw_context& context) noexcept
    {
        context.draw_box(
            this->layout,
            this->layout.rectangle(),
            theme<prefix>.background_color(this),
            theme<prefix>.border_color(this),
            theme<prefix>.border_width(this),
            border_side::inside);
    }

    void draw_check(widget_draw_context& context) noexcept
    {
        if (this->state != widget_state::off) {
            context.draw_glyph(
                this->layout,
                translate_z(0.1f) * _check_glyph_rectangle,
                *_check_glyph.font,
                _check_glyph.glyph,
                theme<prefix>.fill_color(this));
        }
    }
};
}} // namespace hi::v1
