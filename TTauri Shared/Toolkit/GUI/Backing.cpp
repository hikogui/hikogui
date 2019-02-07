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

Backing::Backing(Window *window, VkExtent2D size) :
    window(window), size(size)
{
    instance = window->instance;
}

Backing::~Backing()
{
}

}}}
