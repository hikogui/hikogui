// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "button_shape.hpp"
#include "button_type.hpp"
#include "../animator.hpp"
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
        if (button_shape == button_shape::toolbar || button_shape == button_shape::menu) {
            this->_margin = 0.0f;
        }
    }

    template<typename Value>
    button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate,
        Value &&value) noexcept :
        button_widget(window, std::move(parent), std::move(delegate))
    {
        this->set_value(std::forward<Value>(value));
    }

    template<typename Value>
    button_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        button_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    void init() noexcept override
    {
        auto alignment = tt::alignment{};
        switch (button_shape) {
        case button_shape::toolbar: [[fallthrough]];
        case button_shape::menu: alignment = alignment::middle_left; break;
        case button_shape::label: alignment = alignment::middle_center; break;
        case button_shape::radio: [[fallthrough]];
        case button_shape::checkbox: [[fallthrough]];
        case button_shape::toggle: alignment = alignment::top_left; break;
        default: tt_no_default();
        }

        _on_label_widget = this->make_widget<label_widget>(this->delegate_ptr<label_delegate>(), alignment);
        _off_label_widget = this->make_widget<label_widget>(this->delegate_ptr<label_delegate>(), alignment);
        _other_label_widget = this->make_widget<label_widget>(this->delegate_ptr<label_delegate>(), alignment);

        _on_label_widget->id = "on_label";
        _off_label_widget->id = "off_label";
        _other_label_widget->id = "other_label";
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
            switch (button_shape) {
            case button_shape::toolbar: [[fallthrough]];
            case button_shape::label:
                _button_size = {};
                _short_cut_size = {};
                break;
            case button_shape::checkbox: [[fallthrough]];
            case button_shape::radio:
                _button_size = {theme::global->smallSize, theme::global->smallSize};
                _short_cut_size = {};
                break;
            case button_shape::toggle:
                _button_size = {theme::global->smallSize * 2.0f, theme::global->smallSize};
                _short_cut_size = {};
                break;
            case button_shape::menu:
                _button_size = {theme::global->smallSize, theme::global->smallSize};
                _short_cut_size = {theme::global->smallSize, theme::global->smallSize};
                break;
            }

            auto extra_size = extent2{};
            switch (button_shape) {
            case button_shape::toolbar: [[fallthrough]];
            case button_shape::label:
                // Around the label extra margin.
                extra_size = theme::global->margin2Dx2;
                break;
            case button_shape::checkbox: [[fallthrough]];
            case button_shape::radio: [[fallthrough]];
            case button_shape::toggle:
                // On left side a radio or checkbox button.
                extra_size = extent2{theme::global->margin + _button_size.width(), 0.0f};
                break;
            case button_shape::menu:
                // On left side a check mark, on right side short-cut. Around the label extra margin.
                extra_size = extent2{theme::global->margin * 2.0f + _button_size.width() + _short_cut_size.width(), 0.0f} +
                    theme::global->margin2Dx2;
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

            // Make room for button and margin.
            this->_minimum_size += extra_size;
            this->_preferred_size += extra_size;
            this->_maximum_size += extra_size;

            // Make sure the widget is at least smallSize.
            this->_minimum_size = max(this->_minimum_size, theme::global->smallSize2D);
            this->_preferred_size = max(this->_minimum_size, theme::global->smallSize2D);
            this->_maximum_size = max(this->_minimum_size, theme::global->smallSize2D);

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

            _button_rectangle = button_shape == button_shape::menu ?
                align(this->rectangle(), _button_size, alignment::middle_left) :
                align(this->rectangle(), _button_size, alignment::top_left);

            _short_cut_rectangle = align(this->rectangle(), _short_cut_size, alignment::middle_right);

            ttlet label_p0 = _button_rectangle ? point2{_button_rectangle.right() + inner_margin, 0.0f} : point2{0.0f, 0.0f};
            ttlet label_p3 =
                _short_cut_rectangle ? point2{_short_cut_rectangle.left() - inner_margin, height()} : point2{width(), height()};

            ttlet label_rectangle = aarectangle{label_p0, label_p3};

            _on_label_widget->set_layout_parameters_from_parent(label_rectangle);
            _off_label_widget->set_layout_parameters_from_parent(label_rectangle);
            _other_label_widget->set_layout_parameters_from_parent(label_rectangle);

            _check_glyph = to_font_glyph_ids(elusive_icon::Ok);
            ttlet check_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_check_glyph);
            _check_glyph_rectangle =
                align(_button_rectangle, scale(check_glyph_bb, theme::global->small_icon_size), alignment::middle_center);

            _minus_glyph = to_font_glyph_ids(elusive_icon::Minus);
            ttlet minus_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_minus_glyph);
            _minus_glyph_rectangle =
                align(_button_rectangle, scale(minus_glyph_bb, theme::global->small_icon_size), alignment::middle_center);

            _pip_rectangle = aarectangle{
                _button_rectangle.left() + 2.5f,
                _button_rectangle.bottom() + 2.5f,
                theme::global->smallSize - 5.0f,
                theme::global->smallSize - 5.0f};
            _pip_move_range = _button_rectangle.width() - _pip_rectangle.width() - 5.0f;
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            switch (button_shape) {
            case button_shape::toolbar: draw_toolbar_button(context); break;
            case button_shape::label: draw_label_button(context); break;
            case button_shape::menu:
                draw_toolbar_button(context);
                draw_check_mark(context, false);
                break;
            case button_shape::checkbox:
                draw_check_box(context);
                draw_check_mark(context, true);
                break;
            case button_shape::radio:
                draw_radio_button(context);
                draw_radio_pip(context, display_time_point);
                break;
            case button_shape::toggle:
                draw_toggle_button(context);
                draw_toggle_pip(context, display_time_point);
                break;
            default: tt_no_default();
            }
        }

        super::draw(std::move(context), display_time_point);
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

    //[[nodiscard]] color background_color() const noexcept override
    //{
    //    if (this->state() == button_state::on) {
    //        return this->accent_color();
    //    } else {
    //        return super::background_color();
    //    }
    //}

    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept final
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (_visible_rectangle.contains(position)) {
            return hit_box{weak_from_this(), _draw_layer, enabled() ? hit_box::Type::Button : hit_box::Type::Default};
        } else {
            return hit_box{};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        switch (button_shape) {
        case button_shape::menu: return is_menu(group) and this->enabled();
        case button_shape::toolbar: return is_toolbar(group) and this->enabled();
        default: return is_normal(group) and enabled();
        }
    }

    [[nodiscard]] bool handle_event(command command) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (enabled()) {
            switch (command) {
            case command::gui_menu_next:
                if (button_shape == button_shape::menu and !this->is_last(keyboard_focus_group::menu)) {
                    this->window.update_keyboard_target(keyboard_focus_group::menu, keyboard_focus_direction::forward);
                    return true;
                }
                break;

            case command::gui_menu_prev:
                if (button_shape == button_shape::menu and !this->is_first(keyboard_focus_group::menu)) {
                    this->window.update_keyboard_target(keyboard_focus_group::menu, keyboard_focus_direction::backward);
                    return true;
                }
                break;

            case command::gui_toolbar_next:
                if (button_shape == button_shape::toolbar and !this->is_last(keyboard_focus_group::toolbar)) {
                    this->window.update_keyboard_target(keyboard_focus_group::toolbar, keyboard_focus_direction::forward);
                    return true;
                }
                break;

            case command::gui_toolbar_prev:
                if (button_shape == button_shape::toolbar and !this->is_first(keyboard_focus_group::toolbar)) {
                    this->window.update_keyboard_target(keyboard_focus_group::toolbar, keyboard_focus_direction::backward);
                    return true;
                }
                break;
            case command::gui_activate:
                if (button_shape == button_shape::menu) {
                    this->delegate<delegate_type>().pressed(*this);
                    this->window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::forward);
                    this->window.update_keyboard_target(keyboard_focus_group::normal, keyboard_focus_direction::backward);
                } else {
                    this->delegate<delegate_type>().pressed(*this);
                }
                return true;
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

private:
    static constexpr hires_utc_clock::duration _animation_duration = 150ms;

    std::shared_ptr<label_widget> _on_label_widget;
    std::shared_ptr<label_widget> _off_label_widget;
    std::shared_ptr<label_widget> _other_label_widget;
    extent2 _button_size;
    aarectangle _button_rectangle;
    extent2 _short_cut_size;
    aarectangle _short_cut_rectangle;
    bool _pressed = false;

    font_glyph_ids _check_glyph;
    aarectangle _check_glyph_rectangle;

    font_glyph_ids _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    aarectangle _pip_rectangle;
    float _pip_move_range;

    animator<float> _animated_value = _animation_duration;

    void draw_toolbar_button(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet foreground_color_ = this->_focus && this->window.active ? this->focus_color() : color::transparent();
        context.draw_box_with_border_inside(this->rectangle(), this->background_color(), foreground_color_, corner_shapes{0.0f});
    }

    void draw_label_button(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        // Move the border of the button in the middle of a pixel.
        context.draw_box_with_border_inside(
            this->rectangle(), this->background_color(), this->focus_color(), corner_shapes{theme::global->roundingRadius});
    }

    void draw_check_box(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(_button_rectangle, this->background_color(), this->focus_color());
    }

    void draw_check_mark(draw_context context, bool show_minus) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto state_ = this->state();

        // Checkmark or tristate.
        if (state_ == tt::button_state::on) {
            context.draw_glyph(_check_glyph, translate_z(0.1f) * _check_glyph_rectangle, this->accent_color());

        } else if (state_ == tt::button_state::off) {
            ;

        } else if (show_minus) {
            context.draw_glyph(_minus_glyph, translate_z(0.1f) * _minus_glyph_rectangle, this->accent_color());
        }
    }

    void draw_radio_button(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(
            _button_rectangle, this->background_color(), this->focus_color(), corner_shapes{_button_rectangle.height() * 0.5f});
    }

    void draw_radio_pip(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        _animated_value.update(this->state() == button_state::on ? 1.0f : 0.0f, display_time_point);
        if (_animated_value.is_animating()) {
            this->request_redraw();
        }

        // draw pip
        auto float_value = _animated_value.current_value();
        if (float_value > 0.0) {
            ttlet scaled_pip_rectangle = scale(_pip_rectangle, float_value);
            context.draw_box(scaled_pip_rectangle, this->accent_color(), corner_shapes{scaled_pip_rectangle.height() * 0.5f});
        }
    }

    void draw_toggle_button(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(
            _button_rectangle, this->background_color(), this->focus_color(), corner_shapes{_button_rectangle.height() * 0.5f});
    }

    void draw_toggle_pip(draw_context draw_context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        _animated_value.update(this->state() == button_state::on ? 1.0f : 0.0f, display_time_point);
        if (_animated_value.is_animating()) {
            this->request_redraw();
        }

        ttlet positioned_pip_rectangle =
            translate3{_pip_move_range * _animated_value.current_value(), 0.0f, 0.1f} * _pip_rectangle;

        ttlet forground_color_ = this->state() == button_state::on ? this->accent_color() : this->foreground_color();
        draw_context.draw_box(
            positioned_pip_rectangle, forground_color_, corner_shapes{positioned_pip_rectangle.height() * 0.5f});
    }
};

template<typename T>
using label_button_widget = button_widget<T, button_shape::label, button_type::momentary>;

template<typename T>
using checkbox_widget = button_widget<T, button_shape::checkbox, button_type::toggle>;

template<typename T>
using radio_button_widget = button_widget<T, button_shape::radio, button_type::radio>;

template<typename T>
using toggle_widget = button_widget<T, button_shape::toggle, button_type::toggle>;

template<typename T>
using toolbar_button_widget = button_widget<T, button_shape::toolbar, button_type::momentary>;

template<typename T>
using menu_button_widget = button_widget<T, button_shape::menu, button_type::momentary>;

} // namespace tt
