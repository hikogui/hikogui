// Copyright 2019 Pokitec
// All rights reserved.

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
