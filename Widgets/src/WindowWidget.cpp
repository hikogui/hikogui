// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/WindowWidget.hpp"
#include "TTauri/Widgets/WindowTrafficLightsWidget.hpp"
#include "TTauri/Widgets/WindowToolbarWidget.hpp"
#include "TTauri/Widgets/ContainerWidget.hpp"
#include "TTauri/GUI/utils.hpp"

namespace TTauri::GUI::Widgets {

using namespace std;

WindowWidget::WindowWidget(Window &window) noexcept :
    Widget(window, nullptr, vec{0.0, 0.0})
{
    toolbar = &addWidgetDirectly<WindowToolbarWidget>();
    window.addConstraint(toolbar->box.left == box.left);
    window.addConstraint(toolbar->box.right == box.right);
    window.addConstraint(toolbar->box.top == box.top);

    content = &addWidgetDirectly<ContainerWidget>();
    window.addConstraint(content->box.left == box.left);
    window.addConstraint(content->box.right == box.right);
    window.addConstraint(content->box.top == toolbar->box.bottom);
    window.addConstraint(content->box.bottom == box.bottom);

    // Add constraints for the window widget itself.
    window.addConstraint(box.left == 0);
    window.addConstraint(box.bottom == 0);
    // A upper bound constraint is needed to allow the suggest(width, limit::max()) and suggest(height, limit::max()) to
    // fallback on a upper bound, otherwise it will select the lower bounds instead.
    window.addConstraint(box.width <= std::numeric_limits<uint16_t>::max());
    window.addConstraint(box.height <= std::numeric_limits<uint16_t>::max());

}

HitBox WindowWidget::hitBoxTest(vec position) noexcept
{
    constexpr float BORDER_WIDTH = 5.0;

    auto r = HitBox{this, elevation};

    if (position.x() <= (box.left.value() + BORDER_WIDTH)) {
        if (position.y() <= (box.bottom.value() + BORDER_WIDTH)) {
            r.type = HitBox::Type::BottomLeftResizeCorner;
        } else if (position.y() >= (box.top.evaluate() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopLeftResizeCorner;
        } else {
            r.type = HitBox::Type::LeftResizeBorder;
        }

    } else if (position.x() >= (box.right.evaluate() - BORDER_WIDTH)) {
        if (position.y() <= (box.bottom.value() + BORDER_WIDTH)) {
            r.type = HitBox::Type::BottomRightResizeCorner;
        } else if (position.y() >= (box.top.evaluate() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopRightResizeCorner;
        } else {
            r.type = HitBox::Type::RightResizeBorder;
        }

    } else if (position.y() <= (box.bottom.value() + BORDER_WIDTH)) {
        r.type = HitBox::Type::BottomResizeBorder;

    } else if (position.y() >= (box.top.evaluate() - BORDER_WIDTH)) {
        r.type = HitBox::Type::TopResizeBorder;
    }

    if (r.type != HitBox::Type::Outside) {
        // Resize corners need to override anything else, so that it is
        // always possible to resize a window.
        return r;
    }

    r = std::max(r, toolbar->hitBoxTest(position));
    for (auto& widget : children) {
        r = std::max(r, widget->hitBoxTest(position));
    }

    return r;
}

}
