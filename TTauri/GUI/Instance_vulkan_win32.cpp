// Copyright 2019 Pokitec
// All rights reserved.

#include "Instance_vulkan_win32.hpp"
#include "Window.hpp"

#include <ddraw.h>

namespace TTauri::GUI {

using namespace std;
using namespace gsl;

Instance_vulkan_win32::Instance_vulkan_win32() :
    Instance_vulkan({ VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
{
    // Start update loop.
    maintanceThread = thread(Instance_vulkan_win32::maintenanceLoop, not_null<Instance_vulkan_win32 *>(this));
}

Instance_vulkan_win32::~Instance_vulkan_win32()
{
    try {
        [[gsl::suppress(f.6)]] {
            stopMaintenance = true;

            maintanceThread.join();
        }
    } catch (...) {
        abort();
    }
}

void Instance_vulkan_win32::createWindow(std::shared_ptr<GUI::WindowDelegate> windowDelegate, const std::string &title)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto window = TTauri::make_shared<GUI::Window>(windowDelegate, title);
    add(window);
}

void Instance_vulkan_win32::maintenanceLoop(gsl::not_null<Instance_vulkan_win32 *> self)
{
    auto threadID = GetCurrentThread();
    SetThreadDescription(threadID, L"TTauri::GUI Maintenance");

    while (!self->stopMaintenance) {
        self->maintenance();

        std::this_thread::sleep_for(50ms);
    }
}


}
