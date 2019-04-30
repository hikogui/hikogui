// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Instance_vulkan.hpp"
#include <gsl/gsl>

namespace TTauri::GUI {

class Instance_vulkan_win32 : public Instance_vulkan {
public:
    Instance_vulkan_win32();
    ~Instance_vulkan_win32();

    Instance_vulkan_win32(const Instance_vulkan_win32 &) = delete;
    Instance_vulkan_win32 &operator=(const Instance_vulkan_win32 &) = delete;
    Instance_vulkan_win32(Instance_vulkan_win32 &&) = delete;
    Instance_vulkan_win32 &operator=(Instance_vulkan_win32 &&) = delete;

    void createWindow(std::shared_ptr<GUI::Window::Delegate> windowDelegate, const std::string &title) override;

private:
    std::thread updateAndRenderThread;
    bool stopUpdateAndRender = false;

    std::thread maintanceThread;
    bool stopMaintenance = false;

    static void maintenanceLoop(gsl::not_null<Instance_vulkan_win32 *> self);
    static void updateAndRenderLoop(gsl::not_null<Instance_vulkan_win32 *> self);
};

}
