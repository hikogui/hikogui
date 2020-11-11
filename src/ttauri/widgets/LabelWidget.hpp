// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "widget.hpp"
#include "../GUI/DrawContext.hpp"
#include "../stencils/label_stencil.hpp"
#include "../observable.hpp"
#include "../alignment.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class LabelWidget final : public widget {
public:
    using super = widget;

    observable<label> label;

    template<typename Label>
    LabelWidget(gui_window &window, std::shared_ptr<widget> parent, alignment alignment, Label &&label) noexcept
        :
        super(window, parent),
        alignment(alignment),
        label(std::forward<Label>(label))
    {
    }

    template<typename Label>
    LabelWidget(gui_window &window, std::shared_ptr<widget> parent, Label &&label) noexcept :
        super(window, parent), alignment(alignment::top_right), label(std::forward<Label>(label))
    {
    }

    ~LabelWidget() {
    }

    void initialize() noexcept override {
        label_callback = label.subscribe([this](auto...) {
            _request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            labelCell = stencil::make_unique(alignment, *label, theme->labelStyle);
            _preferred_size = interval_vec2::make_minimum(labelCell->preferred_extent());
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            labelCell->set_layout_parameters(rectangle(), base_line());
        }
        return super::update_layout(displayTimePoint, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override {
        tt_assume(gui_system_mutex.recurse_lock_count());

        if (*enabled) {
            context.color = theme->labelStyle.color;
        }

        labelCell->draw(context, true);
        widget::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(label)::callback_ptr_type label_callback;

    std::unique_ptr<label_stencil> labelCell;
    alignment alignment;
};

}
