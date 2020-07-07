// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/GUI/GUISystem_vulkan.hpp"
#include <nonstd/span>

namespace tt {

class GUISystem_vulkan_win32 final: public GUISystem_vulkan {
public:
    GUISystem_vulkan_win32(GUISystemDelegate *delegate);
    ~GUISystem_vulkan_win32();

    GUISystem_vulkan_win32(const GUISystem_vulkan_win32 &) = delete;
    GUISystem_vulkan_win32 &operator=(const GUISystem_vulkan_win32 &) = delete;
    GUISystem_vulkan_win32(GUISystem_vulkan_win32 &&) = delete;
    GUISystem_vulkan_win32 &operator=(GUISystem_vulkan_win32 &&) = delete;

    vk::ResultValueType<vk::SurfaceKHR>::type createWin32SurfaceKHR(const vk::Win32SurfaceCreateInfoKHR& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createWin32SurfaceKHR(createInfo);
    }
};

}
