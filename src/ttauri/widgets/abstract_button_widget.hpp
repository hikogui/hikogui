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
#include "../weak_or_unique_ptr.hpp"
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
        widget *parent,
        weak_or_unique_ptr<delegate_type> delegate) noexcept :
        super(window, parent), _delegate(std::move(delegate))
    {
        if (auto d = _delegate.lock()) {
            d->subscribe(*this, _relayout_callback);
        }
    }

    void init() noexcept override
    {
        super::init();

        _on_label_widget = &make_widget<label_widget>();
        _off_label_widget = &make_widget<label_widget>();
        _other_label_widget = &make_widget<label_widget>();

        _on_label_widget->alignment = label_alignment;
        _off_label_widget->alignment = label_alignment;
        _other_label_widget->alignment = label_alignment;

        _on_label_widget->label = on_label;
        _off_label_widget->label = off_label;
        _other_label_widget->label = other_label;

        if (auto delegate = _delegate.lock()) {
            delegate->init(*this);
        }
    }

    void deinit() noexcept override
    {
        if (auto delegate = _delegate.lock()) {
            delegate->deinit(*this);
        }
        super::deinit();
    }

    template<typename Label>
    void set_label(Label const &rhs) noexcept
    {
        tt_axiom(is_gui_thread());
        on_label = rhs;
        off_label = rhs;
        other_label = rhs;
    }

    [[nodiscard]] button_state state() const noexcept
    {
        tt_axiom(is_gui_thread());
        if (auto delegate = _delegate.lock()) {
            return delegate->state(*this);
        } else {
            return button_state::off;
        }
    }

    /** Subscribe a callback to call when the button is activated.
     */
    template<typename Callback>
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept
    {
        tt_axiom(is_gui_thread());
        return _notifier.subscribe(std::forward<Callback>(callback));
    }

    /** Unsubscribe a callback.
     */
    void unsubscribe(callback_ptr_type &callback_ptr) noexcept
    {
        tt_axiom(is_gui_thread());
        return _notifier.unsubscribe(callback_ptr);
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _minimum_size = _on_label_widget->minimum_size();
            _preferred_size = _on_label_widget->preferred_size();
            _maximum_size = _on_label_widget->maximum_size();

            _minimum_size = max(_minimum_size, _off_label_widget->minimum_size());
            _preferred_size = max(_preferred_size, _off_label_widget->preferred_size());
            _maximum_size = max(_maximum_size, _off_label_widget->maximum_size());

            _minimum_size = max(_minimum_size, _other_label_widget->minimum_size());
            _preferred_size = max(_preferred_size, _other_label_widget->preferred_size());
            _maximum_size = max(_maximum_size, _other_label_widget->maximum_size());

            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_axiom(is_gui_thread());

        need_layout |= _request_relayout.exchange(false);
        if (need_layout) {
            auto state_ = state();
            _on_label_widget->visible = state_ == button_state::on;
            _off_label_widget->visible = state_ == button_state::off;
            _other_label_widget->visible = state_ == button_state::other;

            _on_label_widget->set_layout_parameters_from_parent(_label_rectangle);
            _off_label_widget->set_layout_parameters_from_parent(_label_rectangle);
            _other_label_widget->set_layout_parameters_from_parent(_label_rectangle);
        }
        widget::update_layout(displayTimePoint, need_layout);
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        tt_axiom(is_gui_thread());
        if (_pressed) {
            return theme::global(theme_color::fill, semantic_layer + 2);
        } else {
            return super::background_color();
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept final
    {
        tt_axiom(is_gui_thread());

        if (_visible_rectangle.contains(position)) {
            return hitbox{this, draw_layer, enabled ? hitbox::Type::Button : hitbox::Type::Default};
        } else {
            return hitbox{};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        tt_axiom(is_gui_thread());
        return is_normal(group) and enabled;
    }

    void activate() noexcept;

    [[nodiscard]] bool handle_event(command command) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (enabled) {
            switch (command) {
            case command::gui_activate: activate(); return true;
            case command::gui_enter:
                activate();
                window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::forward);
                return true;
            default:;
            }
        }

        return super::handle_event(command);
    }

    [[nodiscard]] bool handle_event(mouse_event const &event) noexcept final
    {
        tt_axiom(is_gui_thread());
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
    label_widget *_on_label_widget = nullptr;
    label_widget *_off_label_widget = nullptr;
    label_widget *_other_label_widget = nullptr;

    bool _pressed = false;
    notifier<void()> _notifier;
    weak_or_unique_ptr<delegate_type> _delegate;
};

} // namespace tt
