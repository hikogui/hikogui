//
//  Device.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Instance.hpp"
#include "vulkan_utils.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

using namespace std;

Instance::Instance(const std::vector<const char *> &extensionNames) :
    requiredExtensions(extensionNames), requiredLayers(), requiredFeatures(), requiredLimits()
{
    applicationInfo = vk::ApplicationInfo(
        "TTauri App", VK_MAKE_VERSION(0, 1, 0),
        "TTauri Engine", VK_MAKE_VERSION(0, 1, 0),
        VK_API_VERSION_1_0
    );

    requiredExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    if (!hasRequiredExtensions(requiredExtensions)) {
        BOOST_THROW_EXCEPTION(InstanceError());
    }

    auto instanceCreateInfo = vk::InstanceCreateInfo(
        vk::InstanceCreateFlags(),
        &applicationInfo
    );
    setExtensionNames(instanceCreateInfo, requiredExtensions);
    setLayerNames(instanceCreateInfo, requiredLayers);

    intrinsic = vk::createInstance(instanceCreateInfo);

    for (auto _physicalDevice: intrinsic.enumeratePhysicalDevices()) {
        auto physicalDevice = make_shared<Device>(this, _physicalDevice);
        physicalDevices.push_back(physicalDevice);
    }
}

void Instance::add(std::shared_ptr<Window> window)
{
    int bestScore = -1;
    shared_ptr<Device> bestDevice;
    for (auto physicalDevice: physicalDevices) {
        auto score = physicalDevice->score(window);
        if (score > bestScore) {
            bestScore = score;
            bestDevice = physicalDevice;
        }
    }

    switch (bestScore) {
    case -1:
        BOOST_THROW_EXCEPTION(InstanceError());
    case 0:
        fprintf(stderr, "Could not really find a device that can present this window.");
        /* FALLTHROUGH */
    default:
        bestDevice->add(window);
    }
}

Instance::~Instance()
{
    intrinsic.destroy();
}


void Instance::frameUpdate(uint64_t nowTimestamp, uint64_t outputTimestamp)
{
    for (auto physicalDevice: physicalDevices) {
        physicalDevice->frameUpdate(nowTimestamp, outputTimestamp);
    }
}

}}}
