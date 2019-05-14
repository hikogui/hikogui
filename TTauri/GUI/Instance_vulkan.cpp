// Copyright 2019 Pokitec
// All rights reserved.

#include "Instance_vulkan.hpp"
#include "Device_vulkan.hpp"
#include "TTauri/logging.hpp"
#include <boost/numeric/conversion/cast.hpp>
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
    Instance(),
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

    if (!hasRequiredExtensions(requiredExtensions)) {
        BOOST_THROW_EXCEPTION(MissingRequiredExtensionsError());
    }

    auto instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo);
    instanceCreateInfo.setEnabledExtensionCount(boost::numeric_cast<uint32_t>(requiredExtensions.size()));
    instanceCreateInfo.setPpEnabledExtensionNames(requiredExtensions.data());

#if defined(_WIN32) && !defined(NDEBUG)
    requiredLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
    instanceCreateInfo.setEnabledLayerCount(boost::numeric_cast<uint32_t>(requiredLayers.size()));
    instanceCreateInfo.setPpEnabledLayerNames(requiredLayers.data());

    LOG_INFO("Creating Vulkan instance.");
    intrinsic = vk::createInstance(instanceCreateInfo);

#if (VK_HEADER_VERSION == 97)
    _loader = vk::DispatchLoaderDynamic(intrinsic);
#else
    _loader = vk::DispatchLoaderDynamic(intrinsic, vkGetInstanceProcAddr);
#endif
    
}

void Instance_vulkan::initialize()
{
    scoped_lock lock(TTauri::GUI::mutex);

    Instance::initialize();

    for (auto _physicalDevice : intrinsic.enumeratePhysicalDevices()) {
        auto device = TTauri::make_shared<Device_vulkan>(_physicalDevice);
        devices.push_back(device);
    }
}

}
