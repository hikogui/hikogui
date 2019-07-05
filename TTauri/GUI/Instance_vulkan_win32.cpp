// Copyright 2019 Pokitec
// All rights reserved.

#include "Instance_vulkan_win32.hpp"
#include "Window.hpp"

#include <ddraw.h>

namespace TTauri::GUI {

using namespace std;
using namespace gsl;

Instance_vulkan_win32::Instance_vulkan_win32() :
    Instance_vulkan({ VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
{
}

Instance_vulkan_win32::~Instance_vulkan_win32()
{
}


}
