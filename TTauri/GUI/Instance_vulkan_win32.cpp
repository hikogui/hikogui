#include "Instance_vulkan_win32.hpp"
#include "Window_vulkan_win32.hpp"

namespace TTauri {
namespace GUI {

Instance_vulkan_win32::Instance_vulkan_win32() :
    Instance_vulkan({ VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
{
    // Start update loop.
    updateAndRenderThread = std::thread(Instance_vulkan_win32::updateAndRenderLoop, this);
}

Instance_vulkan_win32::~Instance_vulkan_win32()
{
    stopUpdateAndRender = true;
    updateAndRenderThread.join();
}


void Instance_vulkan_win32::createWindow(std::shared_ptr<GUI::Window::Delegate> windowDelegate, const std::string &title)
{
    std::scoped_lock lock(mutex);

    auto window = std::make_shared<GUI::Window_vulkan_win32>(windowDelegate, title);
    getShared<Instance>()->add(window);
    window->initialize();
}

void Instance_vulkan_win32::updateAndRenderLoop(Instance_vulkan_win32 *self)
{
    while (!self->stopUpdateAndRender) {
        self->updateAndRender(0, 0, true);
    }
}

}}
