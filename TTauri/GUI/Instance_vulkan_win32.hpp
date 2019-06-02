// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Instance_vulkan.hpp"
#include <gsl/gsl>

namespace TTauri::GUI {

class Instance_vulkan_win32 final: public Instance_vulkan {
public:
    Instance_vulkan_win32();
    ~Instance_vulkan_win32();

    Instance_vulkan_win32(const Instance_vulkan_win32 &) = delete;
    Instance_vulkan_win32 &operator=(const Instance_vulkan_win32 &) = delete;
    Instance_vulkan_win32(Instance_vulkan_win32 &&) = delete;
    Instance_vulkan_win32 &operator=(Instance_vulkan_win32 &&) = delete;

    void createWindow(std::shared_ptr<GUI::WindowDelegate> windowDelegate, const std::string &title) override;

    vk::ResultValueType<vk::SurfaceKHR>::type createWin32SurfaceKHR(const vk::Win32SurfaceCreateInfoKHR& createInfo) const {
        std::scoped_lock lock(TTauri::GUI::mutex);
        return intrinsic.createWin32SurfaceKHR(createInfo);
    }

private:
    std::thread maintanceThread;
    bool stopMaintenance = false;

    static void maintenanceLoop(gsl::not_null<Instance_vulkan_win32 *> self);
};

}
