// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/WindowTrafficLightsWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Text/TTauriIcons.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

vec WindowTrafficLightsWidget::calculateExtent(Window &window) noexcept
{
    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        return {
            Theme::toolbarDecorationButtonWidth * 3.0f,
            Theme::toolbarHeight
        };

    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        return {
            DIAMETER * 3.0 + 2.0 * MARGIN + 2 * SPACING,
            DIAMETER + 2.0 * MARGIN
        };

    } else {
        no_default;
    }
}

WindowTrafficLightsWidget::WindowTrafficLightsWidget(Window &window, Widget *parent) noexcept :
    Widget(window, parent, calculateExtent(window))
{
    setFixedExtent(calculateExtent(window));
}


void WindowTrafficLightsWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept
{
    Widget::layout(displayTimePoint);

    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        closeRectangle = aarect{
            vec::point(extent().width() * 2.0f/3.0f, 0.0f),
            vec{extent().width() * 1.0f/3.0f, extent().height()}
        };

        maximizeRectangle = aarect{
            vec::point(extent().width() * 1.0f/3.0f, 0.0f),
            vec{extent().width() * 1.0f/3.0f, extent().height()}
        };

        minimizeRectangle = aarect{
            vec::point(0.0f, 0.0f),
            vec{extent().width() * 1.0f/3.0f, extent().height()}
        };

    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        closeRectangle = aarect{
            vec::point(MARGIN, extent().height() / 2.0 - RADIUS),
            {DIAMETER, DIAMETER}
        };

        minimizeRectangle = aarect{
            vec::point(MARGIN + DIAMETER + SPACING, extent().height() / 2.0 - RADIUS),
            {DIAMETER, DIAMETER}
        };

        maximizeRectangle = aarect{
            vec::point(MARGIN + DIAMETER + SPACING + DIAMETER + SPACING, extent().height() / 2.0 - RADIUS),
            {DIAMETER, DIAMETER}
        };
    } else {
        no_default;
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
        no_default;
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

void WindowTrafficLightsWidget::drawMacOS(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto context = drawContext;
    context.cornerShapes = vec{RADIUS, RADIUS, RADIUS, RADIUS};

    if (!window.active && !hover) {
        context.fillColor = vec::color(0.246, 0.246, 0.246);
    } else if (pressedClose) {
        context.fillColor = vec::color(1.0, 0.242, 0.212);
    } else {
        context.fillColor = vec::color(1.0, 0.1, 0.082);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(closeRectangle);

    if (!window.active && !hover) {
        context.fillColor = vec::color(0.246, 0.246, 0.246);
    } else if (pressedMinimize) {
        context.fillColor = vec::color(1.0, 0.847, 0.093);
    } else {
        context.fillColor = vec::color(0.784, 0.521, 0.021);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(minimizeRectangle);

    if (!window.active && !hover) {
        context.fillColor = vec::color(0.246, 0.246, 0.246);
    } else if (pressedMaximize) {
        context.fillColor = vec::color(0.223, 0.863, 0.1);
    } else {
        context.fillColor = vec::color(0.082, 0.533, 0.024);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(maximizeRectangle);

    if (hover) {
        context.color = vec::color(0.319, 0.0, 0.0);
        context.drawGlyph(closeWindowGlyph, closeWindowGlyphRectangle);

        context.color = vec::color(0.212, 0.1, 0.0);
        context.drawGlyph(minimizeWindowGlyph, minimizeWindowGlyphRectangle);

        context.color = vec::color(0.0, 0.133, 0.0);
        if (window.size == Window::Size::Maximized) {
            context.drawGlyph(restoreWindowGlyph, restoreWindowGlyphRectangle);
        } else {
            context.drawGlyph(maximizeWindowGlyph, maximizeWindowGlyphRectangle);
        }
    }
}

void WindowTrafficLightsWidget::drawWindows(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
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
    if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        drawMacOS(drawContext, displayTimePoint);

    } else if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        drawWindows(drawContext, displayTimePoint);

    } else {
        no_default;
    }

    Widget::draw(drawContext, displayTimePoint);
}

void WindowTrafficLightsWidget::handleMouseEvent(MouseEvent const &event) noexcept
{
    Widget::handleMouseEvent(event);



    if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
        if (pressedClose) {
            window.closeWindow();
        } else if (pressedMinimize) {
            window.minimizeWindow();
        } else if (pressedMaximize) {
            switch (window.size) {
            case Window::Size::Normal:
                window.maximizeWindow();
                break;
            case Window::Size::Maximized:
                window.normalizeWindow();
                break;
            default:
                no_default;
            }
        }
    }

    auto stateHasChanged = false;

    // Check the hover states of each button.
    stateHasChanged |= assign_and_compare(hoverClose, closeRectangle.contains(event.position));
    stateHasChanged |= assign_and_compare(hoverMinimize, minimizeRectangle.contains(event.position));
    stateHasChanged |= assign_and_compare(hoverMaximize, maximizeRectangle.contains(event.position));

    // Only change the pressed state after checking for Button Up, the
    // button up will check which button was pressed from button down.
    stateHasChanged |= assign_and_compare(pressedClose, event.down.leftButton && hoverClose);
    stateHasChanged |= assign_and_compare(pressedMinimize, event.down.leftButton && hoverMinimize);
    stateHasChanged |= assign_and_compare(pressedMaximize, event.down.leftButton && hoverMaximize);


    if (stateHasChanged) {
        forceRedraw = true;
    }

}

HitBox WindowTrafficLightsWidget::hitBoxTest(vec position) const noexcept
{
    if (closeRectangle.contains(position) ||
        minimizeRectangle.contains(position) ||
        maximizeRectangle.contains(position)
    ) {
        return HitBox{this, elevation, HitBox::Type::Button};
    } else {
        return {};
    }
}

}
