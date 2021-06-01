// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "button_shape.hpp"
#include "button_type.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename T, button_shape ButtonShape, button_type ButtonType>
class button_widget final : public widget {
public:
    static constexpr button_shape button_shape = ButtonShape;
    static constexpr button_type button_type = ButtonType;

    using super = widget;
    using value_type = T;
    using delegate_type = typename button_delegate<value_type, button_type>;
    using callback_ptr_type = typename delegate_type::pressed_callback_ptr_type;

    button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>()) noexcept :
        super(window, std::move(parent), std::move(delegate))
    {
    }

    template<typename Value>
    button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate,
        Value &&value) noexcept :
        button_widget(window, std::move(parent), std::move(delegate))
    {
        set_value(std::forward<Value>(value));
    }

    template<typename Value>
    button_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        button_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    void init() noexcept override
    {
        ttlet alignment = button_shape == button_shape::label ? tt::alignment::middle_center : tt::alignment::top_left;

        _on_label_widget = make_widget<label_widget>(delegate_ptr<label_delegate>(), alignment);
        _off_label_widget = make_widget<label_widget>(delegate_ptr<label_delegate>(), alignment);
        _other_label_widget = make_widget<label_widget>(delegate_ptr<label_delegate>(), alignment);

        _on_label_widget->id = "on";
        _off_label_widget->id = "off";
        _other_label_widget->id = "other";
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            auto extra_size = extent2{};
            switch (button_shape) {
            case button_shape::label: extra_size = theme::global->margin2Dx2; break;
            case button_shape::checkbox: [[fallthrough]];
            case button_shape::radio:
                extra_size = extent2{theme::global->smallSize + theme::global->margin, theme::global->smallSize};
                break;
            case button_shape::toggle:
                extra_size = extent2{theme::global->smallSize * 2.0f + theme::global->margin, theme::global->smallSize};
                break;
            default: tt_no_default();
            }

            this->_minimum_size = _on_label_widget->minimum_size();
            this->_preferred_size = _on_label_widget->preferred_size();
            this->_maximum_size = _on_label_widget->maximum_size();

            this->_minimum_size = max(this->_minimum_size, _off_label_widget->minimum_size());
            this->_preferred_size = max(this->_preferred_size, _off_label_widget->preferred_size());
            this->_maximum_size = max(this->_maximum_size, _off_label_widget->maximum_size());

            this->_minimum_size = max(this->_minimum_size, _other_label_widget->minimum_size());
            this->_preferred_size = max(this->_preferred_size, _other_label_widget->preferred_size());
            this->_maximum_size = max(this->_maximum_size, _other_label_widget->maximum_size());

            this->_minimum_size += extra_size;
            this->_preferred_size += extra_size;
            this->_maximum_size += extra_size;
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
            ttlet inner_margin = theme::global->margin;

            auto button_size = extent2{};
            switch (button_shape) {
            case button_shape::label: button_size = size(); break;
            case button_shape::checkbox: [[fallthrough]];
            case button_shape::radio: button_size = extent2{theme::global->smallSize, theme::global->smallSize}; break;
            case button_shape::toggle: button_size = extent2{theme::global->smallSize * 2.0f, theme::global->smallSize}; break;
            default: tt_no_default();
            }

            auto label_rect = button_shape == button_shape::label ?
                aarectangle{inner_margin, inner_margin, width() - inner_margin * 2, height() - inner_margin * 2} :
                aarectangle{button_size.width() + inner_margin, 0.0f, width() - button_size.width() - inner_margin, height()};

            _on_label_widget->set_layout_parameters_from_parent(label_rect);
            _off_label_widget->set_layout_parameters_from_parent(label_rect);
            _other_label_widget->set_layout_parameters_from_parent(label_rect);

            _button_rect = aarectangle{point2{0.0f, height() - button_size.height()}, button_size};
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    //[[nodiscard]] color background_color() const noexcept override
    //{
    //    if (this->state() == button_state::on) {
    //        return this->accent_color();
    //    } else {
    //        return super::background_color();
    //    }
    //}

    [[nodiscard]] color background_color() const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        if (_pressed) {
            return theme::global->fillColor(this->_semantic_layer + 2);
        } else {
            return super::background_color();
        }
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            switch (button_shape) {
            case button_shape::label: draw_label_button(context, display_time_point); break;
            default: tt_no_default();
            }
        }

        super::draw(std::move(context), display_time_point);
    }

    void clicked() noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        delegate<button_delegate>().pressed(*this);
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

private:
    std::shared_ptr<label_widget> _on_label_widget;
    std::shared_ptr<label_widget> _off_label_widget;
    std::shared_ptr<label_widget> _other_label_widget;
    aarectangle _button_rect;
    bool _pressed;

    void draw_label_button(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
    {
        // Move the border of the button in the middle of a pixel.
        context.draw_box_with_border_inside(
            _button_rect, this->background_color(), this->focus_color(), corner_shapes{theme::global->roundingRadius});
    }
};

template<typename T>
using label_button_widget = button_widget<T, button_shape::label, button_type::momentary>;

} // namespace tt
