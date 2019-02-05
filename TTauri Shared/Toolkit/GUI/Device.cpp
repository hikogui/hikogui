//
//  Device.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Device.hpp"
#include "vulkan_utils.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

Device::Device(const std::vector<const char *> &extensionNames) :
    extensionNames(extensionNames)
{
    applicationInfo = vk::ApplicationInfo(
        "TTauri App", VK_MAKE_VERSION(0, 1, 0),
        "TTauri Engine", VK_MAKE_VERSION(0, 1, 0),
        VK_API_VERSION_1_0
    );

    this->extensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    this->extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    this->extensionNames.push_back(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);
    this->extensionNames.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    this->extensionNames.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    checkRequiredExtensions(this->extensionNames);

    auto instanceCreateInfo = vk::InstanceCreateInfo(
        vk::InstanceCreateFlags(),
        &applicationInfo
    );
    setExtensionNames(instanceCreateInfo, this->extensionNames);

    instance = vk::createInstance(instanceCreateInfo);

    auto requiredFeatures = vk::PhysicalDeviceFeatures();
    auto requiredLimits = vk::PhysicalDeviceLimits();
    auto requiredQueueFlags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;
    auto devices = findBestPhysicalDevices(instance, requiredFeatures, requiredLimits, requiredQueueFlags);

    for (auto device: devices) {
        auto deviceProperties = device.getProperties();
        fprintf(stderr, "Suitable device %s.\n", deviceProperties.deviceName);
    }


}

Device::~Device()
{
    instance.destroy();
}

}}}
