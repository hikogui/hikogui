// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/checkbox_widget.hpp Defines checkbox_widget.
 * @ingroup widgets
 */

#pragma once

#include "checkbox_delegate.hpp"
#include "../log.hpp"

namespace hi { inline namespace v1 {

template<typename Context>
concept checkbox_widget_attribute = label_widget_attribute<Context>;

/** A GUI widget that permits the user to make a binary choice.
 * @ingroup widgets
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
 * @snippet widgets/checkbox_example_impl.cpp Create a checkbox
 */
template<fixed_string Name = "">
class checkbox_widget final : public widget {
public:
    using super = widget;
    using delegate_type = checkbox_delegate;
    constexpr static auto prefix = Name / "checkbox";

    /** The delegate that controls the button widget.
     */
    std::shared_ptr<delegate_type> delegate;

    /** The label to show when the button is in the 'on' state.
     */
    observer<label> on_label = tr("on");

    /** The label to show when the button is in the 'off' state.
     */
    observer<label> off_label = tr("off");

    /** The label to show when the button is in the 'other' state.
     */
    observer<label> other_label = tr("other");

    /** The alignment of the button and on/off/other label.
     */
    observer<hi::alignment> alignment = alignment::top_left();

    /** Construct a checkbox widget.
     *
     * @param parent The parent widget that owns this checkbox widget.
     * @param delegate The delegate to use to manage the state of the checkbox button.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    checkbox_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        checkbox_widget_attribute auto&&...attributes) noexcept :
        super(parent), delegate(std::move(delegate))
    {
        hi_assert_not_null(this->delegate);
        this->set_attributes<0>(hi_forward(attributes)...);

        _on_label_widget = std::make_unique<label_widget<prefix / "on">>(this, on_label, alignment);
        _off_label_widget = std::make_unique<label_widget<prefix / "off">>(this, off_label, alignment);
        _other_label_widget = std::make_unique<label_widget<prefix / "other">>(this, other_label, alignment);

        _grid.add_cell(0, 0, cell_type::button);
        _grid.add_cell(1, 0, cell_type::label);

        _delegate_cbt = this->delegate->subscribe([&] {
            ++global_counter<"checkbox_widget:delegate:redraw">;
            hi_assert_not_null(this->delegate);
            state = this->delegate->state(this);

            _on_label_widget->mode = *state == widget_state::on ? widget_mode::display : widget_mode::invisible;
            _off_label_widget->mode = *state == widget_state::off ? widget_mode::display : widget_mode::invisible;
            _other_label_widget->mode = *state == widget_state::other ? widget_mode::display : widget_mode::invisible;

            process_event({gui_event_type::window_redraw});
        });
        this->delegate->init(*this);
        (*_delegate_cbt)();
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    checkbox_widget(
        widget *parent,
        different_from<std::shared_ptr<delegate_type>> auto&& value,
        checkbox_widget_attribute auto&&...attributes) noexcept
        requires requires { make_default_checkbox_delegate(hi_forward(value)); }
        : checkbox_widget(parent, make_default_checkbox_delegate(hi_forward(value)), hi_forward(attributes)...)
    {
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        checkbox_widget_attribute... Attributes>
    checkbox_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires { make_default_checkbox_delegate(hi_forward(value), hi_forward(on_value)); }
        :
        checkbox_widget(
            parent,
            make_default_checkbox_delegate(hi_forward(value), hi_forward(on_value)),
            hi_forward(attributes)...)
    {
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     * @param off_value The off-value. This value is used to determine which value yields an 'off' state.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        forward_of<observer<observer_decay_t<Value>>> OffValue,
        checkbox_widget_attribute... Attributes>
    checkbox_widget(widget *parent, Value&& value, OnValue&& on_value, OffValue&& off_value, Attributes&&...attributes) noexcept
        requires requires { make_default_checkbox_delegate(hi_forward(value), hi_forward(on_value), hi_forward(off_value)); }
        :
        checkbox_widget(
            parent,
            make_default_checkbox_delegate(hi_forward(value), hi_forward(on_value), hi_forward(off_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _check_glyph = find_glyph(elusive_icon::Ok);
        _minus_glyph = find_glyph(elusive_icon::Minus);

        for (auto& cell : _grid) {
            if (cell.value == cell_type::button) {
                cell.set_constraints(box_constraints{
                    theme<prefix>.size(this),
                    theme<prefix>.size(this),
                    theme<prefix>.size(this),
                    *alignment,
                    theme<prefix>.margin(this),
                    -vector2::infinity()});

            } else if (cell.value == cell_type::label) {
                cell.set_constraints(
                    max(_on_label_widget->update_constraints(),
                        _off_label_widget->update_constraints(),
                        _other_label_widget->update_constraints()));

            } else {
                hi_no_default();
            }
        }

        return _grid.constraints(os_settings::left_to_right());
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(layout, context)) {
            _grid.set_layout(context.shape, theme<prefix>.cap_height(this));
        }

        for (hilet& cell : _grid) {
            if (cell.value == cell_type::button) {
                _button_rectangle = align(cell.shape.rectangle, theme<prefix>.size(this), *alignment);

                hilet mark_size = std::min(theme<prefix / "mark">.width(this), theme<prefix / "mark">.height(this));
                hilet check_glyph_bb = _check_glyph.get_bounding_rectangle() * mark_size;
                hilet minus_glyph_bb = _minus_glyph.get_bounding_rectangle() * mark_size;

                _check_glyph_rectangle = align(_button_rectangle, check_glyph_bb, alignment::middle_center());
                _minus_glyph_rectangle = align(_button_rectangle, minus_glyph_bb, alignment::middle_center());

            } else if (cell.value == cell_type::label) {
                _on_label_widget->set_layout(context.transform(cell.shape, 0.0f));
                _off_label_widget->set_layout(context.transform(cell.shape, 0.0f));
                _other_label_widget->set_layout(context.transform(cell.shape, 0.0f));

            } else {
                hi_no_default();
            }
        }
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout)) {
            for (hilet& cell : _grid) {
                if (cell.value == cell_type::button) {
                    draw_button(context);
                    draw_mark(context);

                } else if (cell.value == cell_type::label) {
                    _on_label_widget->draw(context);
                    _off_label_widget->draw(context);
                    _other_label_widget->draw(context);

                } else {
                    hi_no_default();
                }
            }
        }
    }

    [[nodiscard]] generator<widget const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_on_label_widget;
        co_yield *_off_label_widget;
        co_yield *_other_label_widget;
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
        hi_axiom(loop::main().on_thread());
        return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::normal);
    }

    void activate() noexcept
    {
        hi_assert_not_null(delegate);
        delegate->activate(*this);
        this->_state_changed();
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
        case gui_event_type::gui_activate:
            if (*mode >= widget_mode::partial) {
                activate();
                return true;
            }
            break;

        case gui_event_type::mouse_down:
            if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
                clicked = true;
                request_redraw();
                return true;
            }
            break;

        case gui_event_type::mouse_up:
            if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
                clicked = false;

                if (layout.rectangle().contains(event.mouse().position)) {
                    handle_event(gui_event_type::gui_activate);
                }
                request_redraw();
                return true;
            }
            break;

        default:;
        }

        return super::handle_event(event);
    }
    /// @endprivatesection
private:
    enum class cell_type { button, label };

    grid_layout<cell_type> _grid;

    std::unique_ptr<label_widget<prefix / "on">> _on_label_widget;
    std::unique_ptr<label_widget<prefix / "off">> _off_label_widget;
    std::unique_ptr<label_widget<prefix / "other">> _other_label_widget;

    notifier<>::callback_token _delegate_cbt;

    aarectangle _button_rectangle;

    font_book::font_glyph_type _check_glyph;
    aarectangle _check_glyph_rectangle;
    font_book::font_glyph_type _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    void draw_button(widget_draw_context& context) noexcept
    {
        context.draw_box(
            this->layout,
            _button_rectangle,
            theme<prefix>.background_color(this),
            theme<prefix>.border_color(this),
            theme<prefix>.border_width(this),
            border_side::outside,
            theme<prefix>.border_radius(this));
    }

    void draw_mark(widget_draw_context& context) noexcept
    {
        if (this->state == widget_state::on) {
            // Checkmark
            context.draw_glyph(
                this->layout,
                translate_z(0.1f) * _check_glyph_rectangle,
                *_check_glyph.font,
                _check_glyph.glyph,
                theme<prefix>.fill_color(this));

        } else if (this->state == widget_state::off) {
            ;

        } else {
            // Tri-state
            context.draw_glyph(
                this->layout,
                translate_z(0.1f) * _minus_glyph_rectangle,
                *_minus_glyph.font,
                _minus_glyph.glyph,
                theme<prefix>.fill_color(this));
        }
    }

    template<size_t LabelCount>
    void set_attributes() noexcept
    {
    }

    template<size_t LabelCount>
    void set_attributes(checkbox_widget_attribute auto&& first, checkbox_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::label>>) {
            if constexpr (LabelCount == 0) {
                on_label = first;
                off_label = first;
                other_label = hi_forward(first);
            } else if constexpr (LabelCount == 1) {
                other_label.reset();
                off_label.reset();
                off_label = hi_forward(first);
            } else if constexpr (LabelCount == 2) {
                other_label = hi_forward(first);
            } else {
                hi_static_no_default();
            }
            set_attributes<LabelCount + 1>(hi_forward(rest)...);

        } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
            set_attributes<LabelCount>(hi_forward(rest)...);

        } else {
            hi_static_no_default();
        }
    }
};

}} // namespace hi::v1
