// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowTrafficLightsWidget.hpp"
#include "../GUI/utils.hpp"
#include "../text/TTauriIcons.hpp"
#include "../utils.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

WindowTrafficLightsWidget::WindowTrafficLightsWidget(Window &window, Widget *parent) noexcept : Widget(window, parent)
{
    // Toolbar buttons hug the toolbar and neighbour widgets.
    margin = 0.0f;
}

[[nodiscard]] bool WindowTrafficLightsWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (Widget::updateConstraints()) {
        if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
            ttlet width = Theme::toolbarDecorationButtonWidth * 3.0f;
            ttlet height = Theme::toolbarHeight;
            _preferred_size = {vec{width, height}, vec{width, std::numeric_limits<float>::infinity()}};

        } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
            ttlet width = DIAMETER * 3.0f + 2.0f * MARGIN + 2.0f * SPACING;
            ttlet height = DIAMETER + 2.0f * MARGIN;
            _preferred_size = {vec{width, height}, vec{width, std::numeric_limits<float>::infinity()}};

        } else {
            tt_no_default;
        }
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] bool WindowTrafficLightsWidget::updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    need_layout |= requestLayout.exchange(false);
    if (need_layout) {
        auto extent = rectangle().extent();

        if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
            closeRectangle =
                aarect{vec::point(extent.width() * 2.0f / 3.0f, 0.0f), vec{extent.width() * 1.0f / 3.0f, extent.height()}};

            maximizeRectangle =
                aarect{vec::point(extent.width() * 1.0f / 3.0f, 0.0f), vec{extent.width() * 1.0f / 3.0f, extent.height()}};

            minimizeRectangle = aarect{vec::point(0.0f, 0.0f), vec{extent.width() * 1.0f / 3.0f, extent.height()}};

        } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
            closeRectangle = aarect{vec::point(MARGIN, extent.height() / 2.0f - RADIUS), {DIAMETER, DIAMETER}};

            minimizeRectangle =
                aarect{vec::point(MARGIN + DIAMETER + SPACING, extent.height() / 2.0f - RADIUS), {DIAMETER, DIAMETER}};

            maximizeRectangle = aarect{
                vec::point(MARGIN + DIAMETER + SPACING + DIAMETER + SPACING, extent.height() / 2.0f - RADIUS), {DIAMETER, DIAMETER}};
        } else {
            tt_no_default;
        }

        closeWindowGlyph = to_FontGlyphIDs(TTauriIcon::CloseWindow);
        minimizeWindowGlyph = to_FontGlyphIDs(TTauriIcon::MinimizeWindow);

        if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
            maximizeWindowGlyph = to_FontGlyphIDs(TTauriIcon::MaximizeWindowMS);
            restoreWindowGlyph = to_FontGlyphIDs(TTauriIcon::RestoreWindowMS);

        } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
            maximizeWindowGlyph = to_FontGlyphIDs(TTauriIcon::MaximizeWindowMacOS);
            restoreWindowGlyph = to_FontGlyphIDs(TTauriIcon::RestoreWindowMacOS);
        } else {
            tt_no_default;
        }

        ttlet closeWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(closeWindowGlyph);
        ttlet minimizeWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(minimizeWindowGlyph);
        ttlet maximizeWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(maximizeWindowGlyph);
        ttlet restoreWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(restoreWindowGlyph);

        ttlet glyph_size = Theme::operatingSystem == OperatingSystem::MacOS ? 5.0f : Theme::iconSize;

        closeWindowGlyphRectangle = align(closeRectangle, scale(closeWindowGlyphBB, glyph_size), Alignment::MiddleCenter);
        minimizeWindowGlyphRectangle = align(minimizeRectangle, scale(minimizeWindowGlyphBB, glyph_size), Alignment::MiddleCenter);
        maximizeWindowGlyphRectangle = align(maximizeRectangle, scale(maximizeWindowGlyphBB, glyph_size), Alignment::MiddleCenter);
        restoreWindowGlyphRectangle = align(maximizeRectangle, scale(restoreWindowGlyphBB, glyph_size), Alignment::MiddleCenter);
    }
    return Widget::updateLayout(display_time_point, need_layout);
}

