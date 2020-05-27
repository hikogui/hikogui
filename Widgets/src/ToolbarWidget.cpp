// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ToolbarWidget.hpp"
#include "TTauri/Widgets/WindowTrafficLightsWidget.hpp"
#include "TTauri/Widgets/ToolbarButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include <cmath>

namespace TTauri::GUI::Widgets {

using namespace std;

ToolbarWidget::ToolbarWidget(Window &window, Widget *parent) noexcept :
    Widget(window, parent, vec{Theme::width, Theme::smallHeight})
{
    window.addConstraint(box.height <= Theme::smallHeight, rhea::strength::strong());

    trafficLightButtons = &addWidget<WindowTrafficLightsWidget>(
        getResource<Path>(URL("resource:Themes/Icons/Application Icon.tticon"))
    );
    window.addConstraint(trafficLightButtons->box.top == box.top);
    window.addConstraint(trafficLightButtons->box.left == box.left);
    window.addConstraint(trafficLightButtons->box.bottom == box.bottom);

    if constexpr (operatingSystem == OperatingSystem::Windows) {
        closeWindowButton = &addWidget<ToolbarButtonWidget>(
            Text::TTauriIcon::CloseWindow,
            [&]() { window.closeWindow(); }
        );
        closeWindowButton->closeButton = true;
        window.addConstraint(closeWindowButton->box.top == box.top);
        window.addConstraint(closeWindowButton->box.right == box.right);
        window.addConstraint(closeWindowButton->box.bottom == box.bottom);

        maximizeWindowButton = &addWidget<ToolbarButtonWidget>(
            Text::TTauriIcon::MaximizeWindow,
            [&]() { 
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
        );
        window.addConstraint(maximizeWindowButton->box.top == box.top);
        window.addConstraint(maximizeWindowButton->box.right == closeWindowButton->box.left);
        window.addConstraint(maximizeWindowButton->box.bottom == box.bottom);

        minimizeWindowButton = &addWidget<ToolbarButtonWidget>(
            Text::TTauriIcon::MinimizeWindow,
            //getResource<Path>(URL("resource:Themes/Icons/MultiColor.tticon")),
            [&]() { window.minimizeWindow(); }
        );
        window.addConstraint(minimizeWindowButton->box.top == box.top);
        window.addConstraint(minimizeWindowButton->box.right == maximizeWindowButton->box.left);
        window.addConstraint(minimizeWindowButton->box.bottom == box.bottom);
    }
}

void ToolbarWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto context = drawContext;

    context.drawFilledQuad(rectangle());

    Widget::draw(drawContext, displayTimePoint);
}

HitBox ToolbarWidget::hitBoxTest(vec position) const noexcept
{
    auto r = rectangle().contains(position) ?
        HitBox{this, elevation, HitBox::Type::MoveArea} :
        HitBox{};

    for (let &child : children) {
        r = std::max(r, child->hitBoxTest(position - child->offsetFromParent()));
    }
    return r;
}

}
