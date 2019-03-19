#pragma once

#include "Instance_vulkan.hpp"


namespace TTauri {
namespace GUI {

class Instance_vulkan_win32 : public Instance_vulkan {
public:
    std::thread updateAndRenderThread;
    bool stopUpdateAndRender = false;

    Instance_vulkan_win32();
    ~Instance_vulkan_win32();

    Instance_vulkan_win32(const Instance_vulkan_win32 &) = delete;
    Instance_vulkan_win32 &operator=(const Instance_vulkan_win32 &) = delete;
    Instance_vulkan_win32(Instance_vulkan_win32 &&) = delete;
    Instance_vulkan_win32 &operator=(Instance_vulkan_win32 &&) = delete;

    void createWindow(std::shared_ptr<GUI::Window::Delegate> windowDelegate, const std::string &title) override;

    static void updateAndRenderLoop(Instance_vulkan_win32 *self);
};

}}
