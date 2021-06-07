// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"

namespace tt {

template<typename T>
class toggle_widget final : public abstract_button_widget<T, button_type::toggle> {
public:
    using super = abstract_button_widget<T, button_type::toggle>;
    using value_type = typename super::value_type;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::pressed_callback_ptr_type;

    toggle_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>()) noexcept :
        super(window, std::move(parent), std::move(delegate))
    {
        this->_label_alignment = alignment::top_left;
    }

    template<typename Value>
    toggle_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate,
        Value &&value) noexcept :
        toggle_widget(window, std::move(parent), std::move(delegate))
    {
        this->set_value(std::forward<Value>(value));
    }

    template<typename Value>
    toggle_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        toggle_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            // Make room for button and margin.
            this->_button_size = {theme::global->smallSize * 2.0f, theme::global->smallSize};
            ttlet extra_size = extent2{theme::global->margin + this->_button_size.width(), 0.0f};
            this->_minimum_size += extra_size;
            this->_preferred_size += extra_size;
            this->_maximum_size += extra_size;

            // Make sure the widget is at least smallSize.
            this->_minimum_size = max(this->_minimum_size, this->_button_size);
            this->_preferred_size = max(this->_minimum_size, this->_button_size);
            this->_maximum_size = max(this->_minimum_size, this->_button_size);

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
            this->_button_rectangle = align(this->rectangle(), this->_button_size, alignment::top_left);

            ttlet label_rectangle =
                aarectangle{this->_button_rectangle.right() + theme::global->margin, 0.0f, this->width(), this->height()};

            this->_on_label_widget->set_layout_parameters_from_parent(label_rectangle);
            this->_off_label_widget->set_layout_parameters_from_parent(label_rectangle);
            this->_other_label_widget->set_layout_parameters_from_parent(label_rectangle);

            _pip_rectangle = aarectangle{
                this->_button_rectangle.left() + 2.5f,
                this->_button_rectangle.bottom() + 2.5f,
                theme::global->smallSize - 5.0f,
                theme::global->smallSize - 5.0f};
            _pip_move_range = this->_button_rectangle.width() - _pip_rectangle.width() - 5.0f;
        }
        widget::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            draw_toggle_button(context);
            draw_toggle_pip(context, display_time_point);
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    static constexpr hires_utc_clock::duration _animation_duration = 150ms;

    extent2 _button_size;
    aarectangle _button_rectangle;
    animator<float> _animated_value = _animation_duration;
    aarectangle _pip_rectangle;
    float _pip_move_range;

    void draw_toggle_button(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(
            this->_button_rectangle,
            this->background_color(),
            this->focus_color(),
            corner_shapes{this->_button_rectangle.height() * 0.5f});
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

} // namespace tt
