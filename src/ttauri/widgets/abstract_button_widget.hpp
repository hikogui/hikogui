// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "button_type.hpp"
#include "../animator.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename T, button_type ButtonType>
class abstract_button_widget : public widget {
public:
    static constexpr button_type button_type = ButtonType;

    using super = widget;
    using value_type = T;
    using delegate_type = typename button_delegate<value_type, button_type>;
    using callback_ptr_type = typename delegate_type::pressed_callback_ptr_type;

    abstract_button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>()) noexcept :
        super(window, std::move(parent), std::move(delegate))
    {
    }

    template<typename Value>
    abstract_button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate,
        Value &&value) noexcept :
        abstract_button_widget(window, std::move(parent), std::move(delegate))
    {
        this->set_value(std::forward<Value>(value));
    }

    template<typename Value>
    abstract_button_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        abstract_button_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    void init() noexcept override
    {
        _on_label_widget = this->make_widget<label_widget>(this->delegate_ptr<label_delegate>(), _label_alignment);
        _off_label_widget = this->make_widget<label_widget>(this->delegate_ptr<label_delegate>(), _label_alignment);
        _other_label_widget = this->make_widget<label_widget>(this->delegate_ptr<label_delegate>(), _label_alignment);

        _on_label_widget->id = "on_label";
        _off_label_widget->id = "off_label";
        _other_label_widget->id = "other_label";

        return super::init();
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

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            this->_minimum_size = _on_label_widget->minimum_size();
            this->_preferred_size = _on_label_widget->preferred_size();
            this->_maximum_size = _on_label_widget->maximum_size();

            this->_minimum_size = max(this->_minimum_size, _off_label_widget->minimum_size());
            this->_preferred_size = max(this->_preferred_size, _off_label_widget->preferred_size());
            this->_maximum_size = max(this->_maximum_size, _off_label_widget->maximum_size());

            this->_minimum_size = max(this->_minimum_size, _other_label_widget->minimum_size());
            this->_preferred_size = max(this->_preferred_size, _other_label_widget->preferred_size());
            this->_maximum_size = max(this->_maximum_size, _other_label_widget->maximum_size());

            tt_axiom(this->_minimum_size <= this->_preferred_size && this->_preferred_size <= this->_maximum_size);
            return true;
        } else {
            return false;
        }
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

    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept final
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (_visible_rectangle.contains(position)) {
            return hit_box{weak_from_this(), _draw_layer, enabled() ? hit_box::Type::Button : hit_box::Type::Default};
        } else {
            return hit_box{};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_normal(group) and enabled();
    }

    [[nodiscard]] bool handle_event(command command) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (enabled()) {
            switch (command) {
            case command::gui_activate: this->delegate<delegate_type>().pressed(*this); return true;
            case command::gui_enter:
                this->delegate<delegate_type>().pressed(*this);
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

protected:
    tt::alignment _label_alignment;
    std::shared_ptr<label_widget> _on_label_widget;
    std::shared_ptr<label_widget> _off_label_widget;
    std::shared_ptr<label_widget> _other_label_widget;

private:
    bool _pressed = false;

    void draw_label_button(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        // Move the border of the button in the middle of a pixel.
        context.draw_box_with_border_inside(
            this->rectangle(), this->background_color(), this->focus_color(), corner_shapes{theme::global->roundingRadius});
    }
};

} // namespace tt
