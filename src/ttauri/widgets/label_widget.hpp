// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GUI/draw_context.hpp"
#include "../stencils/label_stencil.hpp"
#include "../observable.hpp"
#include "../alignment.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class label_widget final : public widget {
public:
    using super = widget;

    observable<label> label;

    template<typename Label>
    label_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        alignment alignment,
        Label &&label) noexcept
        :
        super(window, parent),
        _alignment(alignment),
        label(std::forward<Label>(label))
    {
    }

    template<typename Label>
    label_widget(gui_window &window, std::shared_ptr<abstract_container_widget> parent, Label &&label) noexcept :
        super(window, parent), _alignment(alignment::top_right), label(std::forward<Label>(label))
    {
    }

    ~label_widget() {
    }

    void init() noexcept override {
        _label_callback = label.subscribe([this](auto...) {
            _request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_cell = stencil::make_unique(_alignment, *label, theme::global->labelStyle);
            _preferred_size = interval_extent2::make_minimum(_label_cell->preferred_extent());
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
            _label_cell->set_layout_parameters(rectangle(), this->base_line());
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, _clipping_rectangle)) {
            _label_cell->draw(context, this->label_color());
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;

    std::unique_ptr<label_stencil> _label_cell;
    alignment _alignment;
};

}
