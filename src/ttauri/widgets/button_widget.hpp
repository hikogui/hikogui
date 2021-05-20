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
    using delegate_type = typename super::delegate_type;
    using value_type = typename super::value_type;

    template<typename Value = observable<value_type>>
    button_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>(),
        Value &&value = value_type{}) noexcept :
        super(window, std::move(parent), std::move(delegate), std::forward<Value>(value))
    {
    }

    template<typename Value = observable<value_type>>
    button_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        Value &&value) noexcept :
        button_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_stencil = stencil::make_unique(alignment::middle_center, this->label(), theme::global->labelStyle);
            this->_minimum_size = _label_stencil->minimum_size() + theme::global->margin2Dx2;
            this->_preferred_size = _label_stencil->preferred_size() + theme::global->margin2Dx2;
            this->_maximum_size = _label_stencil->maximum_size() + theme::global->margin2Dx2;
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
            _label_stencil->set_layout_parameters(this->rectangle(), this->base_line());
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        if (this->state() == button_state::on) {
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

            tt_stencil_draw(_label_stencil, context, this->label_color(), translate_z(0.1f));
        }

        super::draw(std::move(context), display_time_point);
    }

    void clicked() noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        if (compare_then_assign(this->value, !this->value)) {
            this->request_redraw();
        }
    }

private:
    std::unique_ptr<label_stencil> _label_stencil;
};

} // namespace tt
