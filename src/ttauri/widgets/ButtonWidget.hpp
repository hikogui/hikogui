// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_button_widget.hpp"
#include "../cells/TextCell.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class ButtonWidget final : public abstract_button_widget {
public:
    observable<std::u8string> label;

    ButtonWidget(Window &window, Widget *parent) noexcept : abstract_button_widget(window, parent)
    {
        _label_callback = scoped_callback(label, [this](auto...) {
            request_reconstrain = true;
        });

        _button_callback = scoped_callback(*this, [this](auto...) {
            this->clicked();
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (abstract_button_widget::update_constraints()) {
            labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
            p_preferred_size = interval_vec2::make_minimum(labelCell->preferredExtent() + Theme::margin2Dx2);
            return true;
        } else {
            return false;
        }
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        context.cornerShapes = vec{Theme::roundingRadius};
        if (value) {
            context.fillColor = theme->accentColor;
        }

        // Move the border of the button in the middle of a pixel.
        context.drawBoxIncludeBorder(rectangle());

        if (*enabled) {
            context.color = theme->foregroundColor;
        }
        context.transform = mat::T{0.0f, 0.0f, 0.1f} * context.transform;
        labelCell->draw(context, rectangle(), Alignment::MiddleCenter, base_line(), true);

        abstract_button_widget::draw(std::move(context), display_time_point);
    }

    void clicked() noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        if (compare_then_assign(value, !value)) {
            window.requestRedraw = true;
        }
    }

private:
    bool value = false;
    bool pressed = false;
    
    scoped_callback<decltype(label)> _label_callback;
    scoped_callback<ButtonWidget> _button_callback;

    std::unique_ptr<TextCell> labelCell;
};

} // namespace tt
