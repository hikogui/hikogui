// Copyright 2019 Pokitec
// All rights reserved.

#include "gui_system_vulkan_win32.hpp"
#include "Window.hpp"
#include <ddraw.h>

namespace tt {

using namespace std;

gui_system_vulkan_win32::gui_system_vulkan_win32(gui_system_delegate *delegate) :
    gui_system_vulkan(delegate, { VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
}

gui_system_vulkan_win32::~gui_system_vulkan_win32()
{
}


}
