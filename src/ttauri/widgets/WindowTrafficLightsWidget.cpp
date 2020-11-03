// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowTrafficLightsWidget.hpp"
#include "../GUI/utils.hpp"
#include "../text/TTauriIcons.hpp"
#include "../utils.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

WindowTrafficLightsWidget::WindowTrafficLightsWidget(Window &window, std::shared_ptr<widget> parent) noexcept :
    widget(window, parent)
{
    // Toolbar buttons hug the toolbar and neighbor widgets.
    _margin = 0.0f;
}

[[nodiscard]] bool WindowTrafficLightsWidget::update_constraints() noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    if (widget::update_constraints()) {
        if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
            ttlet width = Theme::toolbarDecorationButtonWidth * 3.0f;
            ttlet height = Theme::toolbarHeight;
            _preferred_size = {vec{width, height}, vec{width, std::numeric_limits<float>::infinity()}};

        } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
            ttlet width = DIAMETER * 3.0f + 2.0f * MARGIN + 2.0f * SPACING;
            ttlet height = DIAMETER + 2.0f * MARGIN;
            _preferred_size = {vec{width, height}, vec{width, std::numeric_limits<float>::infinity()}};

        } else {
            tt_no_default();
        }
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] bool
WindowTrafficLightsWidget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    need_layout |= std::exchange(_request_relayout, false);
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
                vec::point(MARGIN + DIAMETER + SPACING + DIAMETER + SPACING, extent.height() / 2.0f - RADIUS),
                {DIAMETER, DIAMETER}};
        } else {
            tt_no_default();
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
            tt_no_default();
        }

        ttlet closeWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(closeWindowGlyph);
        ttlet minimizeWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(minimizeWindowGlyph);
        ttlet maximizeWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(maximizeWindowGlyph);
        ttlet restoreWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(restoreWindowGlyph);

        ttlet glyph_size = Theme::operatingSystem == OperatingSystem::MacOS ? 5.0f : Theme::iconSize;

        closeWindowGlyphRectangle = align(closeRectangle, scale(closeWindowGlyphBB, glyph_size), Alignment::MiddleCenter);
        minimizeWindowGlyphRectangle =
            align(minimizeRectangle, scale(minimizeWindowGlyphBB, glyph_size), Alignment::MiddleCenter);
        maximizeWindowGlyphRectangle =
            align(maximizeRectangle, scale(maximizeWindowGlyphBB, glyph_size), Alignment::MiddleCenter);
        restoreWindowGlyphRectangle = align(maximizeRectangle, scale(restoreWindowGlyphBB, glyph_size), Alignment::MiddleCenter);
    }
    return widget::update_layout(display_time_point, need_layout);
}

