// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system_vulkan_win32.hpp"
#include "gui_surface_vulkan.hpp"
#include <ddraw.h>

namespace tt {

using namespace std;

gui_system_vulkan_win32::gui_system_vulkan_win32(std::weak_ptr<gui_system_delegate> const &delegate, os_handle instance) :
    gui_system_vulkan(delegate, instance, {VK_KHR_WIN32_SURFACE_EXTENSION_NAME})
{
}

gui_system_vulkan_win32::~gui_system_vulkan_win32() {}

[[nodiscard]] std::unique_ptr<gui_surface> gui_system_vulkan_win32::make_surface(void *os_window) const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    auto surface_create_info = vk::Win32SurfaceCreateInfoKHR{
        vk::Win32SurfaceCreateFlagsKHR(), reinterpret_cast<HINSTANCE>(instance), reinterpret_cast<HWND>(os_window)};

    auto vulkan_surface = intrinsic.createWin32SurfaceKHR(surface_create_info);

    return std::make_unique<gui_surface_vulkan>(*const_cast<gui_system_vulkan_win32*>(this), vulkan_surface);
}

} // namespace tt
