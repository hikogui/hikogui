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

class LabelWidget : public Widget {
protected:
    std::unique_ptr<TextCell> labelCell;
    Alignment alignment;

public:
    observable<std::u8string> label;

    template<typename... Args>
    LabelWidget(Window &window, Widget *parent, Alignment alignment, l10n const &fmt, Args const &... args) noexcept :
        Widget(window, parent),
        alignment(alignment),
        label(format(fmt, args...))
    {
        [[maybe_unused]] ttlet label_cbid = label.add_callback([this](auto...){
            requestConstraint = true;
        });
    }

    template<typename... Args>
    LabelWidget(Window &window, Widget *parent, l10n const &fmt, Args const &... args) noexcept :
        LabelWidget(window, parent, Alignment::TopRight, fmt, args...) {}

    LabelWidget(Window &window, Widget *parent, Alignment alignment) noexcept :
        LabelWidget(window, parent, alignment, l10n{}) {}

    ~LabelWidget() {
    }

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override {
        tt_assume(mutex.is_locked_by_current_thread());

        if (ttlet result = Widget::updateConstraints(); result < WidgetUpdateResult::Self) {
            return result;
        }

        labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
        _preferred_size = interval_vec2::make_minimum(labelCell->preferredExtent());
        return WidgetUpdateResult::Self;
    }

    void drawLabel(DrawContext drawContext) noexcept {
        tt_assume(mutex.is_locked_by_current_thread());

        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        labelCell->draw(drawContext, rectangle(), alignment, base_line(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        tt_assume(mutex.is_locked_by_current_thread());

        drawLabel(drawContext);
        Widget::draw(drawContext, displayTimePoint);
    }
};

}
