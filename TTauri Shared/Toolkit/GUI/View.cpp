//
//  Frame.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "View.hpp"
#include "Window.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

View::View(std::weak_ptr<Window> window) :
    window(window)
{
    const auto _window = window.lock();
    device = _window->device;
}

View::View(std::weak_ptr<View> parent) :
    parent(parent)
{
    const auto _parent = parent.lock();
    window = _parent->window;
    device = _parent->device;
}

View::~View()
{
}

}}}
