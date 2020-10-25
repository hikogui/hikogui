// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "GUISystem_vulkan.hpp"
#include <span>

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
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        return intrinsic.createWin32SurfaceKHR(createInfo);
    }
};

}
