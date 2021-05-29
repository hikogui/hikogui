// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"

namespace tt {

/** An abstract button widget.
 * This widgets implements the behavior for a widget where its whole
 * area is clickable, accepts and responds to gui_activate commands.
 */
template<typename T>
class abstract_button_widget : public widget {
public:
    using super = widget;
    using value_type = T;
    using delegate_type = button_delegate<value_type>;
    using callback_ptr_type = typename button_delegate<value_type>::pressed_callback_ptr_type;

    template<typename Value>
    [[nodiscard]] abstract_button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate,
        Value &&value) :
        super(window, parent, delegate)
    {
        set_value(std::forward<Value>(value));
    }

    [[nodiscard]] button_state state() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return this->delegate<delegate_type>().state(*this);
    }

    [[nodiscard]] tt::label label() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return this->delegate<delegate_type>().label(*this);
    }

    void set_label(tt::label const &label) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        set_on_label(label);
        set_off_label(label);
        set_other_label(label);
    }

    [[nodiscard]] tt::label on_label() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return this->delegate<delegate_type>().on_label(*this);
    }

    void set_on_label(tt::label const &label) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        this->delegate<delegate_type>().set_on_label(*this, label);
    }

    [[nodiscard]] tt::label off_label() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return this->delegate<delegate_type>().off_label(*this);
    }

    void set_off_label(tt::label const &label) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        this->delegate<delegate_type>().set_off_label(*this, label);
    }

    [[nodiscard]] tt::label other_label() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return this->delegate<delegate_type>().other_label(*this);
    }

    void set_other_label(tt::label const &label) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        this->delegate<delegate_type>().set_other_label(*this, label);
    }

    [[nodiscard]] value_type value() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return this->delegate<delegate_type>().value(*this);
    }

    template<typename Value>
    void set_value(Value &&value) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return this->delegate<delegate_type>().set_value(*this, std::forward<Value>(value));
    }

    [[nodiscard]] value_type on_value() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return this->delegate<delegate_type>().on_value(*this);
    }

    void set_on_value(value_type const &value) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        this->delegate<delegate_type>().set_on_value(*this, value);
    }

    [[nodiscard]] value_type off_value() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return this->delegate<delegate_type>().off_value(*this);
    }

    void set_off_value(value_type const &value) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        this->delegate<delegate_type>().set_off_value(*this, value);
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        if (_pressed) {
            return theme::global->fillColor(this->_semantic_layer + 2);
        } else {
            return super::background_color();
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_normal(group) && enabled();
    }

    [[nodiscard]] bool handle_event(command command) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (enabled()) {
            switch (command) {
            case command::gui_activate: this->delegate<delegate_type>().pressed(*this, _button_type); return true;
            case command::gui_enter:
                this->delegate<delegate_type>().pressed(*this, _button_type);
                this->window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::forward);
                return true;
            default:;
            }
        }

        return super::handle_event(command);
    }

    [[nodiscard]] bool handle_event(mouse_event const &event) noexcept final
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = super::handle_event(event);

        if (event.cause.leftButton) {
            handled = true;
            if (enabled()) {
                if (compare_then_assign(_pressed, static_cast<bool>(event.down.leftButton))) {
                    request_redraw();
                }

                if (event.type == mouse_event::Type::ButtonUp && rectangle().contains(event.position)) {
                    handled |= handle_event(command::gui_activate);
                }
            }
        }
        return handled;
    }

    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept final
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (_visible_rectangle.contains(position)) {
            return hit_box{weak_from_this(), _draw_layer, enabled() ? hit_box::Type::Button : hit_box::Type::Default};
        } else {
            return hit_box{};
        }
    }

    /** Subscribe a callback to call when the button is activated.
     * @see button_delegate::subscribe_pressed()
     */
    template<typename Callback>
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return this->delegate<delegate_type>().subscribe_pressed(*this, std::forward<Callback>(callback));
    }

    /** Unsubscribe a callback.
     * @see button_delegate::unsubscribe_pressed()
     */
    void unsubscribe(callback_ptr_type &callback_ptr) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return this->delegate<delegate_type>().unsubscribe_pressed(*this, callback_ptr);
    }

protected:
    button_type _button_type = button_type::momentary;

    /** The button is in a pressed state.
     */
    bool _pressed = false;
};

} // namespace tt
