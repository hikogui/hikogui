// Copyright 2019, 2020 Pokitec
// All rights reserved.

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
    button_widget(gui_window &window, std::shared_ptr<widget> parent, value_type true_value, Value &&value = {}) noexcept :
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
        tt_assume(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_stencil = stencil::make_unique(alignment::middle_center, *label, theme::global->labelStyle);
            this->_preferred_size = interval_vec2::make_minimum(_label_stencil->preferred_extent() + theme::global->margin2Dx2);
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            _label_stencil->set_layout_parameters(this->rectangle(), this->base_line());
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->window_clipping_rectangle())) {
            context.corner_shapes = f32x4{theme::global->roundingRadius};
            if (*this->value) {
                context.fill_color = theme::global->accentColor;
            }

            // Move the border of the button in the middle of a pixel.
            context.draw_box_with_border_inside(this->rectangle());

            if (*this->enabled) {
                context.color = theme::global->foregroundColor;
            }
            context.transform = mat::T{0.0f, 0.0f, 0.1f} * context.transform;
            _label_stencil->draw(context, true);
        }

        super::draw(std::move(context), display_time_point);
    }

    void clicked() noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        if (compare_then_assign(this->value, !this->value)) {
            this->window.request_redraw(this->window_clipping_rectangle());
        }
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;
    super::callback_ptr_type _callback;

    std::unique_ptr<label_stencil> _label_stencil;
};

} // namespace tt