void WindowTrafficLightsWidget::drawMacOS(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto context = drawContext;
    context.cornerShapes = vec{RADIUS, RADIUS, RADIUS, RADIUS};

    if (!window.active && !hover) {
        context.fillColor = vec::color(0.246f, 0.246f, 0.246f);
    } else if (pressedClose) {
        context.fillColor = vec::color(1.0f, 0.242f, 0.212f);
    } else {
        context.fillColor = vec::color(1.0f, 0.1f, 0.082f);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(closeRectangle);

    if (!window.active && !hover) {
        context.fillColor = vec::color(0.246f, 0.246f, 0.246f);
    } else if (pressedMinimize) {
        context.fillColor = vec::color(1.0f, 0.847f, 0.093f);
    } else {
        context.fillColor = vec::color(0.784f, 0.521f, 0.021f);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(minimizeRectangle);

    if (!window.active && !hover) {
        context.fillColor = vec::color(0.246f, 0.246f, 0.246f);
    } else if (pressedMaximize) {
        context.fillColor = vec::color(0.223f, 0.863f, 0.1f);
    } else {
        context.fillColor = vec::color(0.082f, 0.533f, 0.024f);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(maximizeRectangle);

    if (hover) {
        context.color = vec::color(0.319f, 0.0f, 0.0f);
        context.drawGlyph(closeWindowGlyph, closeWindowGlyphRectangle);

        context.color = vec::color(0.212f, 0.1f, 0.0f);
        context.drawGlyph(minimizeWindowGlyph, minimizeWindowGlyphRectangle);

        context.color = vec::color(0.0f, 0.133f, 0.0f);
        if (window.size == Window::Size::Maximized) {
            context.drawGlyph(restoreWindowGlyph, restoreWindowGlyphRectangle);
        } else {
            context.drawGlyph(maximizeWindowGlyph, maximizeWindowGlyphRectangle);
        }
    }
}

void WindowTrafficLightsWidget::drawWindows(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto context = drawContext;

    if (pressedClose) {
        context.fillColor = vec::color(1.0f, 0.0f, 0.0f);
    } else if (hoverClose) {
        context.fillColor = vec::color(0.5f, 0.0f, 0.0f);
    } else {
        context.fillColor = theme->fillColor(nestingLevel() - 1);
    }
    context.drawFilledQuad(closeRectangle);

    if (pressedMinimize) {
        context.fillColor = theme->fillColor(nestingLevel() + 1);
    } else if (hoverMinimize) {
        context.fillColor = theme->fillColor(nestingLevel());
    } else {
        context.fillColor = theme->fillColor(nestingLevel() - 1);
    }
    context.drawFilledQuad(minimizeRectangle);

    if (pressedMaximize) {
        context.fillColor = theme->fillColor(nestingLevel() + 1);
    } else if (hoverMaximize) {
        context.fillColor = theme->fillColor(nestingLevel());
    } else {
        context.fillColor = theme->fillColor(nestingLevel() - 1);
    }
    context.drawFilledQuad(maximizeRectangle);

    if (window.active) {
        context.color = theme->foregroundColor;
    } else {
        context.color = theme->borderColor(nestingLevel());
    }
    context.drawGlyph(closeWindowGlyph, closeWindowGlyphRectangle);
    context.drawGlyph(minimizeWindowGlyph, minimizeWindowGlyphRectangle);
    if (window.size == Window::Size::Maximized) {
        context.drawGlyph(restoreWindowGlyph, restoreWindowGlyphRectangle);
    } else {
        context.drawGlyph(maximizeWindowGlyph, maximizeWindowGlyphRectangle);
    }
}

void WindowTrafficLightsWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        drawMacOS(drawContext, displayTimePoint);

    } else if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        drawWindows(drawContext, displayTimePoint);

    } else {
        tt_no_default;
    }

    Widget::draw(drawContext, displayTimePoint);
}

void WindowTrafficLightsWidget::handleMouseEvent(MouseEvent const &event) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    Widget::handleMouseEvent(event);

    if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
        if (pressedClose) {
            window.closeWindow();
        } else if (pressedMinimize) {
            window.minimizeWindow();
        } else if (pressedMaximize) {
            switch (window.size) {
            case Window::Size::Normal: window.maximizeWindow(); break;
            case Window::Size::Maximized: window.normalizeWindow(); break;
            default: tt_no_default;
            }
        }
    }

    auto stateHasChanged = false;

    // Check the hover states of each button.
    stateHasChanged |= compare_then_assign(hoverClose, closeRectangle.contains(event.position));
    stateHasChanged |= compare_then_assign(hoverMinimize, minimizeRectangle.contains(event.position));
    stateHasChanged |= compare_then_assign(hoverMaximize, maximizeRectangle.contains(event.position));

    // Only change the pressed state after checking for Button Up, the
    // button up will check which button was pressed from button down.
    stateHasChanged |= compare_then_assign(pressedClose, event.down.leftButton && hoverClose);
    stateHasChanged |= compare_then_assign(pressedMinimize, event.down.leftButton && hoverMinimize);
    stateHasChanged |= compare_then_assign(pressedMaximize, event.down.leftButton && hoverMaximize);

    if (stateHasChanged) {
        window.requestRedraw = true;
    }
}

HitBox WindowTrafficLightsWidget::hitBoxTest(vec position) const noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (closeRectangle.contains(position) || minimizeRectangle.contains(position) || maximizeRectangle.contains(position)) {
        return HitBox{this, elevation, HitBox::Type::Button};
    } else {
        return {};
    }
}

} // namespace tt
