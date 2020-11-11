// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "widget.hpp"
#include "../text/EditableText.hpp"
#include "../format.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class LineInputWidget final : public widget {
public:
    using super = widget;

    LineInputWidget(Window_base &window, std::shared_ptr<widget> parent, std::u8string const label) noexcept;

    LineInputWidget(Window_base &window, std::shared_ptr<widget> parent, label const label) noexcept :
        LineInputWidget(window, parent, label.text())
    {
    }

    ~LineInputWidget();

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] bool update_layout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override;
    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override;
    bool handle_command(command command) noexcept override;
    bool handle_mouse_event(MouseEvent const &event) noexcept override;
    bool handle_keyboard_event(KeyboardEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override;

    [[nodiscard]] bool accepts_focus() const noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());
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
