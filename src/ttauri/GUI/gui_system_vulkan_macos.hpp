// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_system_vulkan.hpp"
#include <span>

namespace tt {

class gui_system_vulkan_macos final: public gui_system_vulkan {
public:
    gui_system_vulkan_macos(gui_system_delegate *delegate);
    ~gui_system_vulkan_macos();

    gui_system_vulkan_macos(const gui_system_vulkan_macos &) = delete;
    gui_system_vulkan_macos &operator=(const gui_system_vulkan_macos &) = delete;
    gui_system_vulkan_macos(gui_system_vulkan_macos &&) = delete;
    gui_system_vulkan_macos &operator=(gui_system_vulkan_macos &&) = delete;

    vk::ResultValueType<vk::SurfaceKHR>::type createMetalSurfaceKHR(const vk::MetalSurfaceCreateInfoEXT& createInfo) const {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return intrinsic.createMetalSurfaceEXT(createInfo);
    }
};

}
