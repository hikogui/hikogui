// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/WindowWidget.hpp"
#include "TTauri/Widgets/WindowTrafficLightsWidget.hpp"
#include "TTauri/Widgets/ToolbarWidget.hpp"
#include "TTauri/Widgets/ContainerWidget.hpp"
#include "TTauri/GUI/utils.hpp"

namespace TTauri {

using namespace std;

WindowWidget::WindowWidget(Window &window) noexcept :
    Widget(window, nullptr, vec{0.0, 0.0})
{
    toolbar = &makeWidgetDirectly<ToolbarWidget>();
    toolbar->placeLeft(0.0f);
    toolbar->placeRight(0.0f);
    toolbar->placeAtTop(0.0f);

    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
        toolbar->makeAlignedWidget<WindowTrafficLightsWidget>(Alignment::TopRight);
    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        toolbar->makeAlignedWidget<WindowTrafficLightsWidget>(Alignment::TopLeft);
    } else {
        no_default;
    }


    content = &makeWidgetDirectly<ContainerWidget>();
    content->elevation = elevation;
    content->placeLeft(0.0f);
    content->placeRight(0.0f);
    content->placeAtBottom(0.0f);
    content->placeBelow(*toolbar, 0.0f);

    // Add constraints for the window widget itself.
    window.addConstraint(left == 0);
    window.addConstraint(bottom == 0);
    // A upper bound constraint is needed to allow the suggest(width, limit::max()) and suggest(height, limit::max()) to
    // fallback on a upper bound, otherwise it will select the lower bounds instead.
    window.addConstraint(width <= std::numeric_limits<uint16_t>::max());
    window.addConstraint(height <= std::numeric_limits<uint16_t>::max());
}

HitBox WindowWidget::hitBoxTest(vec position) const noexcept
{
    constexpr float BORDER_WIDTH = 5.0;

    auto r = HitBox{this, elevation};

    if (position.x() <= BORDER_WIDTH) {
        if (position.y() <= BORDER_WIDTH) {
            r.type = HitBox::Type::BottomLeftResizeCorner;
        } else if (position.y() >= (rectangle().height() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopLeftResizeCorner;
        } else {
            r.type = HitBox::Type::LeftResizeBorder;
        }

    } else if (position.x() >= (rectangle().width() - BORDER_WIDTH)) {
        if (position.y() <= BORDER_WIDTH) {
            r.type = HitBox::Type::BottomRightResizeCorner;
        } else if (position.y() >= (rectangle().height() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopRightResizeCorner;
        } else {
            r.type = HitBox::Type::RightResizeBorder;
        }

    } else if (position.y() <= BORDER_WIDTH) {
        r.type = HitBox::Type::BottomResizeBorder;

    } else if (position.y() >= (rectangle().height() - BORDER_WIDTH)) {
        r.type = HitBox::Type::TopResizeBorder;
    }

    if (r.type != HitBox::Type::Outside) {
        // Resize corners need to override anything else, so that it is
        // always possible to resize a window.
        return r;
    }

    for (let &child : children) {
        r = std::max(r, child->hitBoxTest(position - child->offsetFromParent()));
    }

    return r;
}

}
