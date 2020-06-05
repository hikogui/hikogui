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
    window.addConstraint(height <= Theme::smallHeight, rhea::strength::strong());

    trafficLightButtons = &addWidget<WindowTrafficLightsWidget>(
        getResource<Path>(URL("resource:Themes/Icons/Application Icon.tticon"))
    );
    window.addConstraint(trafficLightButtons->top == top);
    window.addConstraint(trafficLightButtons->left == left);
    window.addConstraint(trafficLightButtons->bottom == bottom);

    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        closeWindowButton = &addWidget<ToolbarButtonWidget>(
            Text::TTauriIcon::CloseWindow,
            [&]() { window.closeWindow(); }
        );
        closeWindowButton->closeButton = true;
        window.addConstraint(closeWindowButton->top == top);
        window.addConstraint(closeWindowButton->right == right);
        window.addConstraint(closeWindowButton->bottom == bottom);

        maximizeWindowButton = &addWidget<ToolbarButtonWidget>(
            Text::TTauriIcon::MaximizeWindowMS,
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
        window.addConstraint(maximizeWindowButton->top == top);
        window.addConstraint(maximizeWindowButton->right == closeWindowButton->left);
        window.addConstraint(maximizeWindowButton->bottom == bottom);

        minimizeWindowButton = &addWidget<ToolbarButtonWidget>(
            Text::TTauriIcon::MinimizeWindow,
            //getResource<Path>(URL("resource:Themes/Icons/MultiColor.tticon")),
            [&]() { window.minimizeWindow(); }
        );
        window.addConstraint(minimizeWindowButton->top == top);
        window.addConstraint(minimizeWindowButton->right == maximizeWindowButton->left);
        window.addConstraint(minimizeWindowButton->bottom == bottom);
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
