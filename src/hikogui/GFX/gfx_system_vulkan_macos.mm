//#include "gui_system_vulkan_macos.hpp"
//#include "Window.hpp"
//
//namespace hi::inline v1 {
//
//gui_system_vulkan_macos::gui_system_vulkan_macos(gui_systemDelegate *delegate) :
//    gui_system_vulkan(delegate, { VK_EXT_METAL_SURFACE_EXTENSION_NAME })
//{
//    hostFrequency = narrow_cast<uint64_t>(CVGetHostClockFrequency());
//
//    // Start update loop.
//    CVDisplayLinkCreateWithActi32x4GDisplays(&updateAndRenderThread);
//    CVDisplayLinkSetOutputCallback(updateAndRenderThread, &gui_system_vulkan_win32::updateAndRenderLoop, static_cast<void *>(this));
//    CVDisplayLinkStart(updateAndRenderThread);
//}
//
//gui_system_vulkan_macos::~gui_system_vulkan_macos()
//{
//    stopUpdateAndRender = true;
//    updateAndRenderThread.join();
//}
//
//
//std::shared_ptr<Window> gui_system_vulkan_win32::createWindow(std::shared_ptr<Window::Delegate> windowDelegate, const std::string /&title)
//{
//    std::scoped_lock lock(hi::mutex);
//
//    auto window = std::make_shared<gui_window_win32>(windowDelegate, title);
//    getShared<gui_system>()->add(window);
//    window->init();
//    return window;
//}
//
//CVReturn gui_system_vulkan_win32::updateAndRenderLoop(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* //outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* target)
//{
//    scoped_lock lock(hi::mutex);
//
//    auto self = static_cast<gui_system_vulkan_win32 *>(target);
//
//    auto currentHostTime = static_cast<ubig128>(now->hostTime);
//    currentHostTime *= 1000000000;
//    currentHostTime /= self->hostFrequency;
//
//    auto outputHostTime = static_cast<ubig128>(outputTime->hostTime);
//    outputHostTime *= 1000000000;
//    outputHostTime /= self->hostFrequency;
//
//    @autoreleasepool {
//        self->updateAndRender(narrow_cast<uint64_t>(currentHostTime), narrow_cast<uint64_t>(outputHostTime), false);
//    }
//
//    return kCVReturnSuccess;
//}
//
//}
//