
#include "Logging.hpp"
#include "Application_win32.hpp"

#include "GUI/Window_win32.hpp"

#include <vulkan/vulkan.hpp>
#include <thread>

namespace TTauri {

static void redrawLoop(Application_win32 *self)
{
    while (true) {
        self->instance->updateAndRender(0, 0, true);
    }
}

Application_win32::Application_win32(std::shared_ptr<Delegate> delegate, HINSTANCE win32Instance, PWSTR commandLine, int win32Show) :
    Application(delegate, { VK_KHR_WIN32_SURFACE_EXTENSION_NAME }),
    win32Instance(win32Instance),
    win32Show(win32Show)
{
    // Resource path, is the same directory as where the executable lives.
    wchar_t modulePathWChar[MAX_PATH];
    auto r = GetModuleFileNameW(NULL, modulePathWChar, MAX_PATH);
    if (r == 0 || r == MAX_PATH) {
        LOG_FATAL("Could not retrieve filename of executable.");
        abort();
    }

    auto modulePath = boost::filesystem::path(modulePathWChar);
    resourceDir = modulePath.parent_path();

    // Start update loop.
    redrawThread = std::make_shared<std::thread>(redrawLoop, this);
}

Application_win32::~Application_win32()
{
}

std::shared_ptr<Window> Application_win32::createWindow(std::shared_ptr<Window::Delegate> windowDelegate, const std::string &title)
{
    auto window = std::make_shared<Window_win32>(instance.get(), windowDelegate, title, win32Show);
    if (!instance->add(window)) {
        LOG_FATAL("Could not open window.");
        abort();
    }
    window->initialize();
    return window;
}

int Application_win32::loop()
{
    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

}
