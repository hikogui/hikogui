// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_system_vulkan.hpp"
#include <span>

namespace hi::inline v1 {

class gfx_system_vulkan_macos final : public gfx_system_vulkan {
public:
    gfx_system_vulkan_macos(gui_system_delegate *delegate);
    ~gfx_system_vulkan_macos();

    gfx_system_vulkan_macos(const gfx_system_vulkan_macos &) = delete;
    gfx_system_vulkan_macos &operator=(const gfx_system_vulkan_macos &) = delete;
    gfx_system_vulkan_macos(gfx_system_vulkan_macos &&) = delete;
    gfx_system_vulkan_macos &operator=(gfx_system_vulkan_macos &&) = delete;

    vk::ResultValueType<vk::SurfaceKHR>::type createMetalSurfaceKHR(const vk::MetalSurfaceCreateInfoEXT &createInfo) const
    {
        hi_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createMetalSurfaceEXT(createInfo);
    }
};

} // namespace hi::inline v1
