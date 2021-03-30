// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

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

    //! application info passed when the instance was created.
    vk::ApplicationInfo applicationInfo;

    /** Create an instance of a gui_device.
     * After the constructor is completed it may be used to get a
     * Vulkan surface and passed to `Window` constructors.
     *
     * @param delegate The delegate for the gui-system.
     * @param extensions a list of Vulkan extensions required. Most useful
     *      for including operating system specific surface extensions.
     */
    gui_system_vulkan(std::weak_ptr<gui_system_delegate> const &delegate, const std::vector<const char *> extensions);
    ~gui_system_vulkan();

    gui_system_vulkan(const gui_system_vulkan &) = delete;
    gui_system_vulkan &operator=(const gui_system_vulkan &) = delete;
    gui_system_vulkan(gui_system_vulkan &&) = delete;
    gui_system_vulkan &operator=(gui_system_vulkan &&) = delete;

    void init() noexcept(false) override;

    vk::DispatchLoaderDynamic loader() const noexcept {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return _loader;
    }

    void destroySurfaceKHR(vk::SurfaceKHR surface) {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        intrinsic.destroySurfaceKHR(surface);
    }

    static VkBool32 debugUtilsMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};

}
