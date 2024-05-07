// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/async_widget.hpp Defines async_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "with_label_widget.hpp"
#include "menu_button_widget.hpp"
#include "async_delegate.hpp"
#include "../telemetry/telemetry.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.widgets.async_widget);

hi_export namespace hi { inline namespace v1 {

template<typename Context>
concept async_widget_attribute =
    label_widget_attribute<Context> or forward_of<Context, keyboard_focus_group>;

/** A GUI widget that permits the user to make a binary choice.
 * @ingroup widgets
 *
 * A async is a button with three different states with different visual
 * representation:
 *  - **on**: A check-mark is shown inside the box, and the `async_widget::on_label` is shown.
 *  - **off**: An empty box is shown, and the `async_widget::off_label` is shown.
 *
 * @image html async_widget.gif
 *
 * Each time a user activates the async-button it toggles between the 'on' and 'off' states.
 * If the async is in the 'other' state an activation will switch it to
 * the 'off' state.
 *
 * In the following example we create a async widget on the window
 * which observes `value`. When the value is 1 the async is 'on',
 * when the value is 2 the async is 'off'.
 *
 * @snippet widgets/async_example_impl.cpp Create a async
 */
class async_widget : public widget {
public:
    using super = widget;
    using delegate_type = async_delegate;

    struct attributes_type {
        observer<alignment> alignment = alignment::top_left();
        observer<hi::label> label = hi::txt{"<label>"};
        keyboard_focus_group focus_group = keyboard_focus_group::normal;

        attributes_type(attributes_type const &) noexcept = default;
        attributes_type(attributes_type &&) noexcept = default;
        attributes_type &operator=(attributes_type const &) noexcept = default;
        attributes_type &operator=(attributes_type &&) noexcept = default;

        template<async_widget_attribute... Attributes>
        explicit attributes_type(Attributes &&...attributes) noexcept
        {
            set_attributes(std::forward<Attributes>(attributes)...);
        }

        void set_attributes() noexcept
        {
        }

        template<async_widget_attribute First, async_widget_attribute... Rest>
        void set_attributes(First&& first, Rest&&...rest) noexcept
        {
            if constexpr (forward_of<First, observer<hi::alignment>>) {
                alignment = std::forward<First>(first);

            } else if constexpr (forward_of<First, observer<hi::label>>) {
                label = std::forward<First>(first);

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

    template<typename... Args>
    [[nodiscard]] static std::shared_ptr<delegate_type> make_default_delegate(Args &&...args)
        requires requires { default_async_delegate{std::forward<Args>(args)...}; }
    {
        return make_shared_ctad<default_async_delegate>(std::forward<Args>(args)...);
    }

    ~async_widget()
    {
        this->delegate->deinit(*this);
    }

    /** Construct a async widget.
     *
     * @param parent The parent widget that owns this async widget.
     * @param delegate The delegate to use to manage the state of the async button.
     */
    async_widget(
        widget_intf const* parent,
        attributes_type attributes,
        std::shared_ptr<delegate_type> delegate) noexcept :
        super(parent), attributes(std::move(attributes)), delegate(std::move(delegate))
    {
        this->delegate->init(*this);
        _delegate_cbt = this->delegate->subscribe([&] {
            set_value(this->delegate->state(*this));
        });
        _delegate_cbt();
    }

    /** Construct a async widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this async widget.
     * @param value The value or `observer` value which represents the state of the async.
     */
    template<incompatible_with<attributes_type> Value, async_widget_attribute... Attributes>
    async_widget(
        widget_intf const* parent,
        Value&& value,
        Attributes &&...attributes) requires requires
    {
        make_default_delegate(std::forward<Value>(value));
        attributes_type{std::forward<Attributes>(attributes)...};
    } : async_widget(
            parent,
            attributes_type{std::forward<Attributes>(attributes)...},
            make_default_delegate(std::forward<Value>(value)))
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _button_size = {theme().size(), theme().size()};
        return box_constraints{_button_size, _button_size, _button_size, *attributes.alignment, theme().margin()};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _button_rectangle = align(context.rectangle(), _button_size, os_settings::alignment(*attributes.alignment));

            _check_glyph = find_glyph(elusive_icon::Ok);
            auto const check_glyph_bb = _check_glyph.front_glyph_metrics().bounding_rectangle * theme().icon_size();
            _check_glyph_rectangle = align(_button_rectangle, check_glyph_bb, alignment::middle_center());

            _minus_glyph = find_glyph(elusive_icon::Minus);
            auto const minus_glyph_bb = _minus_glyph.front_glyph_metrics().bounding_rectangle * theme().icon_size();
            _minus_glyph_rectangle = align(_button_rectangle, minus_glyph_bb, alignment::middle_center());
        }
        super::set_layout(context);
    }

    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            context.draw_box(
                layout(), _button_rectangle, background_color(), focus_color(), theme().border_width(), border_side::inside);

            switch (value()) {
            case widget_value::on:
                context.draw_glyph(layout(), translate_z(0.1f) * _check_glyph_rectangle, _check_glyph, accent_color());
                break;
            case widget_value::off:
                break;
            default:
                context.draw_glyph(layout(), translate_z(0.1f) * _minus_glyph_rectangle, _minus_glyph, accent_color());
            }
        }
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        if (phase() == widget_phase::pressed) {
            return theme().fill_color(_layout.layer + 2);
        } else {
            return super::background_color();
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
                // may match up with the async.
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
    extent2 _button_size;
    aarectangle _button_rectangle;
    font_glyph_ids _check_glyph;
    aarectangle _check_glyph_rectangle;
    font_glyph_ids _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    callback<void()> _delegate_cbt;
};

using async_with_label_widget = with_label_widget<async_widget>;
using async_menu_button_widget = menu_button_widget<async_widget>;

}} // namespace hi::v1
