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
        Widget(window, parent, Theme::smallSize, Theme::smallSize),
        alignment(alignment)
    {
        [[maybe_unused]] ttlet label_cbid = label.add_callback([this](auto...){
            requestLayout = true;
        });
    }

    ~LabelWidget() {
    }

    bool layout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override {
        if (!Widget::layout(displayTimePoint, forceLayout)) {
            return false;
        }

        ttlet lock = std::scoped_lock(mutex);

        labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
        setMinimumHeight(labelCell->heightForWidth(rectangle().width()));
        setPreferredExtent(vec{labelCell->preferredExtent().width(), labelCell->heightForWidth(rectangle().width())});
        return true;
    }

    void drawLabel(DrawContext drawContext) noexcept {
        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        labelCell->draw(drawContext, rectangle(), alignment, baseHeight(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        drawLabel(drawContext);
        Widget::draw(drawContext, displayTimePoint);
    }
};


}
