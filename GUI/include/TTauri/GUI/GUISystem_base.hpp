// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/GUIDevice.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/VerticalSync.hpp"
#include "TTauri/GUI/GUISystemDelegate.hpp"
#include <nonstd/span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace TTauri {


/** Vulkan GUIDevice controller.
 * Manages Vulkan device and a set of Windows.
 */
class GUISystem_base {
public:
    GUISystemDelegate *delegate;

    std::unique_ptr<VerticalSync> verticalSync;

    //! List of all devices.
    std::vector<std::unique_ptr<GUIDevice>> devices;

    /*! Keep track of the numberOfWindows in the previous render cycle.
     * This way we can call closedLastWindow on the application once.
     */
    ssize_t previousNumberOfWindows = 0;

    GUISystem_base(GUISystemDelegate *delegate) noexcept :
        delegate(delegate)
    {
        verticalSync = std::make_unique<VerticalSync>(_handleVerticalSync, this);
    }

    virtual ~GUISystem_base() {}

    GUISystem_base(const GUISystem_base &) = delete;
    GUISystem_base &operator=(const GUISystem_base &) = delete;
    GUISystem_base(GUISystem_base &&) = delete;
    GUISystem_base &operator=(GUISystem_base &&) = delete;

    virtual void initialize() noexcept(false) = 0;

    template<typename T, typename... Args>
    T *addWindow(Args... args)
    {
        auto window = std::make_unique<T>(args...);
        auto window_ptr = window.get();
        window->initialize();

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
        for (auto &device: devices) {
            device->render(displayTimePoint);
        }
        let currentNumberOfWindows = getNumberOfWindows();
        if (currentNumberOfWindows == 0 && currentNumberOfWindows != previousNumberOfWindows) {
            delegate->lastWindowClosed();
        }
        previousNumberOfWindows = currentNumberOfWindows;
    }

    void handleVerticalSync(hires_utc_clock::time_point displayTimePoint)
    {
        render(displayTimePoint);
    }


    static void _handleVerticalSync(void *data, hires_utc_clock::time_point displayTimePoint);

protected:
    GUIDevice *findBestDeviceForWindow(Window const &window);
};

}
