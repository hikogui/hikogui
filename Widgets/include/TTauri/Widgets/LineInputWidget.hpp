// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/cpu_utc_clock.hpp"
#include "TTauri/GUI/Widget.hpp"
#include "TTauri/Text/EditableText.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

class LineInputWidget : public Widget {
protected:
    std::string label = "<unknown>";

    Text::EditableText field;
    Text::ShapedText shapedText;

    aarect textRectangle = {};
    aarect leftToRightCaret = {};
    aarect partialGraphemeCaret = {};
    std::vector<aarect> selectionRectangles = {};

    static constexpr cpu_utc_clock::duration blinkInterval = 500ms;
    cpu_utc_clock::time_point lastRedrawTimePoint;
    cpu_utc_clock::time_point lastUpdateTimePoint;
public:

    LineInputWidget(
        Window &window, Widget *parent,
        std::string const label
    ) noexcept;

    ~LineInputWidget() {}

    LineInputWidget(const LineInputWidget &) = delete;
    LineInputWidget &operator=(const LineInputWidget &) = delete;
    LineInputWidget(LineInputWidget&&) = delete;
    LineInputWidget &operator=(LineInputWidget &&) = delete;

    [[nodiscard]] WidgetNeed needs() const noexcept override;
    [[nodiscard]] void layout() noexcept override;
    void draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept override;

    void handleCommand(string_ltag command) noexcept;

    void handleMouseEvent(GUI::MouseEvent const &event) noexcept override;
    void handleKeyboardEvent(GUI::KeyboardEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept;

    [[nodiscard]] bool acceptsFocus() noexcept override {
        return enabled;
    }

};

}
