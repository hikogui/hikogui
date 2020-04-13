// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/VerticalSync.hpp"
#include "TTauri/GUI/InstanceDelegate.hpp"
#include <gsl/gsl>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace TTauri::GUI {


/** Vulkan Device controller.
 * Manages Vulkan device and a set of Windows.
 */
class Instance_base {
public:
    InstanceDelegate *delegate;

    std::unique_ptr<VerticalSync> verticalSync;

    //! List of all devices.
    std::vector<std::unique_ptr<Device>> devices;

    /*! Keep track of the numberOfWindows in the previous render cycle.
     * This way we can call closedLastWindow on the application once.
     */
    ssize_t previousNumberOfWindows = 0;

    Instance_base(InstanceDelegate *delegate) noexcept :
        delegate(delegate)
    {
        verticalSync = std::make_unique<VerticalSync>(_handleVerticalSync, this);
    }

    virtual ~Instance_base() {}

    Instance_base(const Instance_base &) = delete;
    Instance_base &operator=(const Instance_base &) = delete;
    Instance_base(Instance_base &&) = delete;
    Instance_base &operator=(Instance_base &&) = delete;

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

    void render(cpu_utc_clock::time_point displayTimePoint) {
        for (auto &device: devices) {
            device->render(displayTimePoint);
        }
        let currentNumberOfWindows = getNumberOfWindows();
        if (currentNumberOfWindows == 0 && currentNumberOfWindows != previousNumberOfWindows) {
            delegate->lastWindowClosed();
        }
        previousNumberOfWindows = currentNumberOfWindows;
    }

    void handleVerticalSync(cpu_utc_clock::time_point displayTimePoint)
    {
        render(displayTimePoint);
    }


    static void _handleVerticalSync(void *data, cpu_utc_clock::time_point displayTimePoint);

protected:
    Device *findBestDeviceForWindow(Window const &window);
};

}
