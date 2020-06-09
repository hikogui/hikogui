// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Instance_base.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri {

/** Vulkan Device controller.
 * Manages Vulkan device and a set of Windows.
 */
class Instance_vulkan : public Instance_base {
protected:

    //! Vulkan dynamic loader of library functions.
    vk::DispatchLoaderDynamic _loader;

    vk::DebugUtilsMessengerEXT debugUtilsMessager;

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

    /*! Create an instance of a Device.
     * After the constructor is completed it may be used to get a
     * Vulkan surface and passed to `Window` constructors.
     *
     * \param extensions a list of Vulkan extensions required. Most useful
     *      for including operating system specific surface extensions.
     */
    Instance_vulkan(InstanceDelegate *delegate, const std::vector<const char *> extensions);
    ~Instance_vulkan();

    Instance_vulkan(const Instance_vulkan &) = delete;
    Instance_vulkan &operator=(const Instance_vulkan &) = delete;
    Instance_vulkan(Instance_vulkan &&) = delete;
    Instance_vulkan &operator=(Instance_vulkan &&) = delete;

    void initialize() noexcept(false) override;

    vk::DispatchLoaderDynamic loader() const noexcept {
        return _loader;
    }

    void destroySurfaceKHR(vk::SurfaceKHR surface) {
        auto lock = std::scoped_lock(guiMutex);
        intrinsic.destroySurfaceKHR(surface);
    }

    static VkBool32 debugUtilsMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};

}
