// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_button_widget.hpp"
#include "../stencils/label_stencil.hpp"
#include "../Path.hpp"
#include "../GUI/DrawContext.hpp"
#include <memory>
#include <string>
#include <array>
#include <variant>

namespace tt {

template<typename T>
class toolbar_button_widget final : public abstract_button_widget<T> {
public:
    using super = abstract_button_widget<T>;
    using value_type = typename super::value_type;

    observable<l10n_label> label;

    template<typename Value = observable<value_type>>
    toolbar_button_widget(Window &window, std::shared_ptr<widget> parent, value_type true_value, Value &&value = {}) noexcept :
        super(window, parent, std::move(true_value), std::forward<Value>(value))
    {

        // Toolbar buttons hug the toolbar and neighbor widgets.
        this->_margin = 0.0f;
    }

    void initialize() noexcept override {
        super::initialize();
        _label_callback = this->label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (super::update_constraints()) {
            _label_stencil = stencil::make_unique(Alignment::MiddleLeft, *label, theme->labelStyle);
            ttlet width = _label_stencil->preferred_extent().width() + Theme::margin * 2.0f;
            ttlet height = _label_stencil->preferred_extent().height() + Theme::margin * 2.0f;
            this->_preferred_size = {
                vec{width, height}, vec{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            _label_stencil->set_layout_parameters(shrink(this->rectangle(), vec{Theme::margin, 0.0f}));
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
    std::unique_ptr<label_stencil> _label_stencil;

    void draw_background(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        context.color = context.fillColor;
        if (this->_focus && this->window.active) {
            context.color = theme->accentColor;
        }

        if (this->value == this->true_value) {
            context.fillColor = theme->accentColor;
        }

        context.drawBoxIncludeBorder(this->rectangle());
    }

    void draw_icon(DrawContext context) noexcept
    {
        context.transform = mat::T(0.0f, 0.0f, 0.1f) * context.transform;
        if (*this->enabled) {
            context.color = theme->foregroundColor;
        }
        _label_stencil->draw(context);
    }
};

} // namespace tt
