// Copyright 2019 Pokitec
// All rights reserved.

#include "GUISystem_vulkan.hpp"
#include "GUISystem.hpp"
#include "GUIDevice.hpp"
#include <chrono>

namespace tt {

using namespace std;

static bool hasFoundationExtensions(const std::vector<const char *> &requiredExtensions)
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

GUISystem_vulkan::GUISystem_vulkan(GUISystemDelegate *delegate, const std::vector<const char *> extensionNames) :
    GUISystem_base(delegate),
    requiredExtensions(std::move(extensionNames))
{
    auto lock = std::scoped_lock(guiMutex);

    applicationInfo = vk::ApplicationInfo(
        "TTauri App", VK_MAKE_VERSION(0, 1, 0), "TTauri Engine", VK_MAKE_VERSION(0, 1, 0), VK_API_VERSION_1_0);

    // VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2 extension is needed to retrieve unique identifiers for
    // each GPU in the system, so that we can select the same one on each startup and so that the
    // user could select a different one.
    requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    // VK_KHR_SURFACE extension is needed to draw in a window.
    requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    if constexpr (OperatingSystem::current == OperatingSystem::Windows && BuildType::current == BuildType::Debug) {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!hasFoundationExtensions(requiredExtensions)) {
        TTAURI_THROW(gui_error("Vulkan instance does not have the required extensions"));
    }

    auto instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo);
    instanceCreateInfo.setEnabledExtensionCount(numeric_cast<uint32_t>(requiredExtensions.size()));
    instanceCreateInfo.setPpEnabledExtensionNames(requiredExtensions.data());

    if constexpr (BuildType::current == BuildType::Debug) {
        requiredFeatures.robustBufferAccess = VK_TRUE;
    }

    if constexpr (OperatingSystem::current == OperatingSystem::Windows && BuildType::current == BuildType::Debug) {
        requiredLayers.push_back("VK_LAYER_LUNARG_standard_validation");
        //requiredLayers.push_back("VK_LAYER_LUNARG_api_dump");
    }

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

GUISystem_vulkan::~GUISystem_vulkan()
{
    if constexpr (OperatingSystem::current == OperatingSystem::Windows && BuildType::current == BuildType::Debug) {
        intrinsic.destroy(debugUtilsMessager, nullptr, loader());
    }
}

void GUISystem_vulkan::initialize() noexcept(false)
{
    auto lock = std::scoped_lock(guiMutex);

    if constexpr (OperatingSystem::current == OperatingSystem::Windows && BuildType::current == BuildType::Debug) {
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
    }

    for (auto _physicalDevice : intrinsic.enumeratePhysicalDevices()) {
        devices.push_back(std::make_unique<GUIDevice>(_physicalDevice));
    }
}

VkBool32 GUISystem_vulkan::debugUtilsMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    //auto self = reinterpret_cast<GUISystem *>(pUserData);

    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_DEBUG("Vulkan: {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_INFO("Vulkan: {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_WARNING("Vulkan: {}", pCallbackData->pMessage);
        std::abort();
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_ERROR("Vulkan: {}", pCallbackData->pMessage);
        std::abort();
    default:
        tt_no_default;
    }

    return VK_FALSE;
}

}
