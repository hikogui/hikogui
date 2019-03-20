//
//  Device.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "Device.hpp"
#include "Window.hpp"

#include "TTauri/utils.hpp"
#include <gsl/gsl>
#include <boost/uuid/uuid.hpp>

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace TTauri {
namespace GUI {

/** Vulkan Device controller.
 * Manages Vulkan device and a set of Windows.
 */
class Instance {
public:
    std::recursive_mutex mutex;

    struct Error : virtual boost::exception, virtual std::exception {};
    struct ErrorNoDeviceForWindow : virtual Error {};

    //! List of all devices.
    std::vector<std::shared_ptr<Device>> devices;

    Instance();
    virtual ~Instance();

    Instance(const Instance &) = delete;
    Instance &operator=(const Instance &) = delete;
    Instance(Instance &&) = delete;
    Instance &operator=(Instance &&) = delete;

    virtual void initialize();

    virtual void add(std::shared_ptr<Window> window);

    /*! Open a new window.
     *
     * \param windowDelegate window delegate to use to manage the window.
     * \param title Title for the new window
     * \return the window that was created
     */
    virtual void createWindow(std::shared_ptr<GUI::Window::Delegate> windowDelegate, const std::string &title) = 0;

    virtual void setPreferedDevice(boost::uuids::uuid deviceUUID);

    /*! Refresh Display.
     *
     * \outTimestamp Number of nanoseconds since system start.
     * \outputTimestamp Number of nanoseconds since system start until the frame will be displayed on the screen.
     */
    virtual void updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync);

    static std::shared_ptr<Instance> singleton;

protected:
    std::shared_ptr<Device> findBestDeviceForWindow(const std::shared_ptr<Window> &window);

    std::thread maintanceThread;

    /*! Called when maintance is needed.
     * Run on seperate thread, 15 times per second.
     */
    void maintenance();
    bool stopMaintenance = false;

    static void maintenanceLoop(gsl::not_null<Instance *> self);
};

}}
