// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
#include "TTauri/Text/EditableText.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri {

class LineInputWidget : public Widget {
protected:
    std::string label = "<unknown>";

    EditableText field;
    ShapedText shapedText;
    aarect textRectangle = {};
    aarect textClippingRectangle = {};
    aarect leftToRightCaret = {};
    aarect partialGraphemeCaret = {};
    std::vector<aarect> selectionRectangles = {};

    /** Scroll speed in points per second.
     * This is used when dragging outside of the widget.
     */
    float dragScrollSpeedX = 0.0f;

    /** Number of mouse clicks that caused the drag.
     */
    int dragClickCount = 0;

    vec dragSelectPosition = {};

    /** How much the text has scrolled in points.
     */
    float textScrollX = 0.0f;

    mat::T2 textTranslate;
    mat::T2 textInvTranslate;

    static constexpr hires_utc_clock::duration blinkInterval = 500ms;
    hires_utc_clock::time_point nextRedrawTimePoint;
    hires_utc_clock::time_point lastUpdateTimePoint;
public:

    LineInputWidget(
        Window &window, Widget *parent,
        std::string const label
    ) noexcept;

    ~LineInputWidget();

    LineInputWidget(const LineInputWidget &) = delete;
    LineInputWidget &operator=(const LineInputWidget &) = delete;
    LineInputWidget(LineInputWidget&&) = delete;
    LineInputWidget &operator=(LineInputWidget &&) = delete;

    [[nodiscard]] int needs(hires_utc_clock::time_point displayTimePoint) noexcept override;
    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override;
    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    void handleCommand(string_ltag command) noexcept override;

    void handleMouseEvent(MouseEvent const &event) noexcept override;
    void handleKeyboardEvent(KeyboardEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

    [[nodiscard]] bool acceptsFocus() const noexcept override {
        return enabled;
    }

private:
    void dragSelect() noexcept;
};

}
