// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "gui_system.hpp"
#include <vulkan/vulkan.hpp>

namespace tt {

/** Vulkan gui_device controller.
 * Manages Vulkan device and a set of Windows.
 */
class gui_system_vulkan : public gui_system {
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

    /*! Create an instance of a gui_device.
     * After the constructor is completed it may be used to get a
     * Vulkan surface and passed to `Window` constructors.
     *
     * \param extensions a list of Vulkan extensions required. Most useful
     *      for including operating system specific surface extensions.
     */
    gui_system_vulkan(gui_system_delegate *delegate, const std::vector<const char *> extensions);
    ~gui_system_vulkan();

    gui_system_vulkan(const gui_system_vulkan &) = delete;
    gui_system_vulkan &operator=(const gui_system_vulkan &) = delete;
    gui_system_vulkan(gui_system_vulkan &&) = delete;
    gui_system_vulkan &operator=(gui_system_vulkan &&) = delete;

    void initialize() noexcept(false) override;

    vk::DispatchLoaderDynamic loader() const noexcept {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return _loader;
    }

    void destroySurfaceKHR(vk::SurfaceKHR surface) {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        intrinsic.destroySurfaceKHR(surface);
    }

    static VkBool32 debugUtilsMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};

}
