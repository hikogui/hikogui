// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Device.hpp"
#include "Window.hpp"
#include "VerticalSync.hpp"
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
class Instance {
public:
    VerticalSync verticalSync;

    struct Error : virtual boost::exception, virtual std::exception {};
    struct ErrorNoDeviceForWindow : virtual Error {};

    //! List of all devices.
    std::vector<std::shared_ptr<Device>> devices;

    Instance() {}
    virtual ~Instance() {}

    Instance(const Instance &) = delete;
    Instance &operator=(const Instance &) = delete;
    Instance(Instance &&) = delete;
    Instance &operator=(Instance &&) = delete;

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
    virtual void createWindow(std::shared_ptr<GUI::Window::Delegate> windowDelegate, const std::string &title) = 0;

    static std::shared_ptr<Instance> singleton;

protected:
    std::shared_ptr<Device> findBestDeviceForWindow(const std::shared_ptr<Window> &window);

    /*! Called when maintance is needed.
     * Run on seperate thread, 15 times per second.
     */
    void maintenance();
};

}
