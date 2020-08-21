// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "GUISystem_vulkan.hpp"
#include <span>

namespace tt {

class GUISystem_vulkan_macos final: public GUISystem_vulkan {
public:
    GUISystem_vulkan_macos(GUISystemDelegate *delegate);
    ~GUISystem_vulkan_macos();

    GUISystem_vulkan_macos(const GUISystem_vulkan_macos &) = delete;
    GUISystem_vulkan_macos &operator=(const GUISystem_vulkan_macos &) = delete;
    GUISystem_vulkan_macos(GUISystem_vulkan_macos &&) = delete;
    GUISystem_vulkan_macos &operator=(GUISystem_vulkan_macos &&) = delete;

    vk::ResultValueType<vk::SurfaceKHR>::type createMetalSurfaceKHR(const vk::MetalSurfaceCreateInfoEXT& createInfo) const {
        ttlet lock = std::scoped_lock(mutex);
        return intrinsic.createMetalSurfaceEXT(createInfo);
    }
};

}