void WindowTrafficLightsWidget::drawMacOS(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    auto context = drawContext;
    context.cornerShapes = vec{RADIUS, RADIUS, RADIUS, RADIUS};

    if (!window.active && !_hover) {
        context.fillColor = vec::color(0.246f, 0.246f, 0.246f);
    } else if (pressedClose) {
        context.fillColor = vec::color(1.0f, 0.242f, 0.212f);
    } else {
        context.fillColor = vec::color(1.0f, 0.1f, 0.082f);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(closeRectangle);

    if (!window.active && !_hover) {
        context.fillColor = vec::color(0.246f, 0.246f, 0.246f);
    } else if (pressedMinimize) {
        context.fillColor = vec::color(1.0f, 0.847f, 0.093f);
    } else {
        context.fillColor = vec::color(0.784f, 0.521f, 0.021f);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(minimizeRectangle);

    if (!window.active && !_hover) {
        context.fillColor = vec::color(0.246f, 0.246f, 0.246f);
    } else if (pressedMaximize) {
        context.fillColor = vec::color(0.223f, 0.863f, 0.1f);
    } else {
        context.fillColor = vec::color(0.082f, 0.533f, 0.024f);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(maximizeRectangle);

    if (_hover) {
        context.transform = mat::T{0.0f, 0.0f, 0.1f} * context.transform;
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
    tt_assume(GUISystem_mutex.recurse_lock_count());

    auto context = drawContext;

    if (pressedClose) {
        context.fillColor = vec::color(1.0f, 0.0f, 0.0f);
    } else if (hoverClose) {
        context.fillColor = vec::color(0.5f, 0.0f, 0.0f);
    } else {
        context.fillColor = theme->fillColor(_semantic_layer - 1);
    }
    context.drawFilledQuad(closeRectangle);

    if (pressedMinimize) {
        context.fillColor = theme->fillColor(_semantic_layer + 1);
    } else if (hoverMinimize) {
        context.fillColor = theme->fillColor(_semantic_layer);
    } else {
        context.fillColor = theme->fillColor(_semantic_layer - 1);
    }
    context.drawFilledQuad(minimizeRectangle);

    if (pressedMaximize) {
        context.fillColor = theme->fillColor(_semantic_layer + 1);
    } else if (hoverMaximize) {
        context.fillColor = theme->fillColor(_semantic_layer);
    } else {
        context.fillColor = theme->fillColor(_semantic_layer - 1);
    }
    context.drawFilledQuad(maximizeRectangle);

    if (window.active) {
        context.color = theme->foregroundColor;
    } else {
        context.color = theme->borderColor(_semantic_layer);
    }
    context.transform = mat::T{0.0f, 0.0f, 0.1f} * context.transform;
    context.drawGlyph(closeWindowGlyph, closeWindowGlyphRectangle);
    context.drawGlyph(minimizeWindowGlyph, minimizeWindowGlyphRectangle);
    if (window.size == Window::Size::Maximized) {
        context.drawGlyph(restoreWindowGlyph, restoreWindowGlyphRectangle);
    } else {
        context.drawGlyph(maximizeWindowGlyph, maximizeWindowGlyphRectangle);
    }
}

void WindowTrafficLightsWidget::draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        drawMacOS(context, display_time_point);

    } else if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        drawWindows(context, display_time_point);

    } else {
        tt_no_default();
    }

    widget::draw(std::move(context), display_time_point);
}

bool WindowTrafficLightsWidget::handle_mouse_event(MouseEvent const &event) noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);
    auto handled = widget::handle_mouse_event(event);

    // Check the hover states of each button.
    auto stateHasChanged = false;
    ttlet position = _from_window_transform * event.position;
    stateHasChanged |= compare_then_assign(hoverClose, closeRectangle.contains(position));
    stateHasChanged |= compare_then_assign(hoverMinimize, minimizeRectangle.contains(position));
    stateHasChanged |= compare_then_assign(hoverMaximize, maximizeRectangle.contains(position));
    if (stateHasChanged) {
        window.requestRedraw = true;
    }

    if (event.cause.leftButton) {
        handled = true;
        
        switch (event.type) {
        using enum MouseEvent::Type;
        case ButtonUp:
            if (pressedClose && hoverClose) {
                window.closeWindow();
            }

            if (pressedMinimize && hoverMinimize) {
                window.minimizeWindow();
            }

            if (pressedMaximize && hoverMaximize) {
                switch (window.size) {
                case Window::Size::Normal: window.maximizeWindow(); break;
                case Window::Size::Maximized: window.normalizeWindow(); break;
                default: tt_no_default();
                }
            }

            window.requestRedraw = true;
            pressedClose = false;
            pressedMinimize = false;
            pressedMaximize = false;
            break;

        case ButtonDown:
            window.requestRedraw = true;
            pressedClose = hoverClose;
            pressedMinimize = hoverMinimize;
            pressedMaximize = hoverMaximize;
            break;
        }
    }

    return handled;
}

HitBox WindowTrafficLightsWidget::hitbox_test(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);
    ttlet position = _from_window_transform * window_position;

    if (_window_clipping_rectangle.contains(window_position)) {
        if (closeRectangle.contains(position) || minimizeRectangle.contains(position) ||
            maximizeRectangle.contains(position)) {
            return HitBox{weak_from_this(), _draw_layer, HitBox::Type::Button};
        } else {
            return HitBox{};
        }
    } else {
        return {};
    }
}

} // namespace tt
