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
#include "toggle_delegate.hpp"
#include "../telemetry/telemetry.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.widgets.checkbox_widget);

hi_export namespace hi {
inline namespace v1 {

template<typename Context>
struct is_checkbox_widget_attribute {
    constexpr static bool value = forward_of<Context, observer<hi::alignment>> or forward_of<Context, keyboard_focus_group>;
};

template<typename Context>
concept checkbox_widget_attribute = is_checkbox_widget_attribute<Context>::value;

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
class checkbox_widget : public widget {
public:
    using super = widget;
    using delegate_type = toggle_delegate;

    struct attributes_type {
        observer<alignment> alignment = alignment::top_left();
        keyboard_focus_group focus_group = keyboard_focus_group::normal;

        attributes_type(attributes_type const&) noexcept = default;
        attributes_type(attributes_type&&) noexcept = default;
        attributes_type& operator=(attributes_type const&) noexcept = default;
        attributes_type& operator=(attributes_type&&) noexcept = default;

        template<checkbox_widget_attribute... Attributes>
        explicit attributes_type(Attributes&&...attributes) noexcept
        {
            set_attributes(std::forward<Attributes>(attributes)...);
        }

        void set_attributes() noexcept {}

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
    std::shared_ptr<delegate_type> delegate;

    hi_num_valid_arguments(consteval static, num_default_delegate_arguments, default_toggle_delegate);
    hi_call_left_arguments(static, make_default_delegate, make_shared_ctad<default_toggle_delegate>);
    hi_call_right_arguments(static, make_attributes, attributes_type);

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
        attributes_type attributes,
        std::shared_ptr<delegate_type> delegate) noexcept :
        super(), attributes(std::move(attributes)), delegate(std::move(delegate))
    {
        hi_axiom_not_null(this->delegate);

        this->delegate->init(*this);
        _delegate_cbt = this->delegate->subscribe([&] {
            set_value(this->delegate->state(*this));
        });
        _delegate_cbt();
        
        style.set_name("checkbox");
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @param parent The parent widget that owns this toggle widget.
     * @param args The arguments to the `default_toggle_delegate`
     *                followed by arguments to `attributes_type`
     */
    template<typename... Args>
    checkbox_widget(Args&&...args)
        requires(num_default_delegate_arguments<Args...>() != 0)
        :
        checkbox_widget(
            parent,
            make_attributes<num_default_delegate_arguments<Args...>()>(std::forward<Args>(args)...),
            make_default_delegate<num_default_delegate_arguments<Args...>()>(std::forward<Args>(args)...))
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        return box_constraints{style.size_px, style.size_px, style.size_px, style.alignment, style.margins_px};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _button_rectangle = align(context.rectangle(), style.size_px, os_settings::alignment(style.alignment));

            _check_glyph = find_glyph(elusive_icon::Ok);
            auto const check_glyph_bb = _check_glyph.front_glyph_metrics().bounding_rectangle * style.font_size_px;
            _check_glyph_rectangle = align(_button_rectangle, check_glyph_bb, alignment::middle_center());

            _minus_glyph = find_glyph(elusive_icon::Minus);
            auto const minus_glyph_bb = _minus_glyph.front_glyph_metrics().bounding_rectangle * style.font_size_px;
            _minus_glyph_rectangle = align(_button_rectangle, minus_glyph_bb, alignment::middle_center());
        }
        super::set_layout(context);
    }

    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            context.draw_box(
                layout(), _button_rectangle, style.background_color, style.border_color, style.border_width_px, border_side::inside);

            switch (value()) {
            case widget_value::on:
                context.draw_glyph(layout(), translate_z(0.1f) * _check_glyph_rectangle, _check_glyph, style.accent_color);
                break;
            case widget_value::off:
                break;
            default:
                context.draw_glyph(layout(), translate_z(0.1f) * _minus_glyph_rectangle, _minus_glyph, style.accent_color);
            }
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (mode() >= widget_mode::partial and layout().contains(position)) {
            return {id, _layout.elevation, hitbox_type::button};
        } else {
            return {};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        return mode() >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::normal);
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
        case gui_event_type::gui_activate:
            if (mode() >= widget_mode::partial) {
                delegate->activate(*this);
                request_redraw();
                return true;
            }
            break;

        case gui_event_type::mouse_down:
            if (mode() >= widget_mode::partial and event.mouse().cause.left_button) {
                set_pressed(true);
                return true;
            }
            break;

        case gui_event_type::mouse_up:
            if (mode() >= widget_mode::partial and event.mouse().cause.left_button) {
                set_pressed(false);

                // with_label_widget or other widgets may have accepted the hitbox
                // for this widget. Which means the widget_id in the mouse-event
                // may match up with the checkbox.
                if (event.mouse().hitbox.widget_id == id) {
                    handle_event(gui_event_type::gui_activate);
                }
                return true;
            }
            break;

        default:;
        }

        return super::handle_event(event);
    }
    /// @endprivatesection

private:
    aarectangle _button_rectangle;
    font_glyph_ids _check_glyph;
    aarectangle _check_glyph_rectangle;
    font_glyph_ids _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    callback<void()> _delegate_cbt;
};

using checkbox_with_label_widget = with_label_widget<checkbox_widget>;
using checkbox_menu_button_widget = menu_button_widget<checkbox_widget>;

} // namespace v1
} // namespace hi::v1
