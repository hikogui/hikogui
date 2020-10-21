// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../GUI/DrawContext.hpp"
#include "../cells/TextCell.hpp"
#include "../text/format10.hpp"
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
    observable<std::u8string> label;

    template<typename... Args>
    LabelWidget(Window &window, Widget *parent, Alignment alignment, l10n const &fmt, Args const &... args) noexcept :
        Widget(window, parent),
        alignment(alignment),
        label(format(fmt, args...))
    {
        label_callback = scoped_callback(label, [this](auto...){
            request_reconstrain = true;
        });
    }

    template<typename... Args>
    LabelWidget(Window &window, Widget *parent, l10n const &fmt, Args const &... args) noexcept :
        LabelWidget(window, parent, Alignment::TopRight, fmt, args...) {}

    LabelWidget(Window &window, Widget *parent, Alignment alignment) noexcept :
        LabelWidget(window, parent, alignment, l10n{}) {}

    ~LabelWidget() {
    }

    [[nodiscard]] bool update_constraints() noexcept override {
        tt_assume(mutex.is_locked_by_current_thread());

        if (Widget::update_constraints()) {
            labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
            p_preferred_size = interval_vec2::make_minimum(labelCell->preferredExtent());
            return true;
        } else {
            return false;
        }
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override {
        tt_assume(mutex.is_locked_by_current_thread());

        if (*enabled) {
            context.color = theme->labelStyle.color;
        }

        labelCell->draw(context, rectangle(), alignment, base_line(), true);
        Widget::draw(std::move(context), display_time_point);
    }

private:
    scoped_callback<decltype(label)> label_callback;

    std::unique_ptr<TextCell> labelCell;
    Alignment alignment;
};

}
