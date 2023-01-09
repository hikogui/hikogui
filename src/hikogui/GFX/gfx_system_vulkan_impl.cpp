// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_system_vulkan.hpp"
#include "gfx_device_vulkan.hpp"
#include "../metadata.hpp"
#include "../architecture.hpp"
#include <chrono>
#include <cstring>

namespace hi::inline v1 {

static bool hasFoundationExtensions(const std::vector<const char *> &requiredExtensions)
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

static std::vector<const char *> filter_available_layers(std::vector<const char *> const &requested_layers)
{
    auto available_layers = vk::enumerateInstanceLayerProperties();

    hi_log_info("Available vulkan layers:");
    auto r = std::vector<const char *>{};
    for (hilet &available_layer : available_layers) {
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

__declspec(no_sanitize_address) vk::Instance vk_create_instance_no_asan(vk::InstanceCreateInfo instance_create_info)
{
    return vk::createInstance(instance_create_info);
}

gfx_system_vulkan::gfx_system_vulkan() : gfx_system()
{
    if constexpr (operating_system::current == operating_system::windows) {
        requiredExtensions = {VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
    } else {
        hi_not_implemented();
        // XXX hi_static_not_implemented();
    }

    applicationInfo = vk::ApplicationInfo(
        metadata::application().name.c_str(),
        VK_MAKE_VERSION(
            metadata::application().version.major, metadata::application().version.minor, metadata::application().version.patch),
        metadata::library().name.c_str(),
        VK_MAKE_VERSION(metadata::library().version.major, metadata::library().version.minor, metadata::library().version.patch),
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

    if (!hasFoundationExtensions(requiredExtensions)) {
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
        "VK_LAYER_KHRONOS_validation",
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

gfx_system_vulkan::~gfx_system_vulkan()
{
    hilet lock = std::scoped_lock(gfx_system_mutex);
#ifndef NDEBUG
    intrinsic.destroy(debugUtilsMessager, nullptr, loader());
#endif
}

void gfx_system_vulkan::init() noexcept(false)
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
         debugUtilsMessageCallback,
         this},
        nullptr,
        loader());
#endif

    for (auto _physicalDevice : intrinsic.enumeratePhysicalDevices()) {
        devices.push_back(std::make_shared<gfx_device_vulkan>(*this, _physicalDevice));
    }
}

VkBool32 gfx_system_vulkan::debugUtilsMessageCallback(
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

} // namespace hi::inline v1
