// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_device.hpp"
#include "gui_window.hpp"
#include "gui_window_vulkan_win32.hpp"
#include "vertical_sync.hpp"
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
    static inline std::unique_ptr<gui_system> global;

    std::weak_ptr<gui_system_delegate> delegate;

    std::unique_ptr<vertical_sync> verticalSync;

    //! List of all devices.
    std::vector<std::shared_ptr<gui_device>> devices;

    /*! Keep track of the numberOfWindows in the previous render cycle.
     * This way we can call closedLastWindow on the application once.
     */
    ssize_t previousNumberOfWindows = 0;

    gui_system(std::weak_ptr<gui_system_delegate> const &delegate) noexcept :
        delegate(delegate)
    {
        verticalSync = std::make_unique<vertical_sync>(_handlevertical_sync, this);
    }

    virtual ~gui_system() {}

    gui_system(const gui_system &) = delete;
    gui_system &operator=(const gui_system &) = delete;
    gui_system(gui_system &&) = delete;
    gui_system &operator=(gui_system &&) = delete;

    /** Initialize after construction.
     * Call this function directly after the constructor on the same thread.
     */
    virtual void init() = 0;

    template<typename... Args>
    gui_window *make_window(Args &&... args)
    {
        // This function should be called from the main thread from the main loop,
        // and therefor should not have a lock on the window.
        tt_assert(is_main_thread(), "createWindow should be called from the main thread.");
        tt_axiom(gui_system_mutex.recurse_lock_count() == 0);

        auto window = std::make_shared<gui_window_vulkan_win32>(static_cast<gui_system &>(*this), std::forward<Args>(args)...);
        auto window_ptr = window.get();
        window->init();

        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto device = findBestDeviceForWindow(*window);
        if (!device) {
            throw gui_error("Could not find a vulkan-device matching this window");
        }

        device->add(std::move(window));
        return window_ptr;
    }

    /*! Count the number of windows managed by the GUI.
     */
    ssize_t num_windows();

    void render(hires_utc_clock::time_point displayTimePoint) {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        for (auto &device: devices) {
            device->render(displayTimePoint);
        }
        ttlet currentNumberOfWindows = num_windows();
        if (currentNumberOfWindows == 0 && currentNumberOfWindows != previousNumberOfWindows) {
            application::global->run_from_main_loop([this]{
                if (auto delegate_ = this->delegate.lock()) {
                    delegate_->last_window_closed(*this);
                }
            });
        }
        previousNumberOfWindows = currentNumberOfWindows;
    }

    void handlevertical_sync(hires_utc_clock::time_point displayTimePoint)
    {
        render(displayTimePoint);
    }


    static void _handlevertical_sync(void *data, hires_utc_clock::time_point displayTimePoint);

protected:
    gui_device *findBestDeviceForWindow(gui_window const &window);
};

}
