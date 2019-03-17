
#include "Application_win32.hpp"

#include "GUI/Instance.hpp"
#include "GUI/Window_win32.hpp"

#include <vulkan/vulkan.hpp>

#include <thread>

namespace TTauri {

Application_win32::Application_win32(std::shared_ptr<Delegate> delegate, HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) :
    Application(delegate),
    hInstance(hInstance),
    hPrevInstance(hPrevInstance),
    pCmdLine(pCmdLine),
    nCmdShow(nCmdShow)
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
}

Application_win32::~Application_win32()
{
}

int Application_win32::loop()
{
    startingLoop();

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
}
