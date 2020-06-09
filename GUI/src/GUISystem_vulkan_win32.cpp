// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/GUISystem_vulkan_win32.hpp"
#include "TTauri/GUI/Window.hpp"
#include <ddraw.h>

namespace TTauri {

using namespace std;

GUISystem_vulkan_win32::GUISystem_vulkan_win32(GUISystemDelegate *delegate) :
    GUISystem_vulkan(delegate, { VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
}

GUISystem_vulkan_win32::~GUISystem_vulkan_win32()
{
}


}
