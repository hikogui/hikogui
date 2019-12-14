// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Pipeline_base.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/GUI/Window.hpp"

namespace TTauri::GUI {

using namespace std;

Pipeline_base::Pipeline_base(Window const &window) :
    window(window)
{
}

Device const &Pipeline_base::device() const
{
    let device = window.device;
    ttauri_assert(device);
    return *device;
}


}
