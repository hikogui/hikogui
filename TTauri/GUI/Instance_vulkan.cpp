// Copyright 2019 Pokitec
// All rights reserved.

#include "Instance_vulkan.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include <chrono>

namespace TTauri::GUI {

using namespace std;

static bool hasRequiredExtensions(const std::vector<const char *> &requiredExtensions)
{
    auto availableExtensions = std::unordered_set<std::string>();
    for (auto availableExtensionProperties : vk::enumerateInstanceExtensionProperties()) {
        availableExtensions.insert(std::string(availableExtensionProperties.extensionName));
    }

    for (auto requiredExtension : requiredExtensions) {
        if (availableExtensions.count(requiredExtension) == 0) {
            return false;
        }
    }
    return true;
}

Instance_vulkan::Instance_vulkan(const std::vector<const char *> extensionNames) :
    Instance_base(),
    requiredExtensions(std::move(extensionNames))
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    applicationInfo = vk::ApplicationInfo(
        "TTauri App", VK_MAKE_VERSION(0, 1, 0), "TTauri Engine", VK_MAKE_VERSION(0, 1, 0), VK_API_VERSION_1_0);

    // VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2 extension is needed to retrieve unique identifiers for
    // each GPU in the system, so that we can select the same one on each startup and so that the
    // user can select a different one.
    requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    // VK_KHR_SURFACE extension is needed to draw in a window.
    requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(_WIN32) && !defined(NDEBUG)
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    if (!hasRequiredExtensions(requiredExtensions)) {
        TTAURI_THROW(gui_error("Vulkan instance does not have the required extensions"));
    }

    auto instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo);
    instanceCreateInfo.setEnabledExtensionCount(numeric_cast<uint32_t>(requiredExtensions.size()));
    instanceCreateInfo.setPpEnabledExtensionNames(requiredExtensions.data());

#if defined(_WIN32) && !defined(NDEBUG)
    requiredLayers.push_back("VK_LAYER_LUNARG_standard_validation");
    //requiredLayers.push_back("VK_LAYER_LUNARG_api_dump");
#endif
    instanceCreateInfo.setEnabledLayerCount(numeric_cast<uint32_t>(requiredLayers.size()));
    instanceCreateInfo.setPpEnabledLayerNames(requiredLayers.data());

    LOG_INFO("Creating Vulkan instance.");
    intrinsic = vk::createInstance(instanceCreateInfo);

#if (VK_HEADER_VERSION == 97)
    _loader = vk::DispatchLoaderDynamic(intrinsic);
#else
    _loader = vk::DispatchLoaderDynamic(intrinsic, vkGetInstanceProcAddr);
#endif
    
}

Instance_vulkan::~Instance_vulkan()
{
#if defined(_WIN32) && !defined(NDEBUG)
    // Boost loggin may get destroyed before Instance gets destroyed,
    // therefor stop sending data to the log now, but still allow abort() to
    // be called on warning or error.
    stopDebugUtilsMessagerLogging = true;
    intrinsic.destroy(debugUtilsMessager, nullptr, loader());
#endif
}

void Instance_vulkan::initialize() noexcept(false)
{
    scoped_lock lock(TTauri::GUI::mutex);

#if defined(_WIN32) && !defined(NDEBUG)
    debugUtilsMessager = intrinsic.createDebugUtilsMessengerEXT({
        vk::DebugUtilsMessengerCreateFlagsEXT(),
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        //vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugUtilsMessageCallback,
        this
    }, nullptr, loader());
#endif

    for (auto _physicalDevice : intrinsic.enumeratePhysicalDevices()) {
        devices.push_back(std::make_unique<Device>(_physicalDevice));
    }
}

VkBool32 Instance_vulkan::debugUtilsMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    auto self = reinterpret_cast<Instance *>(pUserData);

    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        if (!self->stopDebugUtilsMessagerLogging) {
            LOG_DEBUG("Vulkan: %s", pCallbackData->pMessage);
        }
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        if (!self->stopDebugUtilsMessagerLogging) {
            LOG_INFO("Vulkan: %s", pCallbackData->pMessage);
        }
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        if (!self->stopDebugUtilsMessagerLogging) {
            LOG_WARNING("Vulkan: %s", pCallbackData->pMessage);
        }
        std::abort();
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        if (!self->stopDebugUtilsMessagerLogging) {
            LOG_ERROR("Vulkan: %s", pCallbackData->pMessage);
        }
        std::abort();
    }

    return VK_FALSE;
}

}
