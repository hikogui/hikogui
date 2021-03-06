// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"

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

    value_type const true_value;
    observable<value_type> value;

    using notifier_type = notifier<void()>;
    using callback_type = typename notifier_type::callback_type;
    using callback_ptr_type = typename notifier_type::callback_ptr_type;

    template<typename Value = observable<value_type>>
    [[nodiscard]] abstract_button_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        value_type true_value,
        Value &&value = {}) :
        super(window, parent), true_value(std::move(true_value)), value(std::forward<Value>(value))
    {
    }

    color background_color() const noexcept override
    {
        if (_pressed) {
            return theme::global->fillColor(this->_semantic_layer + 2);
        } else {
            return super::background_color();
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_normal(group) && *enabled;
    }

    [[nodiscard]] bool handle_event(command command) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (*enabled) {
            switch (command) {
            case command::gui_activate: this->_notifier(); return true;
            case command::gui_enter:
                this->_notifier();
                this->window.update_keyboard_target(
                    this->shared_from_this(), keyboard_focus_group::normal, keyboard_focus_direction::forward);
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
            if (*enabled) {
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
            return hit_box{weak_from_this(), _draw_layer, *enabled ? hit_box::Type::Button : hit_box::Type::Default};
        } else {
            return hit_box{};
        }
    }

    /** Subscribe a callback to call when the button is activated.
     * @see notifier::subscribe()
     */
    template<typename Callback>
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept
    {
        return _notifier.subscribe(std::forward<Callback>(callback));
    }

    /** Unsubscribe a callback.
     * @see notifier::subscribe()
     */
    void unsubscribe(callback_ptr_type &callback_ptr) noexcept
    {
        return _notifier.unsubscribe(callback_ptr);
    }

protected:
    /** The button is in a pressed state.
     */
    bool _pressed = false;

private:
    notifier_type _notifier;
};

} // namespace tt
