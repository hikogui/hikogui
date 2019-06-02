// Copyright 2019 Pokitec
// All rights reserved.

#include "Pipeline_base.hpp"
#include "Device.hpp"
#include "Window.hpp"

namespace TTauri::GUI {

using namespace std;

Pipeline_base::Pipeline_base(const std::shared_ptr<Window> window) :
    window(std::move(window))
{
}

std::shared_ptr<Device> Pipeline_base::device() const {
    return window.lock()->device.lock();
}

}
