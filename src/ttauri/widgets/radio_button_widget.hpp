// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_radio_button_widget.hpp"
#include "../GUI/DrawContext.hpp"
#include "../observable.hpp"
#include "../l10n_label.hpp"
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

    observable<l10n_label> label;

    template<typename Value, typename Label>
    radio_button_widget(
        Window &window,
        std::shared_ptr<widget> parent,
        value_type true_value,
        Value &&value,
        Label &&label) noexcept :
        abstract_radio_button_widget<T>(window, parent, std::move(true_value), std::forward<Value>(value)),
        label(std::forward<Label>(label))
    {
    }

    template<typename Value>
    radio_button_widget(Window &window, std::shared_ptr<widget> parent, value_type true_value, Value &&value) noexcept :
        radio_button_widget(window, parent, std::move(true_value), std::forward<Value>(value), l10n_label{})
    {
    }

    radio_button_widget(Window &window, std::shared_ptr<widget> parent, value_type true_value) noexcept :
        radio_button_widget(window, parent, std::move(true_value), observable<value_type>{}, l10n_label{})
    {
    }

    ~radio_button_widget() {}

    void initialize() noexcept override
    {
        label_callback = label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (widget::update_constraints()) {
            _label_stencil = (*label).make_stencil(Alignment::TopLeft, theme->labelStyle);

            ttlet minimum_height = std::max(_label_stencil->preferred_extent().height(), Theme::smallSize);
            ttlet minimum_width = Theme::smallSize + Theme::margin + _label_stencil->preferred_extent().width();

            super::_preferred_size = interval_vec2::make_minimum(minimum_width, minimum_height);
            super::_preferred_base_line = relative_base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            _outline_rectangle = aarect{0.0f, this->base_line() - Theme::smallSize * 0.5f, Theme::smallSize, Theme::smallSize};

            ttlet labelX = _outline_rectangle.p3().x() + Theme::margin;
            _label_rectangle = aarect{labelX, 0.0f, this->rectangle().width() - labelX, this->rectangle().height()};
            _label_stencil->set_layout_parameters(_label_rectangle, this->base_line());

            _pip_rectangle = shrink(_outline_rectangle, 1.5f);
        }
        return widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        draw_outline(context);
        draw_pip(context);
        draw_label(context);
        widget::draw(std::move(context), display_time_point);
    }

private:
    aarect _outline_rectangle;
    aarect _pip_rectangle;
    aarect _label_rectangle;
    std::unique_ptr<stencil> _label_stencil;

    typename decltype(label)::callback_ptr_type label_callback;

    void draw_outline(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        context.cornerShapes = vec{_outline_rectangle.height() * 0.5f};
        context.drawBoxIncludeBorder(_outline_rectangle);
    }

    void draw_pip(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        // draw pip
        if (this->value == this->true_value) {
            if (*this->enabled && this->window.active) {
                context.color = theme->accentColor;
            }
            std::swap(context.color, context.fillColor);
            context.cornerShapes = vec{_pip_rectangle.height() * 0.5f};
            context.drawBoxIncludeBorder(_pip_rectangle);
        }
    }

    void draw_label(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (*this->enabled) {
            context.color = theme->labelStyle.color;
        }

        _label_stencil->draw(context, true);
    }
};

} // namespace tt
