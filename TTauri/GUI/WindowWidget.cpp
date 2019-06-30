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

}
