// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "../stencils/label_stencil.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename T>
class button_widget final : public abstract_button_widget<T> {
public:
    using super = abstract_button_widget<T>;
    using value_type = T;

    observable<label> label;

    template<typename Value = observable<value_type>>
    button_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        value_type true_value,
        Value &&value = {}) noexcept :
        super(window, parent, std::move(true_value), std::forward<Value>(value))
    {
    }

    void init() noexcept override
    {
        _label_callback = label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });

        _callback = this->subscribe([this](auto...) {
            this->clicked();
        });
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_stencil = stencil::make_unique(alignment::middle_center, *label, theme::global->labelStyle);
            this->_preferred_size = interval_extent2::make_minimum(_label_stencil->preferred_extent() + extent2{theme::global->margin2Dx2});
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
            _label_stencil->set_layout_parameters(this->rectangle(), this->base_line());
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        if (*this->value) {
            return this->accent_color();
        } else {
            return super::background_color();
        }
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            // Move the border of the button in the middle of a pixel.
            context.draw_box_with_border_inside(
                this->rectangle(), this->background_color(), this->focus_color(), corner_shapes{theme::global->roundingRadius});

            _label_stencil->draw(context, this->label_color(), translate_z(0.1f));
        }

        super::draw(std::move(context), display_time_point);
    }

    void clicked() noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        if (compare_then_assign(this->value, !this->value)) {
            this->window.request_redraw(aarect{this->_local_to_window * this->_clipping_rectangle});
        }
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;
    super::callback_ptr_type _callback;

    std::unique_ptr<label_stencil> _label_stencil;
};

} // namespace tt
