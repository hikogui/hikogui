// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system_vulkan_win32.hpp"
#include <ddraw.h>

namespace tt {

using namespace std;

gui_system_vulkan_win32::gui_system_vulkan_win32(std::weak_ptr<gui_system_delegate> const &delegate, os_handle instance) :
    gui_system_vulkan(delegate, instance, { VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
{
}

gui_system_vulkan_win32::~gui_system_vulkan_win32()
{
}


}
