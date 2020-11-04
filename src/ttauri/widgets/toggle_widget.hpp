// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_bool_toggle_button_widget.hpp"
#include "../l10n_label.hpp"
#include "../stencils/label_stencil.hpp"
#include "../GUI/DrawContext.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class toggle_widget final : public abstract_bool_toggle_button_widget {
public:
    observable<l10n_label> on_label;
    observable<l10n_label> off_label;

    template<typename Value = observable<bool>>
    toggle_widget(
        Window &window, std::shared_ptr<widget> parent,
        Value &&value = observable<bool>{}) noexcept :
        abstract_bool_toggle_button_widget(window, parent, std::forward<Value>(value))
    {
        _on_label_callback = this->on_label.subscribe([this](auto...) {
            _request_reconstrain = true;
        });
        _off_label_callback = this->off_label.subscribe([this](auto...) {
            _request_reconstrain = true;
        });
    }

    ~toggle_widget() {}

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (widget::update_constraints()) {
            _on_label_stencil = stencil::make_unique(Alignment::TopLeft, *on_label, theme->labelStyle);
            _off_label_stencil = stencil::make_unique(Alignment::TopLeft, *off_label, theme->labelStyle);

            ttlet minimumHeight =
                std::max({_on_label_stencil->preferred_extent().height(), _off_label_stencil->preferred_extent().height(), Theme::smallSize});

            ttlet minimumWidth = std::max({_on_label_stencil->preferred_extent().width(), _off_label_stencil->preferred_extent().width()}) +
                Theme::smallSize * 2.0f + Theme::margin;

            _preferred_size = interval_vec2::make_minimum(minimumWidth, minimumHeight);
            _preferred_base_line = relative_base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};

            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            _rail_rectangle = aarect{
                -0.5f, // Expand horizontally due to rounded shape
                base_line() - Theme::smallSize * 0.5f,
                Theme::smallSize * 2.0f + 1.0f, // Expand horizontally due to rounded shape
                Theme::smallSize};

            ttlet labelX = Theme::smallSize * 2.0f + Theme::margin;
            _label_rectangle = aarect{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};
            _on_label_stencil->set_layout_parameters(_label_rectangle, base_line());
            _off_label_stencil->set_layout_parameters(_label_rectangle, base_line());

            _slider_rectangle = shrink(aarect{0.0f, _rail_rectangle.y(), _rail_rectangle.height(), _rail_rectangle.height()}, 1.5f);

            ttlet sliderMoveWidth = Theme::smallSize * 2.0f - (_slider_rectangle.x() * 2.0f);
            _slider_move_range = sliderMoveWidth - _slider_rectangle.width();
        }

        return widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        draw_rail(context);
        draw_slider(context);
        draw_label(context);
        widget::draw(std::move(context), display_time_point);
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

    void draw_rail(DrawContext drawContext) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        drawContext.cornerShapes = vec{_rail_rectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(_rail_rectangle);
    }

    void draw_slider(DrawContext drawContext) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        // Prepare animation values.
        ttlet animationProgress = value.animation_progress(_animation_duration);
        if (animationProgress < 1.0f) {
            window.requestRedraw = true;
        }

        ttlet animatedValue = to_float(value, _animation_duration);

        ttlet positionedSliderRectangle = mat::T2(_slider_move_range * animatedValue, 0.0f) * _slider_rectangle;

        if (*value) {
            if (*enabled && window.active) {
                drawContext.color = theme->accentColor;
            }
        } else {
            if (*enabled && window.active) {
                drawContext.color = _hover ? theme->borderColor(_semantic_layer + 1) : theme->borderColor(_semantic_layer);
            }
        }
        std::swap(drawContext.color, drawContext.fillColor);
        drawContext.transform = mat::T{0.0f, 0.0f, 0.1f} * drawContext.transform;
        drawContext.cornerShapes = vec{positionedSliderRectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(positionedSliderRectangle);
    }

    void draw_label(DrawContext drawContext) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        ttlet &label_stencil = *value ? _on_label_stencil : _off_label_stencil;

        label_stencil->draw(drawContext, true);
    }
};

} // namespace tt
