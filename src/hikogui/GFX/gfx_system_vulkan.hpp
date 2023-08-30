// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_system.hpp"
#include "gfx_device.hpp"
#include "../macros.hpp"
#include <vulkan/vulkan.hpp>
#include <unordered_set>

hi_export_module(hikogui.GUI : gfx_system_vulkan);

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
    gfx_system_vulkan() : gfx_system()
    {
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
        requiredExtensions = {VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
#else
#error "Not Implemented"
#endif

        applicationInfo = vk::ApplicationInfo(
            get_application_name().c_str(),
            VK_MAKE_VERSION(get_application_version().major, get_application_version().minor, get_application_version().patch),
            get_library_name().c_str(),
            VK_MAKE_VERSION(get_library_version().major, get_library_version().minor, get_library_version().patch),
            VK_API_VERSION_1_2);

        // VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2 extension is needed to retrieve unique identifiers for
        // each GPU in the system, so that we can select the same one on each startup and so that the
        // user could select a different one.
        requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        // VK_KHR_SURFACE extension is needed to draw in a window.
        requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifndef NDEBUG
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        if (!has_foundation_extensions(requiredExtensions)) {
            throw gui_error("Vulkan instance does not have the required extensions");
        }

        auto instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo);
        instanceCreateInfo.setEnabledExtensionCount(narrow_cast<uint32_t>(requiredExtensions.size()));
        instanceCreateInfo.setPpEnabledExtensionNames(requiredExtensions.data());

#ifndef NDEBUG
        requiredFeatures.robustBufferAccess = VK_TRUE;
#endif

#ifndef NDEBUG
        hilet requested_layers = std::vector<char const *>{
            "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2"
            //"VK_LAYER_LUNARG_api_dump"
        };

        requiredLayers = filter_available_layers(requested_layers);
#endif

        instanceCreateInfo.setEnabledLayerCount(narrow_cast<uint32_t>(requiredLayers.size()));
        instanceCreateInfo.setPpEnabledLayerNames(requiredLayers.data());

        hi_log_info("Creating Vulkan instance.");
        intrinsic = vk_create_instance_no_asan(instanceCreateInfo);

#if (VK_HEADER_VERSION == 97)
        _loader = vk::DispatchLoaderDynamic(intrinsic);
#else
        _loader = vk::DispatchLoaderDynamic(intrinsic, vkGetInstanceProcAddr);
#endif
    }

    ~gfx_system_vulkan()
    {
        hilet lock = std::scoped_lock(gfx_system_mutex);
#ifndef NDEBUG
        intrinsic.destroy(debugUtilsMessager, nullptr, loader());
#endif
    }

    gfx_system_vulkan(const gfx_system_vulkan&) = delete;
    gfx_system_vulkan& operator=(const gfx_system_vulkan&) = delete;
    gfx_system_vulkan(gfx_system_vulkan&&) = delete;
    gfx_system_vulkan& operator=(gfx_system_vulkan&&) = delete;

    void init() override
    {
        hilet lock = std::scoped_lock(gfx_system_mutex);

#ifndef NDEBUG
        debugUtilsMessager = intrinsic.createDebugUtilsMessengerEXT(
            {vk::DebugUtilsMessengerCreateFlagsEXT(),
             vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                 // vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                 vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
             vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                 vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
             debug_utils_message_callback,
             this},
            nullptr,
            loader());
#endif

        for (auto _physicalDevice : intrinsic.enumeratePhysicalDevices()) {
            devices.push_back(std::make_shared<gfx_device>(*this, _physicalDevice));
        }
    }

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

private:
    [[nodiscard]] static VkBool32 debug_utils_message_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        auto message = std::string_view(pCallbackData->pMessage);

        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            hi_log_info("Vulkan: {}", message);

        } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            hi_log_warning("Vulkan: {}", message);

        } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            if (message.starts_with("Failed to open dynamic library")) {
                // Steelseries mouse driver will inject:
                // C:\ProgramData\obs-studio-hook\graphics-hook{32,64}.dll
                // One of them will always fail to load.
                hi_log_warning("Vulkan: {}", pCallbackData->pMessage);

            } else {
                hi_log_error("Vulkan: {}", pCallbackData->pMessage);
            }
        }

        return VK_FALSE;
    }

    [[nodiscard]] static bool has_foundation_extensions(const std::vector<const char *>& requiredExtensions)
    {
        auto availableExtensions = std::unordered_set<std::string>();
        for (auto availableExtensionProperties : vk::enumerateInstanceExtensionProperties()) {
            availableExtensions.insert(std::string(availableExtensionProperties.extensionName.data()));
        }

        for (auto requiredExtension : requiredExtensions) {
            if (availableExtensions.count(requiredExtension) == 0) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] static std::vector<const char *> filter_available_layers(std::vector<const char *> const& requested_layers)
    {
        auto available_layers = vk::enumerateInstanceLayerProperties();

        hi_log_info("Available vulkan layers:");
        auto r = std::vector<const char *>{};
        for (hilet& available_layer : available_layers) {
            hilet layer_name = std::string{available_layer.layerName.data()};

            hilet it = std::find(begin(requested_layers), end(requested_layers), layer_name);

            if (it != end(requested_layers)) {
                // Use the *it, because the lifetime of its `char const *` is still available after the function call.
                r.push_back(*it);
                hi_log_info("  * {}", layer_name);
            } else {
                hi_log_info("    {}", layer_name);
            }
        }
        return r;
    }

    [[nodiscard]] __declspec(no_sanitize_address) static vk::Instance vk_create_instance_no_asan(vk::InstanceCreateInfo instance_create_info)
    {
        return vk::createInstance(instance_create_info);
    }
};

} // namespace hi::inline v1
