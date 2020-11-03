// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_button_widget.hpp"
#include "../stencils/text_stencil.hpp"
#include "../l10n_label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;

    observable<l10n_label> label;

    button_widget(Window &window, std::shared_ptr<widget> parent) noexcept : super(window, parent) {}

    void initialize() noexcept override
    {
        _label_callback = label.subscribe([this](auto...) {
            _request_reconstrain = true;
        });

        _callback = subscribe([this](auto...) {
            this->clicked();
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (super::update_constraints()) {
            _label_stencil = (*label).make_stencil(Alignment::MiddleCenter, theme->labelStyle);
            _preferred_size = interval_vec2::make_minimum(_label_stencil->preferred_extent() + Theme::margin2Dx2);
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            _label_stencil->set_layout_parameters(rectangle(), base_line());
        }
        return super::update_layout(displayTimePoint, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

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
        _label_stencil->draw(context, true);

        abstract_button_widget::draw(std::move(context), display_time_point);
    }

    void clicked() noexcept
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        if (compare_then_assign(value, !value)) {
            window.requestRedraw = true;
        }
    }

private:
    bool value = false;
    bool pressed = false;

    decltype(label)::callback_ptr_type _label_callback;
    callback_ptr_type _callback;

    std::unique_ptr<stencil> _label_stencil;
};

} // namespace tt
