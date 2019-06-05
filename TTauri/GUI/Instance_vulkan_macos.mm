#include "Instance_vulkan_macos.hpp"
#include "Window.hpp"

namespace TTauri {
namespace GUI {

Instance_vulkan_macos::Instance_vulkan_macos() :
    Instance_vulkan({ VK_MVK_MACOS_SURFACE_EXTENSION_NAME })
{
    hostFrequency = boost::numeric_cast<uint64_t>(CVGetHostClockFrequency());

    // Start update loop.
    CVDisplayLinkCreateWithActiveCGDisplays(&updateAndRenderThread);
    CVDisplayLinkSetOutputCallback(updateAndRenderThread, &Instance_vulkan_win32::updateAndRenderLoop, static_cast<void *>(this));
    CVDisplayLinkStart(updateAndRenderThread);
}

Instance_vulkan_macos::~Instance_vulkan_macos()
{
    stopUpdateAndRender = true;
    updateAndRenderThread.join();
}


std::shared_ptr<GUI::Window> Instance_vulkan_win32::createWindow(std::shared_ptr<GUI::Window::Delegate> windowDelegate, const std::string &title)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto window = std::make_shared<GUI::Window_win32>(windowDelegate, title);
    getShared<Instance>()->add(window);
    window->initialize();
    return window;
}

CVReturn Instance_vulkan_win32::updateAndRenderLoop(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* target)
{
    scoped_lock lock(TTauri::GUI::mutex);

    auto self = static_cast<Instance_vulkan_win32 *>(target);

    auto currentHostTime = static_cast<__uint128_t>(now->hostTime);
    currentHostTime *= 1000000000;
    currentHostTime /= self->hostFrequency;

    auto outputHostTime = static_cast<__uint128_t>(outputTime->hostTime);
    outputHostTime *= 1000000000;
    outputHostTime /= self->hostFrequency;

    @autoreleasepool {
        self->updateAndRender(boost::numeric_cast<uint64_t>(currentHostTime), boost::numeric_cast<uint64_t>(outputHostTime), false);
    }

    return kCVReturnSuccess;
}

}}
