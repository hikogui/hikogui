// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/WindowWidget.hpp"
#include "TTauri/GUI/WindowTrafficLightsWidget.hpp"
#include "TTauri/GUI/WindowToolbarWidget.hpp"
#include "TTauri/GUI/utils.hpp"

namespace TTauri::GUI::Widgets {

using namespace std;

WindowWidget::WindowWidget() noexcept :
    Widget()
{
}

void WindowWidget::setParentWindow(gsl::not_null<Window *> window) noexcept
{
    this->window = window;

    toolbar = addWidget<WindowToolbarWidget>();
    window->addConstraint(toolbar->box.left == box.left);
    window->addConstraint(toolbar->box.right == box.right);
    window->addConstraint(toolbar->box.top == box.top);

    window->addConstraint(box.left == 0);
    window->addConstraint(box.bottom == 0);

    backgroundColor = vec{0.058, 0.078, 0.010, 1.0};
}

HitBox WindowWidget::hitBoxTest(vec position) noexcept
{
    constexpr float BORDER_WIDTH = 5.0;

    auto r = HitBox{this, depth};

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
