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

HitBox WindowWidget::hitBoxTest(vec position) const noexcept
{
    constexpr float BORDER_WIDTH = 5.0;

    if (position.x() <= (box.left.value() + BORDER_WIDTH)) {
        if (position.y() <= (box.bottom.value() + BORDER_WIDTH)) {
            return HitBox::BottomLeftResizeCorner;
        } else if (position.y() >= (box.top.evaluate() - BORDER_WIDTH)) {
            return HitBox::TopLeftResizeCorner;
        } else {
            return HitBox::LeftResizeBorder;
        }

    } else if (position.x() >= (box.right.evaluate() - BORDER_WIDTH)) {
        if (position.y() <= (box.bottom.value() + BORDER_WIDTH)) {
            return HitBox::BottomRightResizeCorner;
        } else if (position.y() >= (box.top.evaluate() - BORDER_WIDTH)) {
            return HitBox::TopRightResizeCorner;
        } else {
            return HitBox::RightResizeBorder;
        }

    } else if (position.y() <= (box.bottom.value() + BORDER_WIDTH)) {
        return HitBox::BottomResizeBorder;

    } else if (position.y() >= (box.top.evaluate() - BORDER_WIDTH)) {
        return HitBox::TopResizeBorder;

    } else if (toolbar->box.contains(position)) {
        // The toolbar will say HitBox::MoveArea where there are no widgets.
        return toolbar->hitBoxTest(position);

    } else {
        // Don't send hitbox tests to the rest of the widgets.
        return HitBox::NoWhereInteresting;
    }
}

}
