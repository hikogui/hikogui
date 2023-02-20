// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/abstract_button_widget.hpp Defines abstract_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "../animator.hpp"
#include "../i18n/translate.hpp"
#include "../notifier.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept button_widget_attribute = label_widget_attribute<Context>;

/** Base class for implementing button widgets.
 *
 * @ingroup widgets
 */
template<fixed_string Tag>
class abstract_button_widget : public widget<Tag> {
public:
    using super = widget<Tag>;
    using delegate_type = button_delegate;

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
    observer<hi::alignment> alignment;

    /** The text style to button's label.
     */
    observer<hi::text_theme> text_theme = tv<"button.text.style", hi::text_theme>{}();

    notifier<void()> pressed;

    ~abstract_button_widget()
    {
        hi_assert_not_null(delegate);
        delegate->deinit(*this);
    }

    abstract_button_widget(widget_intf *parent, std::shared_ptr<delegate_type> delegate) noexcept :
        super(parent), delegate(std::move(delegate))
    {
        hi_assert_not_null(this->delegate);

        _on_label_widget = std::make_unique<label_widget>(this, on_label, alignment, text_theme);
        _off_label_widget = std::make_unique<label_widget>(this, off_label, alignment, text_theme);
        _other_label_widget = std::make_unique<label_widget>(this, other_label, alignment, text_theme);
        _delegate_cbt = this->delegate->subscribe([&] {
            ++global_counter<"abstract_button_widget:delegate:relayout">;
            on = state() == button_state::on;
            process_event({gui_event_type::window_relayout});
        });
        this->delegate->init(*this);
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
        _on_label_constraints = _on_label_widget->update_constraints();
        _off_label_constraints = _off_label_widget->update_constraints();
        _other_label_constraints = _other_label_widget->update_constraints();
        return max(_on_label_constraints, _off_label_constraints, _other_label_constraints);
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        auto state_ = state();
        _on_label_widget->mode = state_ == button_state::on ? widget_mode::display : widget_mode::invisible;
        _off_label_widget->mode = state_ == button_state::off ? widget_mode::display : widget_mode::invisible;
        _other_label_widget->mode = state_ == button_state::other ? widget_mode::display : widget_mode::invisible;

        _on_label_widget->set_layout(context.transform(_on_label_shape));
        _off_label_widget->set_layout(context.transform(_off_label_shape));
        _other_label_widget->set_layout(context.transform(_other_label_shape));
    }

    [[nodiscard]] generator<widget_intf const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_on_label_widget;
        co_yield *_off_label_widget;
        co_yield *_other_label_widget;
    }

    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept final
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

        this->pressed();
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
                pressed = true;
                request_redraw();
                return true;
            }
            break;

        case gui_event_type::mouse_up:
            if (*mode >= widget_mode::partial and event.mouse().cause.left_button) {
                pressed = false;

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
protected:
    std::unique_ptr<label_widget> _on_label_widget;
    box_constraints _on_label_constraints;
    box_shape _on_label_shape;

    std::unique_ptr<label_widget> _off_label_widget;
    box_constraints _off_label_constraints;
    box_shape _off_label_shape;

    std::unique_ptr<label_widget> _other_label_widget;
    box_constraints _other_label_constraints;
    box_shape _other_label_shape;

    bool _pressed = false;
    notifier<>::callback_token _delegate_cbt;

    template<size_t I>
    void set_attributes() noexcept
    {
    }

    template<size_t I>
    void set_attributes(button_widget_attribute auto&& first, button_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::label>>) {
            if constexpr (I == 0) {
                on_label = first;
                off_label = first;
                other_label = hi_forward(first);
            } else if constexpr (I == 1) {
                other_label.reset();
                off_label.reset();
                off_label = hi_forward(first);
            } else if constexpr (I == 2) {
                other_label = hi_forward(first);
            } else {
                hi_static_no_default();
            }
            set_attributes<I + 1>(hi_forward(rest)...);

        } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
            set_attributes<I>(hi_forward(rest)...);

        } else if constexpr (forward_of<decltype(first), observer<hi::text_theme>>) {
            text_theme = hi_forward(first);
            set_attributes<I>(hi_forward(rest)...);

        } else {
            hi_static_no_default();
        }
    }

    void draw_button(draw_context const& context) noexcept
    {
        _on_label_widget->draw(context);
        _off_label_widget->draw(context);
        _other_label_widget->draw(context);
    }
};

}} // namespace hi::v1
