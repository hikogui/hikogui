// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../text/EditableText.hpp"
#include "../text/l10n.hpp"
#include "../text/format.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class LineInputWidget : public Widget {
protected:
    std::u8string label = u8"<unknown>";

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
    LineInputWidget(Window &window, Widget *parent, std::u8string const label) noexcept;

    LineInputWidget(Window &window, Widget *parent, l10n const label) noexcept :
        LineInputWidget(window, parent, std::u8string{static_cast<std::u8string_view>(label)}) {}

    ~LineInputWidget();

    [[nodiscard]] bool updateConstraints() noexcept override;
    [[nodiscard]] WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override;
    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    void handleCommand(command command) noexcept override;

    void handleMouseEvent(MouseEvent const &event) noexcept override;
    void handleKeyboardEvent(KeyboardEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

    [[nodiscard]] bool acceptsFocus() const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return *enabled;
    }

private:
    void dragSelect() noexcept;
};

} // namespace tt
