// Copyright 2019 Pokitec
// All rights reserved.

#include "GUISystem_vulkan_win32.hpp"
#include "Window.hpp"
#include <ddraw.h>

namespace tt {

using namespace std;

GUISystem_vulkan_win32::GUISystem_vulkan_win32(gui_system_delegate *delegate) :
    GUISystem_vulkan(delegate, { VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
}

GUISystem_vulkan_win32::~GUISystem_vulkan_win32()
{
}


}
