//
//  Device.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-06.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Device.hpp"

#include <vector>
#include <tuple>
#include "Instance.hpp"
#include "vulkan_utils.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

using namespace std;

Device::Device(Instance *parent, vk::PhysicalDevice physicalDevice) :
    instance(parent), physicalIntrinsic(physicalDevice), state(DeviceState::NO_DEVICE)
{
}

Device::~Device()
{
    windows.clear();
    intrinsic.destroy();
}

void Device::initializeDevice(std::shared_ptr<Window> window)
{
    float defaultQueuePriority = 1.0;
    auto queueFamilyIndicesAndQueueCapabilitiess = findBestQueueFamilyIndices(window);

    vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    for (auto queueFamilyIndexAndQueueCapabilities: queueFamilyIndicesAndQueueCapabilitiess) {
        auto index = queueFamilyIndexAndQueueCapabilities.first;

        auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), index, 1, &defaultQueuePriority);
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    }

    auto deviceCreateInfo = vk::DeviceCreateInfo();
    deviceCreateInfo.setPEnabledFeatures(&(instance->requiredFeatures));
    setQueueCreateInfos(deviceCreateInfo, deviceQueueCreateInfos);
    setExtensionNames(deviceCreateInfo, instance->requiredExtensions);
    setLayerNames(deviceCreateInfo, instance->requiredLayers);

    intrinsic = physicalIntrinsic.createDevice(deviceCreateInfo);

    for (auto queueFamilyIndexAndQueueCapabilities: queueFamilyIndicesAndQueueCapabilitiess) {
        auto index = queueFamilyIndexAndQueueCapabilities.first;
        auto queueCapabilities = queueFamilyIndexAndQueueCapabilities.second;

        auto queue = make_shared<Queue>(this, index, 0, queueCapabilities);
        if (queueCapabilities.handlesGraphics) {
            graphicQueue = queue;
        }
        if (queueCapabilities.handlesPresent) {
            presentQueue = queue;
        }
        if (queueCapabilities.handlesCompute) {
            computeQueue = queue;
        }
    }

    state = DeviceState::READY_TO_DRAW;
}

void Device::add(std::shared_ptr<Window> window)
{
    if (!intrinsic) {
        initializeDevice(window);
    }

    boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

    windows.insert(window);
    window->setDevice(this);
}

void Device::remove(std::shared_ptr<Window> window)
{
    boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

    window->setDevice(nullptr);
    windows.erase(window);
}

static bool scoreIsGreater(const pair<uint32_t, QueueCapabilities> &a, const pair<uint32_t, QueueCapabilities> &b) {
    return a.second.score() > b.second.score();
}

std::vector<std::pair<uint32_t, QueueCapabilities>> Device::findBestQueueFamilyIndices(std::shared_ptr<Window> window)
{
    uint32_t index = 0;

    // Create a sorted list of queueFamilies depending on the scoring.
    vector<pair<uint32_t, QueueCapabilities>> queueFamilieScores;
    for (auto queueFamilyProperties: physicalIntrinsic.getQueueFamilyProperties()) {
        QueueCapabilities capabilities;
        if (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
            capabilities.handlesGraphics = true;
        }
        if (physicalIntrinsic.getSurfaceSupportKHR(index, window->intrinsic)) {
            capabilities.handlesPresent = true;
        }
        if (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute) {
            capabilities.handlesCompute = true;
        }

        uint32_t score = 0;
        score += capabilities.handlesEverything() ? 10 : 0;
        score += capabilities.handlesGraphicsAndPresent() ? 5 : 0;
        score += capabilities.handlesGraphics ? 1 : 0;
        score += capabilities.handlesPresent ? 1 : 0;
        score += capabilities.handlesCompute ? 1 : 0;

        queueFamilieScores.push_back({index, capabilities});
        index++;
    }
    sort(queueFamilieScores.begin(), queueFamilieScores.end(), scoreIsGreater);

    // Iterativly add indices if it completes the totalQueueCapabilities.
    vector<pair<uint32_t, QueueCapabilities>> queueFamilyIndicesAndQueueCapabilitiess;
    QueueCapabilities totalCapabilities;
    for (auto queueFamilyScore: queueFamilieScores) {
        auto index = get<0>(queueFamilyScore);
        auto capabilities = get<1>(queueFamilyScore);

        if (!totalCapabilities.handlesAllOff(capabilities)) {
            queueFamilyIndicesAndQueueCapabilitiess.push_back({index, capabilities - totalCapabilities});
            totalCapabilities |= capabilities;
        }
    }

    return queueFamilyIndicesAndQueueCapabilitiess;
}

int Device::score(std::shared_ptr<Window> window)
{
    if (!hasRequiredFeatures(physicalIntrinsic, instance->requiredFeatures)) {
        return -1;
    }

    if (!meetsRequiredLimits(physicalIntrinsic, instance->requiredLimits)) {
        return -1;
    }

    if (!hasRequiredExtensions(physicalIntrinsic, instance->requiredExtensions)) {
        return -1;
    }

    QueueCapabilities queueCapabilities;
    for (auto queueFamilyIndexAndQueueCapabilities: findBestQueueFamilyIndices(window)) {
        queueCapabilities |= queueFamilyIndexAndQueueCapabilities.second;
    }
    if (queueCapabilities.handlesGraphicsAndCompute()) {
        return -1; // Both Graphics and Compute MUST be available.
    } else if (queueCapabilities.handlesPresent) {
        return 0; // Present SHOULD be available, but could still work, but penalise.
    }

    auto properties = physicalIntrinsic.getProperties();
    switch (properties.deviceType) {
    case vk::PhysicalDeviceType::eCpu: return 1;
    case vk::PhysicalDeviceType::eOther: return 2;
    case vk::PhysicalDeviceType::eVirtualGpu: return 2;
    case vk::PhysicalDeviceType::eIntegratedGpu: return 3;
    case vk::PhysicalDeviceType::eDiscreteGpu: return 4;
    }
}

void Device::frameUpdate(uint64_t nowTimestamp, uint64_t outputTimestamp)
{
    if (stateMutex.try_lock_shared()) {
        if (state == DeviceState::READY_TO_DRAW) {
            for (auto window: windows) {
                window->frameUpdate(nowTimestamp, outputTimestamp);
            }
        }
        stateMutex.unlock_shared();
    }
}

}}}
