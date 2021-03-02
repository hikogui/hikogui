// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system_vulkan.hpp"
#include "gui_device_vulkan.hpp"
#include "ttauri/current_version.hpp"
#include <chrono>
#include <cstring>

namespace tt {

using namespace std;

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

    tt_log_info("Available vulkan layers:");
    auto r = std::vector<const char *>{};
    for (ttlet &available_layer : available_layers) {
        ttlet layer_name = std::string{available_layer.layerName.data()};

        ttlet it = std::find(std::begin(requested_layers), std::end(requested_layers), layer_name);

        if (it != std::end(requested_layers)) {
            // Use the *it, because the lifetime of its `char const *` is still available after the function call.
            r.push_back(*it);
            tt_log_info("  * {}", layer_name);
        } else {
            tt_log_info("    {}", layer_name);
        }
    }
    return r;
}

gui_system_vulkan::gui_system_vulkan(
    std::weak_ptr<gui_system_delegate> const &delegate,
    const std::vector<const char *> extensionNames) :
    gui_system(delegate), requiredExtensions(std::move(extensionNames))
{
    applicationInfo = vk::ApplicationInfo(
        application_version.name.c_str(),
        VK_MAKE_VERSION(application_version.major, application_version.minor, application_version.patch),
        ttauri_version.name.c_str(),
        VK_MAKE_VERSION(ttauri_version.major, ttauri_version.minor, ttauri_version.patch),
        VK_API_VERSION_1_2);

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
        throw gui_error("Vulkan instance does not have the required extensions");
    }

    auto instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo);
    instanceCreateInfo.setEnabledExtensionCount(narrow_cast<uint32_t>(requiredExtensions.size()));
    instanceCreateInfo.setPpEnabledExtensionNames(requiredExtensions.data());

    if constexpr (BuildType::current == BuildType::Debug) {
        requiredFeatures.robustBufferAccess = VK_TRUE;
    }

    if constexpr (OperatingSystem::current == OperatingSystem::Windows && BuildType::current == BuildType::Debug) {
        ttlet requested_layers = std::vector<char const *>{
            "VK_LAYER_KHRONOS_validation",
            //"VK_LAYER_LUNARG_api_dump"
        };

        requiredLayers = filter_available_layers(requested_layers);
    }

    instanceCreateInfo.setEnabledLayerCount(narrow_cast<uint32_t>(requiredLayers.size()));
    instanceCreateInfo.setPpEnabledLayerNames(requiredLayers.data());

    tt_log_info("Creating Vulkan instance.");
    intrinsic = vk::createInstance(instanceCreateInfo);

#if (VK_HEADER_VERSION == 97)
    _loader = vk::DispatchLoaderDynamic(intrinsic);
#else
    _loader = vk::DispatchLoaderDynamic(intrinsic, vkGetInstanceProcAddr);
#endif
}

gui_system_vulkan::~gui_system_vulkan()
{
    if constexpr (OperatingSystem::current == OperatingSystem::Windows && BuildType::current == BuildType::Debug) {
        intrinsic.destroy(debugUtilsMessager, nullptr, loader());
    }
}

void gui_system_vulkan::init() noexcept(false)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    if constexpr (OperatingSystem::current == OperatingSystem::Windows && BuildType::current == BuildType::Debug) {
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
    }

    for (auto _physicalDevice : intrinsic.enumeratePhysicalDevices()) {
        devices.push_back(std::make_shared<gui_device_vulkan>(*this, _physicalDevice));
    }
}

VkBool32 gui_system_vulkan::debugUtilsMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: tt_log_debug("Vulkan: {}", pCallbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: tt_log_info("Vulkan: {}", pCallbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: tt_log_warning("Vulkan: {}", pCallbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: tt_log_error("Vulkan: {}", pCallbackData->pMessage); break;
    default: tt_no_default();
    }

    return VK_FALSE;
}

} // namespace tt
