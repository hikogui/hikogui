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

    void updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp) {
        for (auto &device: devices) {
            device->updateAndRender(nowTimestamp, outputTimestamp);
        }
    }

protected:
    std::shared_ptr<Device> findBestDeviceForWindow(const std::shared_ptr<Window> &window);
};

}
