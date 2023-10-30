// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/checkbox_widget.hpp Defines checkbox_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "with_label_widget.hpp"
#include "menu_button_widget.hpp"
#include "button_delegate.hpp"
#include "../telemetry/telemetry.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.widgets.checkbox_widget);

hi_export namespace hi { inline namespace v1 {

template<typename Context>
concept checkbox_widget_attribute =
    forward_of<Context, observer<hi::alignment>> or forward_of<Context, keyboard_focus_group>;

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

    struct attributes_type {
        observer<alignment> alignment = alignment::top_left();
        keyboard_focus_group focus_group = keyboard_focus_group::normal;

        attributes_type(attributes_type const &) noexcept = default;
        attributes_type(attributes_type &&) noexcept = default;
        attributes_type &operator=(attributes_type const &) noexcept = default;
        attributes_type &operator=(attributes_type &&) noexcept = default;

        template<checkbox_widget_attribute... Attributes>
        explicit attributes_type(Attributes &&...attributes) noexcept
        {
            set_attributes(std::forward<Attributes>(attributes)...);
        }

        void set_attributes() noexcept
        {
        }

        template<checkbox_widget_attribute First, checkbox_widget_attribute... Rest>
        void set_attributes(First&& first, Rest&&...rest) noexcept
        {
            if constexpr (forward_of<First, observer<hi::alignment>>) {
                alignment = std::forward<First>(first);

            } else if constexpr (forward_of<First, keyboard_focus_group>) {
                focus_group = std::forward<First>(first);

            } else {
                hi_static_no_default();
            }

            set_attributes(std::forward<Rest>(rest)...);
        }
    };

    attributes_type attributes;

    /** The delegate that controls the button widget.
     */
    not_null<std::shared_ptr<delegate_type>> delegate;

    template<typename... Args>
    [[nodiscard]] static not_null<std::shared_ptr<delegate_type>> make_default_delegate(Args &&...args)
        requires requires { default_toggle_button_delegate{std::forward<Args>(args)...}; }
    {
        return make_shared_ctad_not_null<default_toggle_button_delegate>(std::forward<Args>(args)...);
    }

    ~checkbox_widget()
    {
        this->delegate->deinit(*this);
    }

    /** Construct a checkbox widget.
     *
     * @param parent The parent widget that owns this checkbox widget.
     * @param delegate The delegate to use to manage the state of the checkbox button.
     */
    checkbox_widget(
        not_null<widget_intf const *> parent,
        attributes_type attributes,
        not_null<std::shared_ptr<delegate_type>> delegate) noexcept :
        super(parent), attributes(std::move(attributes)), delegate(std::move(delegate))
    {
        _delegate_cbt = this->delegate->subscribe([&]{
            this->request_redraw();
        });

        this->delegate->init(*this);
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     */
    template<incompatible_with<attributes_type> Value, checkbox_widget_attribute... Attributes>
    checkbox_widget(
        not_null<widget_intf const *> parent,
        Value&& value,
        Attributes &&...attributes) requires requires
    {
        make_default_delegate(std::forward<Value>(value));
        attributes_type{std::forward<Attributes>(attributes)...};
    } : checkbox_widget(
            parent,
            attributes_type{std::forward<Attributes>(attributes)...},
            make_default_delegate(std::forward<Value>(value)))
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
        incompatible_with<attributes_type> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        checkbox_widget_attribute... Attributes>
    checkbox_widget(
        not_null<widget_intf const *> parent,
        Value&& value,
        OnValue&& on_value,
        Attributes &&...attributes) noexcept
        requires requires
    {
        make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value));
        attributes_type{std::forward<Attributes>(attributes)...};
    } :
        checkbox_widget(
            parent,
            attributes_type{std::forward<Attributes>(attributes)...},
            make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value)))
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
        typename Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        forward_of<observer<observer_decay_t<Value>>> OffValue,
        checkbox_widget_attribute... Attributes>
    checkbox_widget(
        not_null<widget_intf const *> parent,
        Value&& value,
        OnValue&& on_value,
        OffValue&& off_value,
        Attributes &&...attributes) noexcept requires requires
    {
        make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value), std::forward<OffValue>(off_value));
        attributes_type{std::forward<Attributes>(attributes)...};
    } :
        checkbox_widget(
            parent,
            attributes_type{std::forward<Attributes>(attributes)...},
            make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value), std::forward<OffValue>(off_value)))
    {
    }

    /** Get the current state of the button.
     * @return The state of the button: on / off / other.
     */
    [[nodiscard]] button_state state() const noexcept
    {
        hi_axiom(loop::main().on_thread());
        return delegate->state(*this);
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _button_size = {theme().size(), theme().size()};
        return box_constraints{_button_size, _button_size, _button_size, *attributes.alignment, theme().margin(), {}};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _button_rectangle = align(context.rectangle(), _button_size, os_settings::alignment(*attributes.alignment));

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
            return theme().color(semantic_color::fill, _layout.layer + 2);
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
                delegate->activate(*this);
                request_redraw();
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
};

using checkbox_with_label_widget = with_label_widget<checkbox_widget>;
using checkbox_menu_button_widget = menu_button_widget<checkbox_widget>;

}} // namespace hi::v1
