//
//  Device.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "Device.hpp"
#include "vulkan_utils.hpp"
#include "Window.hpp"
#include <vulkan/vulkan.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>
#include <thread>
#include <vector>

namespace TTauri {
namespace GUI {

struct InstanceError: virtual boost::exception, virtual std::exception {};


enum class InstanceState {
    IDLE,
    RUNNING,
    STOPPING,
    STOPPED
};

/** Vulkan Device controller.
 * Manages Vulkan device and a set of Windows.
 */
class Instance {
public:
    InstanceState state = InstanceState::IDLE;
    
    //! Vulkan instance.
    vk::Instance intrinsic;

    vk::DispatchLoaderDynamic loader;

    //! List of extension that where requested when the instance was created.
    std::vector<const char *> requiredExtensions;

    //! List of extension that where requested when the instance was created.
    std::vector<const char *> requiredLayers;

    //! List of required features for each device.
    vk::PhysicalDeviceFeatures requiredFeatures;

    //! List of required limits for each device.
    vk::PhysicalDeviceLimits requiredLimits;

    //! Application info passed when the instance was created.
    vk::ApplicationInfo applicationInfo;

    //! List of all devices.
    std::vector<std::shared_ptr<Device>> physicalDevices;

    bool add(std::shared_ptr<Window> window);

    void setPreferedDeviceUUID(boost::uuids::uuid deviceUUID) {

        // XXX Move Windows to new prefered device.
    }

    /*! Refresh Display.
     *
     * \outTimestamp Number of nanoseconds since system start.
     * \outputTimestamp Number of nanoseconds since system start until the frame will be displayed on the screen.
     */
    void updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync);

    /*! Create an instance of a Device.
     * After the constructor is completed it may be used to get a
     * Vulkan surface and passed to `Window` constructors.
     *
     * \param extensions a list of Vulkan extensions required. Most useful
     *      for including operating system specific surface extensions.
     */
    Instance(const std::vector<const char *> &extensions);
    ~Instance();

private:
    std::thread maintanceThreadInstance;

    /*! Called when maintance is needed.
     * Run on seperate thread, 15 times per second.
     */
    void maintance(void);

    static void maintanceThread(Instance *self);
};

}}
