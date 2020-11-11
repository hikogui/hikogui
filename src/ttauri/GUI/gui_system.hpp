// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "gui_device.hpp"
#include "Window_base.hpp"
#include "Window_vulkan_win32.hpp"
#include "VerticalSync.hpp"
#include "gui_system_delegate.hpp"
#include "../unfair_recursive_mutex.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace tt {


/** Vulkan gui_device controller.
 * Manages Vulkan device and a set of Windows.
 */
class gui_system {
public:
    gui_system_delegate *delegate;

    std::unique_ptr<VerticalSync> verticalSync;

    //! List of all devices.
    std::vector<std::shared_ptr<gui_device>> devices;

    /*! Keep track of the numberOfWindows in the previous render cycle.
     * This way we can call closedLastWindow on the application once.
     */
    ssize_t previousNumberOfWindows = 0;

    gui_system(gui_system_delegate *delegate) noexcept :
        delegate(delegate)
    {
        verticalSync = std::make_unique<VerticalSync>(_handleVerticalSync, this);
    }

    virtual ~gui_system() {}

    gui_system(const gui_system &) = delete;
    gui_system &operator=(const gui_system &) = delete;
    gui_system(gui_system &&) = delete;
    gui_system &operator=(gui_system &&) = delete;

    /** Initialize after construction.
     * Call this function directly after the constructor on the same thread.
     */
    virtual void initialize() = 0;

    template<typename... Args>
    Window_base *makeWindow(Args &&... args)
    {
        // This function should be called from the main thread from the main loop,
        // and therefor should not have a lock on the window.
        tt_assert2(is_main_thread(), "createWindow should be called from the main thread.");
        tt_assume(gui_system_mutex.recurse_lock_count() == 0);

        auto window = std::make_shared<Window_vulkan_win32>(static_cast<gui_system &>(*this), std::forward<Args>(args)...);
        auto window_ptr = window.get();
        window->initialize();

        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto device = findBestDeviceForWindow(*window);
        if (!device) {
            TTAURI_THROW(gui_error("Could not find a vulkan-device matching this window"));
        }

        device->add(std::move(window));
        return window_ptr;
    }

    /*! Count the number of windows managed by the GUI.
     */
    ssize_t getNumberOfWindows();

    void render(hires_utc_clock::time_point displayTimePoint) {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        for (auto &device: devices) {
            device->render(displayTimePoint);
        }
        ttlet currentNumberOfWindows = getNumberOfWindows();
        if (currentNumberOfWindows == 0 && currentNumberOfWindows != previousNumberOfWindows) {
            application->run_from_main_loop([this]{
                this->delegate->last_window_closed();
            });
        }
        previousNumberOfWindows = currentNumberOfWindows;
    }

    void handleVerticalSync(hires_utc_clock::time_point displayTimePoint)
    {
        render(displayTimePoint);
    }


    static void _handleVerticalSync(void *data, hires_utc_clock::time_point displayTimePoint);

protected:
    gui_device *findBestDeviceForWindow(Window_base const &window);
};

}
