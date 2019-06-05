// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Device.hpp"
#include "Window.hpp"
#include "TTauri/all.hpp"
#include "globals.hpp"
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

    //! List of all devices.
    std::vector<std::shared_ptr<Device>> devices;

    /*! Keep track of the numberOfWindows in the previous render cycle.
     * This way we can call closedLastWindow on the application once.
     */
    size_t previousNumberOfWindows = 0;

    Instance_base() {}
    virtual ~Instance_base() {}

    Instance_base(const Instance_base &) = delete;
    Instance_base &operator=(const Instance_base &) = delete;
    Instance_base(Instance_base &&) = delete;
    Instance_base &operator=(Instance_base &&) = delete;

    virtual void initialize() {}

    virtual void add(std::shared_ptr<Window> window);

    /*! Count the number of windows managed by the GUI.
     */
    size_t getNumberOfWindows();

    /*! Open a new window.
     *
     * \param windowDelegate window delegate to use to manage the window.
     * \param title Title for the new window
     * \return the window that was created
     */
    virtual void createWindow(std::shared_ptr<GUI::WindowDelegate> windowDelegate, const std::string &title) = 0;

    void render() {
        for (auto &device: devices) {
            device->render();
        }
        let currentNumberOfWindows = getNumberOfWindows();
        if (currentNumberOfWindows == 0 && currentNumberOfWindows != previousNumberOfWindows) {
            application->lastWindowClosed();
        }
        previousNumberOfWindows = currentNumberOfWindows;
    }

protected:
    std::shared_ptr<Device> findBestDeviceForWindow(const std::shared_ptr<Window> &window);
};

}
