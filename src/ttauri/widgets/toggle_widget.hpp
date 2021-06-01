// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "../label.hpp"
#include "../stencils/label_stencil.hpp"
#include "../GUI/draw_context.hpp"
#include "../observable.hpp"
#include "../animator.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class toggle_widget final : public abstract_button_widget<bool,button_type::toggle> {
public:
    using super = abstract_button_widget<bool,button_type::toggle>;
    using value_type = typename super::value_type;
    using delegate_type = typename super::delegate_type;

    template<typename Value = value_type>
    toggle_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>(),
        Value &&value = value_type{}) noexcept :
        super(window, std::move(parent), std::move(delegate), std::forward<Value>(value))
    {
    }

    ~toggle_widget() {}

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_stencil = stencil::make_unique(alignment::top_left, this->label(), theme::global->labelStyle);

            ttlet minimum_label_size = _label_stencil->minimum_size();
            ttlet preferred_label_size = _label_stencil->preferred_size();
            ttlet maximum_label_size = _label_stencil->maximum_size();

            _minimum_size = {
                minimum_label_size.width() + theme::global->smallSize * 2.0f + theme::global->margin,
                std::max(minimum_label_size.height(), theme::global->smallSize)};
            _preferred_size = {
                preferred_label_size.width() + theme::global->smallSize * 2.0f + theme::global->margin,
                std::max(preferred_label_size.height(), theme::global->smallSize)};
            _maximum_size = {
                maximum_label_size.width() + theme::global->smallSize * 2.0f + theme::global->margin,
                std::max(maximum_label_size.height(), theme::global->smallSize)};

            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            _rail_rectangle = aarectangle{
                -0.5f, // Expand horizontally due to rounded shape
                std::round(base_line() - theme::global->smallSize * 0.5f),
                theme::global->smallSize * 2.0f + 1.0f, // Expand horizontally due to rounded shape
                theme::global->smallSize};

            ttlet labelX = theme::global->smallSize * 2.0f + theme::global->margin;
            _label_rectangle = aarectangle{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};
            _label_stencil->set_layout_parameters(_label_rectangle, base_line());

            _slider_rectangle =
                shrink(aarectangle{0.0f, _rail_rectangle.bottom(), _rail_rectangle.height(), _rail_rectangle.height()}, 2.5f);

            ttlet sliderMoveWidth = theme::global->smallSize * 2.0f - (_slider_rectangle.left() * 2.0f);
            _slider_move_range = sliderMoveWidth - _slider_rectangle.width();
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, _clipping_rectangle)) {
            draw_rail(context);
            draw_slider(context, display_time_point);
            draw_label(context);
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    static constexpr hires_utc_clock::duration _animation_duration = 150ms;

    aarectangle _rail_rectangle;

    aarectangle _slider_rectangle;
    float _slider_move_range;

    aarectangle _label_rectangle;

    std::unique_ptr<label_stencil> _label_stencil;

    animator<float> _animated_value = animator<float>(_animation_duration);

    void draw_rail(draw_context draw_context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        draw_context.draw_box_with_border_inside(
            _rail_rectangle, background_color(), focus_color(), corner_shapes{_rail_rectangle.height() * 0.5f});
    }

    void draw_slider(draw_context draw_context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        _animated_value.update(this->state() == button_state::on ? 1.0f : 0.0f, display_time_point);

        // Prepare animation values.
        if (_animated_value.is_animating()) {
            request_redraw();
        }

        ttlet positionedSliderRectangle =
            translate3{_slider_move_range * _animated_value.current_value(), 0.0f, 0.1f} * _slider_rectangle;

        draw_context.draw_box(
            positionedSliderRectangle, accent_color(), corner_shapes{positionedSliderRectangle.height() * 0.5f});
    }

    void draw_label(draw_context draw_context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_stencil_draw(_label_stencil, draw_context, label_color());
    }
};

} // namespace tt
