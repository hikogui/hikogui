// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Instance.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI {

/** Vulkan Device controller.
 * Manages Vulkan device and a set of Windows.
 */
class Instance_vulkan : public Instance {
protected:
    //! Vulkan instance.
    vk::Instance intrinsic;

    //! Vulkan dynamic loader of library functions.
    vk::DispatchLoaderDynamic _loader;

public:
    struct MissingRequiredExtensionsError : virtual Instance::Error {};

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

    /*! Create an instance of a Device.
     * After the constructor is completed it may be used to get a
     * Vulkan surface and passed to `Window` constructors.
     *
     * \param extensions a list of Vulkan extensions required. Most useful
     *      for including operating system specific surface extensions.
     */
    Instance_vulkan(const std::vector<const char *> extensions);
    ~Instance_vulkan() {}

    Instance_vulkan(const Instance_vulkan &) = delete;
    Instance_vulkan &operator=(const Instance_vulkan &) = delete;
    Instance_vulkan(Instance_vulkan &&) = delete;
    Instance_vulkan &operator=(Instance_vulkan &&) = delete;

    void initialize() override;

    vk::DispatchLoaderDynamic const &loader() const {
        // Mutex not required, all callers that require a loader will use the const version.
        return _loader;
    }

    void destroySurfaceKHR(vk::SurfaceKHR surface) {
        std::scoped_lock lock(TTauri::GUI::mutex);
        intrinsic.destroySurfaceKHR(surface);
    }
};

}
