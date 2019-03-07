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
namespace GUI {

View::View(Window *window) :
    window(window)
{
    instance = window->instance;
}

View::View(View *parent) :
    parent(parent)
{
    window = parent->window;
    instance = parent->instance;
}

View::~View()
{
}

}}
