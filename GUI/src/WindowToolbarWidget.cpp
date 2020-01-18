// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/WindowToolbarWidget.hpp"
#include "TTauri/GUI/WindowTrafficLightsWidget.hpp"
#include "TTauri/GUI/ToolbarButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include <cmath>

namespace TTauri::GUI::Widgets {

using namespace std;

WindowToolbarWidget::WindowToolbarWidget() noexcept :
    Widget()
{
}

void WindowToolbarWidget::setParent(Widget *parent) noexcept
{
    Widget::setParent(parent);

    trafficLightButtons = addWidget<WindowTrafficLightsWidget>(
        getResource<Path>(URL("resource:Themes/Icons/Application Icon.tticon"))
    );
    window->addConstraint(trafficLightButtons->box.outerTop() == box.top());
    window->addConstraint(trafficLightButtons->box.outerLeft() == box.left);
    window->addConstraint(trafficLightButtons->box.outerBottom() == box.bottom);

    if constexpr (operatingSystem == OperatingSystem::Windows) {
        closeWindowButton = addWidget<ToolbarButtonWidget>(
            0.33f * getResource<Path>(URL("resource:Themes/Icons/Close%20Window.tticon")),
            [&]() { window->closeWindow(); }
        );
        closeWindowButton->hoverBackgroundColor = wsRGBA{ 0xdd0000ff };
        closeWindowButton->pressedBackgroundColor = wsRGBA{ 0xff0000ff };
        window->addConstraint(closeWindowButton->box.outerTop() == box.top());
        window->addConstraint(closeWindowButton->box.outerRight() == box.right());
        window->addConstraint(closeWindowButton->box.outerBottom() == box.bottom);

        maximizeWindowButton = addWidget<ToolbarButtonWidget>(
            0.33f * getResource<Path>(URL("resource:Themes/Icons/Maximize%20Window.tticon")),
            [&]() { 
                switch (window->size) {
                case Window::Size::Normal:
                    window->maximizeWindow();
                    break;
                case Window::Size::Maximized:
                    window->normalizeWindow();
                    break;
                default:
                    no_default;
                }
            }
        );
        window->addConstraint(maximizeWindowButton->box.outerTop() == box.top());
        window->addConstraint(maximizeWindowButton->box.outerRight() == closeWindowButton->box.outerLeft());
        window->addConstraint(maximizeWindowButton->box.outerBottom() == box.bottom);

        minimizeWindowButton = addWidget<ToolbarButtonWidget>(
            0.33f * getResource<Path>(URL("resource:Themes/Icons/Minimize%20Window.tticon")),
            //getResource<Path>(URL("resource:Themes/Icons/MultiColor.tticon")),
            [&]() { window->minimizeWindow(); }
        );
        window->addConstraint(minimizeWindowButton->box.outerTop() == box.top());
        window->addConstraint(minimizeWindowButton->box.outerRight() == maximizeWindowButton->box.outerLeft());
        window->addConstraint(minimizeWindowButton->box.outerBottom() == box.bottom);
    }
}

void WindowToolbarWidget::pipelineFlatPlaceVertices(gsl::span<PipelineFlat::Vertex> &vertices, int &offset) noexcept
{
    PipelineFlat::Vertex::placeBox(vertices, offset, box.currentRectangle(), glm::vec4{0.0,0.0,0.0,0.5}, box.currentRectangle());

    Widget::pipelineFlatPlaceVertices(vertices, offset);
}

HitBox WindowToolbarWidget::hitBoxTest(glm::vec2 position) const noexcept
{
    if (trafficLightButtons->box.contains(position)) {
        return trafficLightButtons->hitBoxTest(position);
    }

    for (auto& widget : children) {
        if (widget->box.contains(position)) {
            return widget->hitBoxTest(position);
        }
    }
    return HitBox::MoveArea;
}

}
