// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../utility/win32_headers.hpp"

#include "gfx_system_vulkan.hpp"
#include "gfx_surface_vulkan.hpp"

namespace hi::inline v1 {

[[nodiscard]] std::unique_ptr<gfx_surface> gfx_system_vulkan::make_surface(os_handle instance, void *os_window) const noexcept
{
    hilet lock = std::scoped_lock(gfx_system_mutex);

    auto surface_create_info = vk::Win32SurfaceCreateInfoKHR{
        vk::Win32SurfaceCreateFlagsKHR(), reinterpret_cast<HINSTANCE>(instance), reinterpret_cast<HWND>(os_window)};

    auto vulkan_surface = intrinsic.createWin32SurfaceKHR(surface_create_info);

    auto ptr = std::make_unique<gfx_surface_vulkan>(*const_cast<gfx_system_vulkan *>(this), vulkan_surface);
    ptr->init();
    return ptr;
}

} // namespace hi::inline v1
