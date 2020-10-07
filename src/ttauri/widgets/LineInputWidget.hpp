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

class LineInputWidget final : public Widget {
public:
    LineInputWidget(Window &window, Widget *parent, std::u8string const label) noexcept;

    LineInputWidget(Window &window, Widget *parent, l10n const label) noexcept :
        LineInputWidget(window, parent, std::u8string{static_cast<std::u8string_view>(label)})
    {
    }

    ~LineInputWidget();

    [[nodiscard]] bool updateConstraints() noexcept override;
    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override;
    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override;
    void handleCommand(command command) noexcept override;
    bool handleMouseEvent(MouseEvent const &event) noexcept override;
    void handleKeyboardEvent(KeyboardEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override;

    [[nodiscard]] bool acceptsFocus() const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return *enabled;
    }

private:
    std::u8string label = u8"<unknown>";

    EditableText field;
    ShapedText shapedText;
    aarect textRectangle = {};
    aarect textClippingRectangle = {};
    aarect leftToRightCaret = {};

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

    void dragSelect() noexcept;
    void scrollText() noexcept;
    void drawBackgroundBox(DrawContext const &context) const noexcept;
    void drawSelectionRectangles(DrawContext context) const noexcept;
    void drawPartialGraphemeCaret(DrawContext context) const noexcept;
    void drawCaret(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept;
    void drawText(DrawContext context) const noexcept;
};

} // namespace tt
