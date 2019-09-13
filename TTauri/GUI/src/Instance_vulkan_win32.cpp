// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Instance_vulkan_win32.hpp"
#include "TTauri/GUI/Window.hpp"
#include <ddraw.h>

namespace TTauri::GUI {

using namespace std;
using namespace gsl;

Instance_vulkan_win32::Instance_vulkan_win32(InstanceDelegate *delegate) :
    Instance_vulkan(delegate, { VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
{
}

Instance_vulkan_win32::~Instance_vulkan_win32()
{
}


}
