// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/WindowTrafficLightsWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Text/TTauriIcons.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

WindowTrafficLightsWidget::WindowTrafficLightsWidget(Window &window, Widget *parent, Path applicationIcon) noexcept :
    Widget(window, parent, vec{WIDTH, HEIGHT}),
    applicationIcon(std::move(applicationIcon))
{
}

int WindowTrafficLightsWidget::state() const noexcept {
    int r = 0;
    r |= window.active ? 1 : 0;
    if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        r |= hover ? 2 : 0;
        r |= pressedRed ? 4 : 0;
        r |= pressedYellow ? 8 : 0;
        r |= pressedGreen ? 16 : 0;
        r |= (window.size == Window::Size::Maximized) ? 32 : 0;
    }
    return r;
}

void WindowTrafficLightsWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept
{
    Widget::layout(displayTimePoint);

    redRectangle = aarect{
        vec::point(MARGIN, extent().height() / 2.0 - RADIUS),
        {DIAMETER, DIAMETER}
    };

    yellowRectangle = aarect{
        vec::point(MARGIN + DIAMETER + SPACING, extent().height() / 2.0 - RADIUS),
        {DIAMETER, DIAMETER}
    };

    greenRectangle = aarect{
        vec::point(MARGIN + DIAMETER + SPACING + DIAMETER + SPACING, extent().height() / 2.0 - RADIUS),
        {DIAMETER, DIAMETER}
    };

    closeWindowGlyph = Text::to_FontGlyphIDs(Text::TTauriIcon::CloseWindow);
    minimizeWindowGlyph = Text::to_FontGlyphIDs(Text::TTauriIcon::MinimizeWindow);
    maximizeWindowGlyph = Text::to_FontGlyphIDs(Text::TTauriIcon::MaximizeWindowMacOS);
    restoreWindowGlyph = Text::to_FontGlyphIDs(Text::TTauriIcon::RestoreWindowMacOS);

    let closeWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(closeWindowGlyph);
    let minimizeWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(minimizeWindowGlyph);
    let maximizeWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(maximizeWindowGlyph);
    let restoreWindowGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(restoreWindowGlyph);

    closeWindowGlyphRectangle = align(redRectangle, scale(closeWindowGlyphBB, GLYPH_SIZE), Alignment::MiddleCenter);
    minimizeWindowGlyphRectangle = align(yellowRectangle, scale(minimizeWindowGlyphBB, GLYPH_SIZE), Alignment::MiddleCenter);
    maximizeWindowGlyphRectangle = align(greenRectangle, scale(maximizeWindowGlyphBB, GLYPH_SIZE), Alignment::MiddleCenter);
    restoreWindowGlyphRectangle = align(greenRectangle, scale(restoreWindowGlyphBB, GLYPH_SIZE), Alignment::MiddleCenter);
}

void WindowTrafficLightsWidget::drawMacOS(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto context = drawContext;
    context.cornerShapes = vec{RADIUS, RADIUS, RADIUS, RADIUS};

    if (!window.active && !hover) {
        context.fillColor = vec::color(0.246, 0.246, 0.246);
    } else if (pressedRed) {
        context.fillColor = vec::color(1.0, 0.242, 0.212);
    } else {
        context.fillColor = vec::color(1.0, 0.1, 0.082);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(redRectangle);

    if (!window.active && !hover) {
        context.fillColor = vec::color(0.246, 0.246, 0.246);
    } else if (pressedYellow) {
        context.fillColor = vec::color(1.0, 0.847, 0.093);
    } else {
        context.fillColor = vec::color(0.784, 0.521, 0.021);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(yellowRectangle);

    if (!window.active && !hover) {
        context.fillColor = vec::color(0.246, 0.246, 0.246);
    } else if (pressedGreen) {
        context.fillColor = vec::color(0.223, 0.863, 0.1);
    } else {
        context.fillColor = vec::color(0.082, 0.533, 0.024);
    }
    context.color = context.fillColor;
    context.drawBoxIncludeBorder(greenRectangle);

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

void WindowTrafficLightsWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        drawMacOS(drawContext, displayTimePoint);

    } else if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {

        auto drawingBackingImage = backingImage.loadOrDraw(
            window,
            extent(),
            [&](auto image) {
                return drawImage(image);
            },
            "WindowTrafficLightsWidget", state()
        );

        if (drawingBackingImage) {
            forceRedraw = true;
        }

        if (backingImage.image) {
            let currentScale = (extent() / vec{backingImage.image->extent}).xy11();

            auto context = drawContext;
            context.transform = context.transform * mat::S(currentScale);
            context.drawImage(*(backingImage.image));
        }
    }

    Widget::draw(drawContext, displayTimePoint);
}

PixelMap<R16G16B16A16SFloat> WindowTrafficLightsWidget::drawApplicationIconImage(PipelineImage::Image &image) noexcept
{
    auto linearMap = PixelMap<R16G16B16A16SFloat>{image.extent};
    fill(linearMap);

    let iconPath = applicationIcon.centerScale(vec{image.extent}, 3.0);
    composit(linearMap, iconPath);

    if (!window.active) {
        desaturate(linearMap, 0.5f);
    }
    return linearMap;
}

PipelineImage::Backing::ImagePixelMap WindowTrafficLightsWidget::drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept
{
    return { std::move(image), drawApplicationIconImage(*image) };
}

void WindowTrafficLightsWidget::handleMouseEvent(MouseEvent const &event) noexcept
{
    Widget::handleMouseEvent(event);

    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        // The system menu is opened by Windows 10, due to HitBox returning "system menu".
        return;

    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            if (pressedRed) {
                window.closeWindow();
            } else if (pressedYellow) {
                window.minimizeWindow();
            } else if (pressedGreen) {
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

        // Only change the pressed state after checking for Button Up, the
        // button up will check which button was pressed from button down.
        auto stateHasChanged = false;
        stateHasChanged |= assign_and_compare(pressedRed, event.down.leftButton && redRectangle.contains(event.position));
        stateHasChanged |= assign_and_compare(pressedYellow, event.down.leftButton && yellowRectangle.contains(event.position));
        stateHasChanged |= assign_and_compare(pressedGreen, event.down.leftButton && greenRectangle.contains(event.position));
        if (stateHasChanged) {
            forceRedraw = true;
        }

    } else {
        no_default;
    }
}

HitBox WindowTrafficLightsWidget::hitBoxTest(vec position) const noexcept
{
    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        if (rectangle().contains(position)) {
            return HitBox{this, elevation, HitBox::Type::ApplicationIcon};
        } else {
            return {};
        }

    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        if (redRectangle.contains(position) ||
            yellowRectangle.contains(position) ||
            greenRectangle.contains(position)
        ) {
            return HitBox{this, elevation, HitBox::Type::Button};
        } else {
            return {};
        }

    } else {
        no_default;
    }
}

}
