// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowWidget.hpp"

namespace TTauri::GUI {

using namespace std;

WindowWidget::WindowWidget(const std::weak_ptr<Window> window) :
    Widget()
{
    this->window = move(window);

    backgroundColor = wsRGBA{ 0x444f19ff };
}


}
