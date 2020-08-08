// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../GUI/DrawContext.hpp"
#include "../cells/TextCell.hpp"
#include "../text/format10.hpp"
#include "../observable.hpp"
#include "../attributes.hpp"
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
    observable<std::string> label;

    LabelWidget(Window &window, Widget *parent, Alignment alignment) noexcept :
        Widget(window, parent),
        alignment(alignment)
    {
        [[maybe_unused]] ttlet label_cbid = label.add_callback([this](auto...){
            requestConstraint = true;
        });
    }

    ~LabelWidget() {
    }

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override {
        tt_assume(mutex.is_locked_by_current_thread());

        if (ttlet result = Widget::updateConstraints(); result < WidgetUpdateResult::Self) {
            return result;
        }

        labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
        window.stopConstraintSolver();
        window.replaceConstraint(minimumWidthConstraint, width >= labelCell->preferredExtent().width());
        window.replaceConstraint(minimumHeightConstraint, height >= labelCell->preferredExtent().height());
        window.startConstraintSolver();
        return WidgetUpdateResult::Self;
    }

    void drawLabel(DrawContext drawContext) noexcept {
        tt_assume(mutex.is_locked_by_current_thread());

        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        labelCell->draw(drawContext, rectangle(), alignment, baseHeight(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        tt_assume(mutex.is_locked_by_current_thread());

        drawLabel(drawContext);
        Widget::draw(drawContext, displayTimePoint);
    }
};


}
