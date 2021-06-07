// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"

namespace tt {

template<typename T>
class toolbar_button_widget final : public abstract_button_widget<T, button_type::momentary> {
public:
    using super = abstract_button_widget<T, button_type::momentary>;
    using value_type = typename super::value_type;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::pressed_callback_ptr_type;

    toolbar_button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>()) noexcept :
        super(window, std::move(parent), std::move(delegate))
    {
        this->_margin = 0.0f;
        this->_label_alignment = alignment::middle_left;
    }

    template<typename Value>
    toolbar_button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate,
        Value &&value) noexcept :
        toolbar_button_widget(window, std::move(parent), std::move(delegate))
    {
        this->set_value(std::forward<Value>(value));
    }

    template<typename Value>
    toolbar_button_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        toolbar_button_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            // On left side a check mark, on right side short-cut. Around the label extra margin.
            ttlet extra_size = theme::global->margin2Dx2;
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
            ttlet label_rectangle =
                aarectangle{theme::global->margin, 0.0f, this->width() - theme::global->margin * 2.0f, this->height()};

            this->_on_label_widget->set_layout_parameters_from_parent(label_rectangle);
            this->_off_label_widget->set_layout_parameters_from_parent(label_rectangle);
            this->_other_label_widget->set_layout_parameters_from_parent(label_rectangle);
        }
        widget::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            draw_toolbar_button(context);
        }

        super::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_toolbar(group) and this->enabled();
    }

    [[nodiscard]] bool handle_event(command command) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (this->enabled()) {
            switch (command) {
            case command::gui_toolbar_next:
                if (!this->is_last(keyboard_focus_group::toolbar)) {
                    this->window.update_keyboard_target(keyboard_focus_group::toolbar, keyboard_focus_direction::forward);
                    return true;
                }
                break;

            case command::gui_toolbar_prev:
                if (!this->is_first(keyboard_focus_group::toolbar)) {
                    this->window.update_keyboard_target(keyboard_focus_group::toolbar, keyboard_focus_direction::backward);
                    return true;
                }
                break;

            default:;
            }
        }

        return super::handle_event(command);
    }

private:
    void draw_toolbar_button(draw_context const &context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet foreground_color_ = this->_focus && this->window.active ? this->focus_color() : color::transparent();
        context.draw_box_with_border_inside(this->rectangle(), this->background_color(), foreground_color_, corner_shapes{0.0f});
    }
};

} // namespace tt
