// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowWidget.hpp"
#include "ToolbarWidget.hpp"
#include "Window.hpp"

namespace TTauri::GUI {

using namespace std;

WindowWidget::WindowWidget() :
    Widget()
{
}

void WindowWidget::setParent(Window *window)
{
    this->window = window;

    auto _toolbar = make_shared<ToolbarWidget>();
    add(_toolbar);
    toolbar = _toolbar.get();

    window->addConstraint(box.left == 0);
    window->addConstraint(box.bottom == 0);

    backgroundColor = wsRGBA{ 0x444f19ff };
}

HitBox WindowWidget::hitBoxTest(glm::vec2 position) const
{
    constexpr float BORDER_WIDTH = 5.0;

    if (position.x <= (box.left.value() + BORDER_WIDTH)) {
        if (position.y <= (box.bottom.value() + BORDER_WIDTH)) {
            return HitBox::BottomLeftResizeCorner;
        } else if (position.y >= (box.top().evaluate() - BORDER_WIDTH)) {
            return HitBox::TopLeftResizeCorner;
        } else {
            return HitBox::LeftResizeBorder;
        }

    } else if (position.x >= (box.right().evaluate() - BORDER_WIDTH)) {
        if (position.y <= (box.bottom.value() + BORDER_WIDTH)) {
            return HitBox::BottomRightResizeCorner;
        } else if (position.y >= (box.top().evaluate() - BORDER_WIDTH)) {
            return HitBox::TopRightResizeCorner;
        } else {
            return HitBox::RightResizeBorder;
        }

    } else if (position.y <= (box.bottom.value() + BORDER_WIDTH)) {
        return HitBox::BottomResizeBorder;

    } else if (position.y >= (box.top().evaluate() - BORDER_WIDTH)) {
        return HitBox::TopResizeBorder;

    } else if (toolbar->box.contains(position)) {
        // The toolbar will say HitBox::MoveArea where there are no widgets.
        return toolbar->hitBoxTest(position);
    }

    // Don't send hitbox tests to the rest of the widgets.
    return HitBox::NoWhereInteresting;
}

}
