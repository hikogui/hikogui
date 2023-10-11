// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/checkbox_widget.hpp Defines checkbox_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "with_label_widget.hpp"
#include "button_delegate.hpp"
#include "../telemetry/telemetry.hpp"
#include "../macros.hpp"

namespace hi { inline namespace v1 {

template<typename Context>
concept checkbox_widget_attribute =
    forward_of<Context, observer<hi::alignment>>;

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
class checkbox_widget final : public widget {
public:
    using super = widget;
    using delegate_type = button_delegate;

    /** The delegate that controls the button widget.
     */
    std::shared_ptr<delegate_type> delegate;

    /** The alignment of the button and on/off/other label.
     */
    observer<alignment> alignment;

    /** Notifier to await or callback on when the button was activated.
     */
    notifier<> activated;

    ~checkbox_widget()
    {
        this->delegate->deinit(*this);
    }

    /** Construct a checkbox widget.
     *
     * @param parent The parent widget that owns this checkbox widget.
     * @param delegate The delegate to use to manage the state of the checkbox button.
     */
    template<checkbox_widget_attribute... Attributes>
    checkbox_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        Attributes &&...attributes) noexcept :
        super(parent), delegate(std::move(delegate))
    {
        hi_assert_not_null(this->delegate);
        alignment = alignment::top_left();
        set_attributes<0>(std::forward<Attributes>(attributes)...);

        _delegate_cbt = this->delegate->subscribe([&]{
            this->request_redraw();
            activated();
        });

        this->delegate->init(*this);
    }

    template<typename... Args>
    [[nodiscard]] static std::shared_ptr<delegate_type> make_default_delegate(Args &&...args)
        requires requires { default_toggle_button_delegate{std::forward<Args>(args)...}; }
    {
        return make_shared_ctad<default_toggle_button_delegate>(std::forward<Args>(args)...);
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     */
    template<different_from<std::shared_ptr<delegate_type>> Value, checkbox_widget_attribute... Attributes>
    checkbox_widget(
        widget *parent,
        Value&& value,
        Attributes &&...attributes) requires requires
    {
        make_default_delegate(std::forward<Value>(value));
    } : checkbox_widget(
            parent,
            make_default_delegate(std::forward<Value>(value)),
            std::forward<Attributes>(attributes)...)
    {
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        checkbox_widget_attribute... Attributes>
    checkbox_widget(
        widget *parent,
        Value&& value,
        OnValue&& on_value,
        Attributes &&...attributes) noexcept
        requires requires
    {
        make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value));
    } :
        checkbox_widget(
            parent,
            make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value)),
            std::forward<Attributes>(attributes)...)
    {
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     * @param off_value The off-value. This value is used to determine which value yields an 'off' state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        forward_of<observer<observer_decay_t<Value>>> OffValue,
        checkbox_widget_attribute... Attributes>
    checkbox_widget(
        widget *parent,
        Value&& value,
        OnValue&& on_value,
        OffValue&& off_value,
        Attributes &&...attributes) noexcept requires requires
    {
        make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value), std::forward<OffValue>(off_value));
    } :
        checkbox_widget(
            parent,
            make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value), std::forward<OffValue>(off_value)),
            std::forward<Attributes>(attributes)...)
    {
    }

    /** Get the current state of the button.
     * @return The state of the button: on / off / other.
     */
    [[nodiscard]] button_state state() const noexcept
    {
        hi_axiom(loop::main().on_thread());
        hi_assert_not_null(delegate);
        return delegate->state(*this);
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _button_size = {theme().size(), theme().size()};
        return box_constraints{_button_size, _button_size, _button_size, *alignment, theme().margin(), {}};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _button_rectangle = align(context.rectangle(), _button_size, os_settings::alignment(*alignment));

            _check_glyph = find_glyph(elusive_icon::Ok);
            hilet check_glyph_bb = _check_glyph.get_metrics().bounding_rectangle * theme().icon_size();
            _check_glyph_rectangle = align(_button_rectangle, check_glyph_bb, alignment::middle_center());

            _minus_glyph = find_glyph(elusive_icon::Minus);
            hilet minus_glyph_bb = _minus_glyph.get_metrics().bounding_rectangle * theme().icon_size();
            _minus_glyph_rectangle = align(_button_rectangle, minus_glyph_bb, alignment::middle_center());
        }
        super::set_layout(context);
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout())) {
            context.draw_box(
                layout(), _button_rectangle, background_color(), focus_color(), theme().border_width(), border_side::inside);

            switch (state()) {
            case hi::button_state::on:
                context.draw_glyph(layout(), translate_z(0.1f) * _check_glyph_rectangle, _check_glyph, accent_color());
                break;
            case hi::button_state::off:
                break;
            default:
                context.draw_glyph(layout(), translate_z(0.1f) * _minus_glyph_rectangle, _minus_glyph, accent_color());
            }
        }
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        if (_pressed) {
            return theme().color(semantic_color::fill, semantic_layer + 2);
        } else {
            return super::background_color();
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept final
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout().contains(position)) {
            return {id, _layout.elevation, hitbox_type::button};
        } else {
            return {};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::normal);
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
        case gui_event_type::gui_activate:
            if (*mode >= widget_mode::partial) {
                hi_assert_not_null(delegate);
                delegate->activate(*this);
                ++global_counter<"checkbox_widget:handle_event:relayout">;
                process_event({gui_event_type::window_relayout});
                return true;
            }
            break;

        case gui_event_type::mouse_down:
            if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
                _pressed = true;
                request_redraw();
                return true;
            }
            break;

        case gui_event_type::mouse_up:
            if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
                _pressed = false;

                // with_label_widget or other widgets may have accepted the hitbox
                // for this widget. Which means the widget_id in the mouse-event
                // may match up with the checkbox.
                if (event.mouse().hitbox.widget_id == id) {
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
    extent2 _button_size;
    aarectangle _button_rectangle;
    font_book::font_glyph_type _check_glyph;
    aarectangle _check_glyph_rectangle;
    font_book::font_glyph_type _minus_glyph;
    aarectangle _minus_glyph_rectangle;
    bool _pressed = false;
    
    callback<void()> _delegate_cbt;

    template<size_t I>
    void set_attributes() noexcept
    {
    }

    template<size_t I>
    void set_attributes(button_widget_attribute auto&& first, button_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
            set_attributes<I>(hi_forward(rest)...);

        } else {
            hi_static_no_default();
        }
    }
};

using checkbox_with_label_widget = with_label_widget<checkbox_widget>;

}} // namespace hi::v1
