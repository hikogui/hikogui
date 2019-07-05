// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "globals.hpp"
#include "Device.hpp"
#include "Window.hpp"
#include "VerticalSync.hpp"
#include "TTauri/Application.hpp"
#include <gsl/gsl>
#include <boost/uuid/uuid.hpp>
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
    struct Error : virtual boost::exception, virtual std::exception {};
    struct ErrorNoDeviceForWindow : virtual Error {};

    std::unique_ptr<VerticalSync> verticalSync;

    //! List of all devices.
    std::vector<std::unique_ptr<Device>> devices;

    /*! Keep track of the numberOfWindows in the previous render cycle.
     * This way we can call closedLastWindow on the application once.
     */
    size_t previousNumberOfWindows = 0;

    Instance_base() {
        verticalSync = std::make_unique<VerticalSync>(_handleVerticalSync, this);
    }

    virtual ~Instance_base() {}

    Instance_base(const Instance_base &) = delete;
    Instance_base &operator=(const Instance_base &) = delete;
    Instance_base(Instance_base &&) = delete;
    Instance_base &operator=(Instance_base &&) = delete;

    virtual void initialize() {}

    template<typename T, typename... Args>
    T *addWindow(Args... args)
    {
        auto window = std::make_unique<T>(args...);
        auto window_ptr = window.get();
        window->initialize();

        auto device = findBestDeviceForWindow(*window);
        if (!device) {
            BOOST_THROW_EXCEPTION(ErrorNoDeviceForWindow());
        }

        device->add(std::move(window));
        return window_ptr;
    }


    /*! Count the number of windows managed by the GUI.
     */
    size_t getNumberOfWindows();

    void render() {
        for (auto &device: devices) {
            device->render();
        }
        let currentNumberOfWindows = getNumberOfWindows();
        if (currentNumberOfWindows == 0 && currentNumberOfWindows != previousNumberOfWindows) {
            singleton<Application>->lastWindowClosed();
        }
        previousNumberOfWindows = currentNumberOfWindows;
    }

    void handleVerticalSync()
    {
        render();
    }


    static void _handleVerticalSync(void *data);

protected:
    Device *findBestDeviceForWindow(Window const &window);
};

}
