// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "GUISystem_base.hpp"
#include <vulkan/vulkan.hpp>

namespace tt {

/** Vulkan GUIDevice controller.
 * Manages Vulkan device and a set of Windows.
 */
class GUISystem_vulkan : public GUISystem_base {
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

    /*! Create an instance of a GUIDevice.
     * After the constructor is completed it may be used to get a
     * Vulkan surface and passed to `Window` constructors.
     *
     * \param extensions a list of Vulkan extensions required. Most useful
     *      for including operating system specific surface extensions.
     */
    GUISystem_vulkan(GUISystemDelegate *delegate, const std::vector<const char *> extensions);
    ~GUISystem_vulkan();

    GUISystem_vulkan(const GUISystem_vulkan &) = delete;
    GUISystem_vulkan &operator=(const GUISystem_vulkan &) = delete;
    GUISystem_vulkan(GUISystem_vulkan &&) = delete;
    GUISystem_vulkan &operator=(GUISystem_vulkan &&) = delete;

    void initialize() noexcept(false) override;

    vk::DispatchLoaderDynamic loader() const noexcept {
        ttlet lock = std::scoped_lock(mutex);
        return _loader;
    }

    void destroySurfaceKHR(vk::SurfaceKHR surface) {
        ttlet lock = std::scoped_lock(mutex);
        intrinsic.destroySurfaceKHR(surface);
    }

    static VkBool32 debugUtilsMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};

}
