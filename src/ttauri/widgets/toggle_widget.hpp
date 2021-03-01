// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_bool_toggle_button_widget.hpp"
#include "../label.hpp"
#include "../stencils/label_stencil.hpp"
#include "../GUI/draw_context.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class toggle_widget final : public abstract_bool_toggle_button_widget {
public:
    using super = abstract_bool_toggle_button_widget;

    observable<label> on_label;
    observable<label> off_label;

    template<typename Value = observable<bool>>
    toggle_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        Value &&value = observable<bool>{}) noexcept :
        super(window, parent, std::forward<Value>(value))
    {
        _on_label_callback = this->on_label.subscribe([this](auto...) {
            _request_reconstrain = true;
        });
        _off_label_callback = this->off_label.subscribe([this](auto...) {
            _request_reconstrain = true;
        });
    }

    ~toggle_widget() {}

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _on_label_stencil = stencil::make_unique(alignment::top_left, *on_label, theme::global->labelStyle);
            _off_label_stencil = stencil::make_unique(alignment::top_left, *off_label, theme::global->labelStyle);

            ttlet minimumHeight = std::max(
                {_on_label_stencil->preferred_extent().height(),
                 _off_label_stencil->preferred_extent().height(),
                 theme::global->smallSize});

            ttlet minimumWidth =
                std::max({_on_label_stencil->preferred_extent().width(), _off_label_stencil->preferred_extent().width()}) +
                theme::global->smallSize * 2.0f + theme::global->margin;

            _preferred_size = interval_extent2::make_minimum(minimumWidth, minimumHeight);

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
            _rail_rectangle = aarect{
                -0.5f, // Expand horizontally due to rounded shape
                std::round(base_line() - theme::global->smallSize * 0.5f),
                theme::global->smallSize * 2.0f + 1.0f, // Expand horizontally due to rounded shape
                theme::global->smallSize};

            ttlet labelX = theme::global->smallSize * 2.0f + theme::global->margin;
            _label_rectangle = aarect{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};
            _on_label_stencil->set_layout_parameters(_label_rectangle, base_line());
            _off_label_stencil->set_layout_parameters(_label_rectangle, base_line());

            _slider_rectangle =
                shrink(aarect{0.0f, _rail_rectangle.y(), _rail_rectangle.height(), _rail_rectangle.height()}, 2.5f);

            ttlet sliderMoveWidth = theme::global->smallSize * 2.0f - (_slider_rectangle.x() * 2.0f);
            _slider_move_range = sliderMoveWidth - _slider_rectangle.width();
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, _clipping_rectangle)) {
            draw_rail(context);
            draw_slider(context);
            draw_label(context);
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    static constexpr hires_utc_clock::duration _animation_duration = 150ms;

    aarect _rail_rectangle;

    aarect _slider_rectangle;
    float _slider_move_range;

    aarect _label_rectangle;

    std::unique_ptr<label_stencil> _on_label_stencil;
    std::unique_ptr<label_stencil> _off_label_stencil;

    decltype(value)::callback_ptr_type _value_callback;
    decltype(on_label)::callback_ptr_type _on_label_callback;
    decltype(off_label)::callback_ptr_type _off_label_callback;

    void draw_rail(draw_context draw_context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        draw_context.draw_box_with_border_inside(
            _rail_rectangle, background_color(), focus_color(), corner_shapes{_rail_rectangle.height() * 0.5f});
    }

    void draw_slider(draw_context draw_context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        // Prepare animation values.
        ttlet animationProgress = value.animation_progress(_animation_duration);
        if (animationProgress < 1.0f) {
            window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
        }

        ttlet animatedValue = to_float(value, _animation_duration);
        ttlet positionedSliderRectangle = translate3{_slider_move_range * animatedValue, 0.0f, 0.1f} * _slider_rectangle;

        draw_context.draw_box(
            positionedSliderRectangle, accent_color(), corner_shapes{positionedSliderRectangle.height() * 0.5f});
    }

    void draw_label(draw_context draw_context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet &label_stencil = *value ? _on_label_stencil : _off_label_stencil;
        label_stencil->draw(draw_context, label_color());
    }
};

} // namespace tt
