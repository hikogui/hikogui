//
//  Backing.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Backing.hpp"
#include "Window.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

Backing::Backing(std::weak_ptr<Window> window, float2 size) :
    window(window), size(size.round())
{
    auto _window = window.lock();
    device = _window->device;
}

Backing::~Backing()
{
}

}}}
