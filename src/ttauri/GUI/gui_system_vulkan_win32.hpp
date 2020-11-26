// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "gui_system_vulkan.hpp"
#include <span>

namespace tt {

class gui_system_vulkan_win32 final: public gui_system_vulkan {
public:
    gui_system_vulkan_win32(std::weak_ptr<gui_system_delegate> const &delegate);
    ~gui_system_vulkan_win32();

    gui_system_vulkan_win32(const gui_system_vulkan_win32 &) = delete;
    gui_system_vulkan_win32 &operator=(const gui_system_vulkan_win32 &) = delete;
    gui_system_vulkan_win32(gui_system_vulkan_win32 &&) = delete;
    gui_system_vulkan_win32 &operator=(gui_system_vulkan_win32 &&) = delete;

    vk::ResultValueType<vk::SurfaceKHR>::type createWin32SurfaceKHR(const vk::Win32SurfaceCreateInfoKHR& createInfo) const {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return intrinsic.createWin32SurfaceKHR(createInfo);
    }
};

}
