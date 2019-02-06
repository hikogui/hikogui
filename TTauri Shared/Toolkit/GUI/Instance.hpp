//
//  Device.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <memory>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <vulkan/vulkan.hpp>
#include "vulkan_utils.hpp"
#include "Window.hpp"
#include "Device.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

/** Vulkan Device controller.
 * Manages Vulkan device and a set of Windows.
 */
class Instance {
public:
    //! Vulkan instance.
    vk::Instance intrinsic;

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

    void add(std::shared_ptr<Window> window) {
        // XXX Find a correct device to add the window to.
    }

    void setPreferedDeviceUUID(boost::uuids::uuid deviceUUID) {

        // XXX Move Windows to new prefered device.
    }

    /*! Create an instance of a Device.
     * After the constructor is completed it may be used to get a
     * Vulkan surface and passed to `Window` constructors.
     *
     * \param extensions a list of Vulkan extensions required. Most useful
     *      for including operating system specific surface extensions.
     */
    Instance(const std::vector<const char *> &extensions);
    ~Instance();
};

}}}
