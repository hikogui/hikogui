// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "button_type.hpp"
#include "../animator.hpp"
#include "../l10n.hpp"
#include "../notifier.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class abstract_button_widget : public widget {
public:
    using super = widget;
    using delegate_type = button_delegate;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    observable<label> on_label = l10n("on");
    observable<label> off_label = l10n("off");
    observable<label> other_label = l10n("other");
    observable<alignment> label_alignment;

    abstract_button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>()) noexcept :
        super(window, std::move(parent)), _delegate(std::move(delegate))
    {
        _delegate->subscribe(*this, _relayout_callback);
    }

    void init() noexcept override
    {
        super::init();

        _on_label_widget = this->make_widget<label_widget>();
        _off_label_widget = this->make_widget<label_widget>();
        _other_label_widget = this->make_widget<label_widget>();

        _on_label_widget->alignment = label_alignment;
        _off_label_widget->alignment = label_alignment;
        _other_label_widget->alignment = label_alignment;

        _on_label_widget->label = on_label;
        _off_label_widget->label = off_label;
        _other_label_widget->label = other_label;

        _delegate->init(*this);
    }

    void deinit() noexcept override
    {
        _delegate->deinit(*this);
        super::deinit();
    }

    template<typename Label>
    void set_label(Label const &rhs) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        on_label = rhs;
        off_label = rhs;
        other_label = rhs;
    }

    [[nodiscard]] button_state state() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return _delegate->state(*this);
    }

    /** Subscribe a callback to call when the button is activated.
     */
    template<typename Callback>
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return _notifier.subscribe(std::forward<Callback>(callback));
    }

    /** Unsubscribe a callback.
     */
    void unsubscribe(callback_ptr_type &callback_ptr) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return _notifier.unsubscribe(callback_ptr);
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

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            auto state_ = state();
            this->_on_label_widget->visible = state_ == button_state::on;
            this->_off_label_widget->visible = state_ == button_state::off;
            this->_other_label_widget->visible = state_ == button_state::other;

            this->_on_label_widget->set_layout_parameters_from_parent(_label_rectangle);
            this->_off_label_widget->set_layout_parameters_from_parent(_label_rectangle);
            this->_other_label_widget->set_layout_parameters_from_parent(_label_rectangle);
        }
        widget::update_layout(displayTimePoint, need_layout);
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        if (_pressed) {
            return theme::global(theme_color::fill, this->_semantic_layer + 2);
        } else {
            return super::background_color();
        }
    }

    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept final
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (_visible_rectangle.contains(position)) {
            return hit_box{weak_from_this(), _draw_layer, enabled ? hit_box::Type::Button : hit_box::Type::Default};
        } else {
            return hit_box{};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_normal(group) and enabled;
    }

    void activate() noexcept
    {
        _delegate->activate(*this);

        run_from_main_loop([this]() {
            this->_notifier();
        });
    }

    [[nodiscard]] bool handle_event(command command) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (enabled) {
            switch (command) {
            case command::gui_activate: activate(); return true;
            case command::gui_enter:
                activate();
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
            if (enabled) {
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
    aarectangle _label_rectangle;
    std::shared_ptr<label_widget> _on_label_widget;
    std::shared_ptr<label_widget> _off_label_widget;
    std::shared_ptr<label_widget> _other_label_widget;

    bool _pressed = false;
    notifier<void()> _notifier;
    std::shared_ptr<delegate_type> _delegate;
};

} // namespace tt
