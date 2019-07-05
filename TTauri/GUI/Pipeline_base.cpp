// Copyright 2019 Pokitec
// All rights reserved.

#include "Pipeline_base.hpp"
#include "Device.hpp"
#include "Window.hpp"

namespace TTauri::GUI {

using namespace std;

Pipeline_base::Pipeline_base(Window const &window) :
    window(window)
{
}

Device const &Pipeline_base::device() const
{
    let device = window.device;
    required_assert(device);
    return *device;
}


}
