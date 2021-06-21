// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_system_vulkan_win32.hpp"
#include "gfx_surface_vulkan.hpp"
#include <ddraw.h>

namespace tt {

using namespace std;

gfx_system_vulkan_win32::gfx_system_vulkan_win32(std::weak_ptr<gfx_system_delegate> const &delegate, os_handle instance) :
    gfx_system_vulkan(delegate, instance, {VK_KHR_WIN32_SURFACE_EXTENSION_NAME})
{
}

gfx_system_vulkan_win32::~gfx_system_vulkan_win32() {}

[[nodiscard]] std::unique_ptr<gfx_surface> gfx_system_vulkan_win32::make_surface(void *os_window) const noexcept
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);

    auto surface_create_info = vk::Win32SurfaceCreateInfoKHR{
        vk::Win32SurfaceCreateFlagsKHR(), reinterpret_cast<HINSTANCE>(instance), reinterpret_cast<HWND>(os_window)};

    auto vulkan_surface = intrinsic.createWin32SurfaceKHR(surface_create_info);

    auto ptr = std::make_unique<gfx_surface_vulkan>(*const_cast<gfx_system_vulkan_win32*>(this), vulkan_surface);
    ptr->init();
    return ptr;
}

} // namespace tt
