// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/GUISystem_vulkan.hpp"
#include <nonstd/span>

namespace tt {

class GUISystem_vulkan_macos final: public GUISystem_vulkan {
public:
    GUISystem_vulkan_macos(GUISystemDelegate *delegate);
    ~GUISystem_vulkan_macos();

    GUISystem_vulkan_macos(const GUISystem_vulkan_macos &) = delete;
    GUISystem_vulkan_macos &operator=(const GUISystem_vulkan_macos &) = delete;
    GUISystem_vulkan_macos(GUISystem_vulkan_macos &&) = delete;
    GUISystem_vulkan_macos &operator=(GUISystem_vulkan_macos &&) = delete;

    vk::ResultValueType<vk::SurfaceKHR>::type createWin32SurfaceKHR(const vk::MetalSurfaceCreateInfoEXT& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createMetalSurfaceEXT(createInfo);
    }
};

}
