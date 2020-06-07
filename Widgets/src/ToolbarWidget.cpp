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

    trafficLightButtons = &addWidget<WindowTrafficLightsWidget>();
    trafficLightButtons->placeAtTop(0.0f);
    trafficLightButtons->placeAtBottom(0.0f);
    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        trafficLightButtons->placeRight(0.0f);
    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        trafficLightButtons->placeLeft(0.0f);
    } else {
        no_default;
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
