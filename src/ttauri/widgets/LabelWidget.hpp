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
            forceLayout = true;
        });
    }

    ~LabelWidget() {
    }

    LabelWidget(const LabelWidget &) = delete;
    LabelWidget &operator=(const LabelWidget &) = delete;
    LabelWidget(LabelWidget&&) = delete;
    LabelWidget &operator=(LabelWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override {
        Widget::layout(displayTimePoint);

        labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
        setMinimumHeight(labelCell->heightForWidth(rectangle().width()));
        setPreferredExtent(vec{labelCell->preferredExtent().width(), labelCell->heightForWidth(rectangle().width())});
    }

    void drawLabel(DrawContext drawContext) noexcept {
        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        labelCell->draw(drawContext, rectangle(), alignment, center(rectangle()).y(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        drawLabel(drawContext);
        Widget::draw(drawContext, displayTimePoint);
    }
};


}
