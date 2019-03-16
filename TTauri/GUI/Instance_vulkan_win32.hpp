#pragma once

#include "Instance_vulkan.hpp"

namespace TTauri {
namespace GUI {

class Instance_vulkan_win32 : public Instance_vulkan {
public:
    std::thread updateAndRenderThread;
    bool stopUpdateAndRender = false;

    Instance_vulkan_win32();
    virtual ~Instance_vulkan_win32();

    virtual std::shared_ptr<GUI::Window> createWindow(std::shared_ptr<GUI::Window::Delegate> windowDelegate, const std::string &title);

    static void updateAndRenderLoop(Instance_vulkan_win32 *self);
};

}}
