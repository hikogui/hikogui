// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../GUI/DrawContext.hpp"
#include "../cells/TextCell.hpp"
#include "../observable.hpp"
#include "../alignment.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class LabelWidget final : public Widget {
public:
    observable<l10n_label> label;

    template<typename Label>
    LabelWidget(Window &window, std::shared_ptr<Widget> parent, Alignment alignment, Label &&label) noexcept
        :
        Widget(window, parent),
        alignment(alignment),
        label(std::forward<Label>(label))
    {
    }

    template<typename Label>
    LabelWidget(Window &window, std::shared_ptr<Widget> parent, Label &&label) noexcept :
        Widget(window, parent), alignment(Alignment::TopRight), label(std::forward<Label>(label))
    {
    }

    ~LabelWidget() {
    }

    void initialize() noexcept override {
        label_callback = label.subscribe([this](auto...) {
            request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (Widget::update_constraints()) {
            labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
            p_preferred_size = interval_vec2::make_minimum(labelCell->preferredExtent());
            return true;
        } else {
            return false;
        }
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (*enabled) {
            context.color = theme->labelStyle.color;
        }

        labelCell->draw(context, rectangle(), alignment, base_line(), true);
        Widget::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(label)::callback_ptr_type label_callback;

    std::unique_ptr<TextCell> labelCell;
    Alignment alignment;
};

}
