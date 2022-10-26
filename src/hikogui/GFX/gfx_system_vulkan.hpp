// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_system.hpp"
#include <vulkan/vulkan.hpp>

namespace hi::inline v1 {

/** Vulkan gfx_device controller.
 * Manages Vulkan device and a set of Windows.
 */
class gfx_system_vulkan final : public gfx_system {
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

    /** Create an instance of a gfx_device.
     *
     * After the constructor is completed it may be used to get a
     * Vulkan surface and passed to `Window` constructors.
     *
     */
    gfx_system_vulkan();
    ~gfx_system_vulkan();

    gfx_system_vulkan(const gfx_system_vulkan &) = delete;
    gfx_system_vulkan &operator=(const gfx_system_vulkan &) = delete;
    gfx_system_vulkan(gfx_system_vulkan &&) = delete;
    gfx_system_vulkan &operator=(gfx_system_vulkan &&) = delete;

    void init() noexcept(false) override;

    [[nodiscard]] std::unique_ptr<gfx_surface> make_surface(os_handle instance, void *os_window) const noexcept override;

    vk::DispatchLoaderDynamic loader() const noexcept
    {
        hi_axiom(gfx_system_mutex.recurse_lock_count());
        return _loader;
    }

    void destroySurfaceKHR(vk::SurfaceKHR surface)
    {
        hi_axiom(gfx_system_mutex.recurse_lock_count());
        intrinsic.destroySurfaceKHR(surface);
    }

    static VkBool32 debugUtilsMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);
};

} // namespace hi::inline v1
