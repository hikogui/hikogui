// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_button_widget.hpp"
#include "../Path.hpp"
#include "../GUI/DrawContext.hpp"
#include <memory>
#include <string>
#include <array>
#include <variant>

namespace tt {

class toolbar_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    observable<l10n_label> label;

    toolbar_button_widget(Window &window, std::shared_ptr<widget> parent) noexcept :
        abstract_button_widget(window, parent)
    {

        // Toolbar buttons hug the toolbar and neighbor widgets.
        _margin = 0.0f;
    }

    void initialize() noexcept override {
        super::initialize();
        _label_callback = this->label.subscribe([this](auto...) {
            _request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (super::update_constraints()) {
            _label_stencil = (*label).make_stencil(Alignment::MiddleCenter);
            ttlet width = Theme::toolbarDecorationButtonWidth;
            ttlet height = Theme::toolbarHeight;
            _preferred_size = {vec{width, height}, vec{width, std::numeric_limits<float>::infinity()}};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            _label_stencil->set_layout_parameters(rectangle());
        }
        return super::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        draw_background(context);
        draw_icon(context);
        super::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;
    std::unique_ptr<stencil> _label_stencil;

    void draw_background(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (_pressed) {
            context.fillColor = theme->fillColor(_semantic_layer + 1);
        } else if (_hover) {
            context.fillColor = theme->fillColor(_semantic_layer);
        } else {
            context.fillColor = theme->fillColor(_semantic_layer - 1);
        }
        context.drawFilledQuad(rectangle());
    }

    void draw_icon(DrawContext context) noexcept
    {
        context.transform = mat::T(0.0f, 0.0f, 0.1f) * context.transform;
        if (*enabled) {
            context.color = theme->foregroundColor;
        }
        _label_stencil->draw(context);
    }
};

} // namespace tt
