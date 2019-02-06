//
//  Device.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-06.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Device.hpp"
#include "Instance.hpp"
#include "vulkan_utils.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

void Device::initializeDevice()
{
    auto deviceCreateInfo = vk::DeviceCreateInfo();
    deviceCreateInfo.setPEnabledFeatures(&(instance->requiredFeatures));
    setExtensionNames(deviceCreateInfo, instance->requiredExtensions);
    setLayerNames(deviceCreateInfo, instance->requiredLayers);

    intrinsic = physicalIntrinsic.createDevice(deviceCreateInfo);
}

void Device::add(std::shared_ptr<Window> window)
{
    if (!intrinsic) {
        initializeDevice();
    }

    windows.insert(window);
    window->setDevice(this);
}

void Device::remove(std::shared_ptr<Window> window)
{
    window->setDevice(nullptr);
    windows.erase(window);
}

std::unordered_set<uint32_t> Device::findBestQueueFamilyIndices(std::shared_ptr<Window> window)
{
    uint32_t index = 0;
    uint32_t graphicsPresentAndComputeIndex = UINT32_MAX;
    uint32_t graphicsAndPresentIndex = UINT32_MAX;
    uint32_t graphicsAndComputeIndex = UINT32_MAX;
    uint32_t graphicsIndex = UINT32_MAX;
    uint32_t computeIndex = UINT32_MAX;
    uint32_t presentIndex = UINT32_MAX;
    for (auto queueFamilyProperties: physicalIntrinsic.getQueueFamilyProperties()) {
        if (
            (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) &&
            (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute) &&
            physicalIntrinsic.getSurfaceSupportKHR(index, window->intrinsic)
        ) {
            graphicsPresentAndComputeIndex = index;
        }

        if (
            (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) &&
            physicalIntrinsic.getSurfaceSupportKHR(index, window->intrinsic)
        ) {
            graphicsAndPresentIndex = index;
        }

        if (
            (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) &&
            (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute)
        ) {
            graphicsAndComputeIndex = index;
        }

        // Compute and Present combination is weird, just fallback to single indices.
        if (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsIndex = index;
        }
        if (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute) {
            computeIndex = index;
        }

        if (physicalIntrinsic.getSurfaceSupportKHR(index, window->intrinsic)) {
            presentIndex = index;
        }

        index++;
    }

    std::unordered_set<uint32_t> queueFamilyIndices;

    if (graphicsPresentAndComputeIndex != UINT32_MAX) {
        queueFamilyIndices.insert(graphicsPresentAndComputeIndex);

    } else if (graphicsAndPresentIndex != UINT32_MAX) {
        queueFamilyIndices.insert(graphicsAndPresentIndex);
        if (computeIndex != UINT32_MAX) {
            queueFamilyIndices.insert(computeIndex);
        }

    } else if (graphicsAndComputeIndex != UINT32_MAX) {
        queueFamilyIndices.insert(graphicsAndComputeIndex);

        if (presentIndex != UINT32_MAX) {
            queueFamilyIndices.insert(presentIndex);
        }
    } else {
        if (computeIndex != UINT32_MAX) {
            queueFamilyIndices.insert(computeIndex);
        }
        if (graphicsIndex != UINT32_MAX) {
            queueFamilyIndices.insert(graphicsIndex);
        }
        if (presentIndex != UINT32_MAX) {
            queueFamilyIndices.insert(presentIndex);
        }
    }

    return queueFamilyIndices;
}

int Device::deviceScore(std::shared_ptr<Window> window)
{
    if (!hasRequiredFeatures(physicalIntrinsic, instance->requiredFeatures)) {
        return -1;
    }

    if (!meetsRequiredLimits(physicalIntrinsic, instance->requiredLimits)) {
        return -1;
    }

    // XXX Check if layers are supported.
    // XXX Check if extensions are supported.
    // XXX Check if the best familly index support all commands.

    auto properties = physicalIntrinsic.getProperties();
    switch (properties.deviceType) {
    case vk::PhysicalDeviceType::eCpu: return 1;
    case vk::PhysicalDeviceType::eOther: return 2;
    case vk::PhysicalDeviceType::eVirtualGpu: return 2;
    case vk::PhysicalDeviceType::eIntegratedGpu: return 3;
    case vk::PhysicalDeviceType::eDiscreteGpu: return 4;
    }
}

Device::Device(Instance *parent, vk::PhysicalDevice physicalDevice) :
    instance(parent), physicalIntrinsic(physicalDevice)
{
}

Device::~Device()
{
    windows.clear();
    intrinsic.destroy();
}

}}}
