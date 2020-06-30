#include "GUISystem_vulkan_macos.hpp"
#include "Window.hpp"

namespace tt {

GUISystem_vulkan_macos::GUISystem_vulkan_macos(GUISystemDelegate *delegate) :
    GUISystem_vulkan(delegate, { VK_MVK_MACOS_SURFACE_EXTENSION_NAME })
{
    hostFrequency = numeric_cast<uint64_t>(CVGetHostClockFrequency());

    // Start update loop.
    CVDisplayLinkCreateWithActiveCGDisplays(&updateAndRenderThread);
    CVDisplayLinkSetOutputCallback(updateAndRenderThread, &GUISystem_vulkan_win32::updateAndRenderLoop, static_cast<void *>(this));
    CVDisplayLinkStart(updateAndRenderThread);
}

GUISystem_vulkan_macos::~GUISystem_vulkan_macos()
{
    stopUpdateAndRender = true;
    updateAndRenderThread.join();
}


std::shared_ptr<Window> GUISystem_vulkan_win32::createWindow(std::shared_ptr<Window::Delegate> windowDelegate, const std::string &title)
{
    std::scoped_lock lock(tt::mutex);

    auto window = std::make_shared<Window_win32>(windowDelegate, title);
    getShared<GUISystem>()->add(window);
    window->initialize();
    return window;
}

CVReturn GUISystem_vulkan_win32::updateAndRenderLoop(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* target)
{
    scoped_lock lock(tt::mutex);

    auto self = static_cast<GUISystem_vulkan_win32 *>(target);

    auto currentHostTime = static_cast<ubig128>(now->hostTime);
    currentHostTime *= 1000000000;
    currentHostTime /= self->hostFrequency;

    auto outputHostTime = static_cast<ubig128>(outputTime->hostTime);
    outputHostTime *= 1000000000;
    outputHostTime /= self->hostFrequency;

    @autoreleasepool {
        self->updateAndRender(numeric_cast<uint64_t>(currentHostTime), numeric_cast<uint64_t>(outputHostTime), false);
    }

    return kCVReturnSuccess;
}

}
