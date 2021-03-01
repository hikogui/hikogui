// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_radio_button_widget.hpp"
#include "../GUI/draw_context.hpp"
#include "../observable.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename T>
class radio_button_widget final : public abstract_radio_button_widget<T> {
public:
    using super = abstract_radio_button_widget<T>;
    using value_type = typename super::value_type;

    observable<label> label;

    template<typename Value, typename Label>
    radio_button_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        value_type true_value,
        Value &&value,
        Label &&label) noexcept :
        abstract_radio_button_widget<T>(window, parent, std::move(true_value), std::forward<Value>(value)),
        label(std::forward<Label>(label))
    {
    }

    template<typename Value>
    radio_button_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        value_type true_value,
        Value &&value) noexcept :
        radio_button_widget(window, parent, std::move(true_value), std::forward<Value>(value), observable<tt::label>{})
    {
    }

    radio_button_widget(gui_window &window, std::shared_ptr<abstract_container_widget> parent, value_type true_value) noexcept :
        radio_button_widget(window, parent, std::move(true_value), observable<value_type>{}, observable<tt::label>{})
    {
    }

    ~radio_button_widget() {}

    void init() noexcept override
    {
        label_callback = label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_stencil = stencil::make_unique(alignment::top_left, *label, theme::global->labelStyle);

            ttlet minimum_height = std::max(_label_stencil->preferred_extent().height(), theme::global->smallSize);
            ttlet minimum_width = theme::global->smallSize + theme::global->margin + _label_stencil->preferred_extent().width();

            super::_preferred_size = interval_extent2::make_minimum(minimum_width, minimum_height);
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            _outline_rectangle = aarect{
                0.0f, std::round(this->base_line() - theme::global->smallSize * 0.5f), theme::global->smallSize, theme::global->smallSize};

            ttlet labelX = _outline_rectangle.p3().x() + theme::global->margin;
            _label_rectangle = aarect{labelX, 0.0f, this->rectangle().width() - labelX, this->rectangle().height()};
            _label_stencil->set_layout_parameters(_label_rectangle, this->base_line());

            _pip_rectangle = shrink(_outline_rectangle, 2.5f);
        }
        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            draw_outline(context);
            draw_pip(context);
            draw_label(context);
        }
        super::draw(std::move(context), display_time_point);
    }

private:
    aarect _outline_rectangle;
    aarect _pip_rectangle;
    aarect _label_rectangle;
    std::unique_ptr<stencil> _label_stencil;

    typename decltype(label)::callback_ptr_type label_callback;

    void draw_outline(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(
            _outline_rectangle, this->background_color(), this->focus_color(), corner_shapes{_outline_rectangle.height() * 0.5f});
    }

    void draw_pip(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        // draw pip
        if (this->value == this->true_value) {
            context.draw_box(
                _pip_rectangle, this->accent_color(), corner_shapes{_pip_rectangle.height() * 0.5f});
        }
    }

    void draw_label(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        _label_stencil->draw(context, this->label_color());
    }
};

} // namespace tt
