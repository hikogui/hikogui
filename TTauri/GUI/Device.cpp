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
#include "TTauri/Logging.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <vector>
#include <tuple>

namespace TTauri {
namespace GUI {

using namespace std;

static bool hasRequiredExtensions(const vk::PhysicalDevice &physicalDevice, const std::vector<const char *> &requiredExtensions)
{
    auto availableExtensions = std::unordered_set<std::string>();
    for (auto availableExtensionProperties : physicalDevice.enumerateDeviceExtensionProperties()) {
        availableExtensions.insert(std::string(availableExtensionProperties.extensionName));
    }

    for (auto requiredExtension : requiredExtensions) {
        if (availableExtensions.count(requiredExtension) == 0) {
            return false;
        }
    }
    return true;
}

Device::Device(Instance *instance, vk::PhysicalDevice physicalDevice) :
    instance(instance), physicalIntrinsic(physicalDevice), state(DeviceState::NO_DEVICE)
{
    auto result = physicalIntrinsic.getProperties2KHR<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceIDProperties>(instance->loader);

    auto resultDeviceProperties2 = result.get<vk::PhysicalDeviceProperties2>();
    auto resultDeviceIDProperties = result.get<vk::PhysicalDeviceIDProperties>();

    requiredExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);

    deviceID = resultDeviceProperties2.properties.deviceID;
    vendorID = resultDeviceProperties2.properties.vendorID;
    deviceName = std::string(resultDeviceProperties2.properties.deviceName);
    memcpy(deviceUUID.data, resultDeviceIDProperties.deviceUUID, deviceUUID.size());

    memoryProperties = physicalIntrinsic.getMemoryProperties();
}

Device::~Device()
{
    windows.clear();
    intrinsic.destroy();
}

std::string Device::str() const
{
    return (boost::format("%04x:%04x %s %s") % vendorID % deviceID % deviceName % deviceUUID).str();
}


void Device::initializeDevice(std::shared_ptr<Window> window)
{
    float defaultQueuePriority = 1.0;

    vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    for (auto queueFamilyIndexAndCapabilities: queueFamilyIndicesAndCapabilities) {
        auto index = queueFamilyIndexAndCapabilities.first;

        auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), index, 1, &defaultQueuePriority);
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    }

    auto deviceCreateInfo = vk::DeviceCreateInfo();
    deviceCreateInfo.setPEnabledFeatures(&(instance->requiredFeatures));
    setQueueCreateInfos(deviceCreateInfo, deviceQueueCreateInfos);
    setExtensionNames(deviceCreateInfo, requiredExtensions);
    setLayerNames(deviceCreateInfo, instance->requiredLayers);

    intrinsic = physicalIntrinsic.createDevice(deviceCreateInfo);

    for (auto queueFamilyIndexAndCapabilities: queueFamilyIndicesAndCapabilities) {
        auto index = queueFamilyIndexAndCapabilities.first;
        auto queueCapabilities = queueFamilyIndexAndCapabilities.second;

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

    std::scoped_lock lock(stateMutex);

    windows.insert(window);
    window->setDevice(this);
}

void Device::remove(std::shared_ptr<Window> window)
{
    std::scoped_lock lock(stateMutex);

    window->setDevice(nullptr);
    windows.erase(window);
}

static bool scoreIsGreater(const pair<uint32_t, QueueCapabilities> &a, const pair<uint32_t, QueueCapabilities> &b) {
    return a.second.score() > b.second.score();
}

std::vector<std::pair<uint32_t, QueueCapabilities>> Device::findBestQueueFamilyIndices(std::shared_ptr<Window> window)
{
    LOG_INFO(" - Scoring QueueFamilies");

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

        LOG_INFO("    * %i: capabilities=%s, score=%i") % index % capabilities.str() % score;

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
    uint32_t score = 0;

    LOG_INFO("Scoring device: %s") % str();
    if (!hasRequiredFeatures(physicalIntrinsic, instance->requiredFeatures)) {
        LOG_INFO(" - Does not have the required features.");
        return -1;
    }

    if (!meetsRequiredLimits(physicalIntrinsic, instance->requiredLimits)) {
        LOG_INFO(" - Does not meet the required limits.");
        return -1;
    }

    if (!hasRequiredExtensions(physicalIntrinsic, requiredExtensions)) {
        LOG_INFO(" - Does not have the required extensions.");
        return -1;
    }

    queueFamilyIndicesAndCapabilities = findBestQueueFamilyIndices(window);
    QueueCapabilities deviceCapabilities;
    for (auto queueFamilyIndexAndCapabilities: queueFamilyIndicesAndCapabilities) {
        deviceCapabilities |= queueFamilyIndexAndCapabilities.second;
    }
    LOG_INFO(" - Capabilities=%s") % deviceCapabilities.str();

    if (!deviceCapabilities.handlesGraphicsAndCompute()) {
        LOG_INFO(" - Does not have both the graphics and compute queues.");
        return -1;

    } else if (!deviceCapabilities.handlesPresent) {
        LOG_INFO(" - Does not have a present queue.");
        return 0;
    }

    // Give score based on colour quality.
    LOG_INFO(" - Surface formats:");
    uint32_t bestSurfaceFormatScore = 0;
    auto formats = physicalIntrinsic.getSurfaceFormatsKHR(window->intrinsic);
    for (auto format: formats) {
        uint32_t score = 0;

        LOG_INFO("    * colorSpace=%s, format=%s") % vk::to_string(format.colorSpace) % vk::to_string(format.format);

        switch (format.colorSpace) {
        case vk::ColorSpaceKHR::eSrgbNonlinear: score += 1; break;
        case vk::ColorSpaceKHR::eExtendedSrgbNonlinearEXT: score += 100; break;
        default: continue;
        }

        switch (format.format) {
        case vk::Format::eR8G8B8A8Unorm: score += 2; break;
        case vk::Format::eB8G8R8A8Unorm: score += 2; break;
        case vk::Format::eR16G16B16A16Sfloat: score += 12; break;
        case vk::Format::eR8G8B8Unorm: score += 1; break;
        case vk::Format::eR16G16B16Sfloat: score += 11; break;
        case vk::Format::eUndefined: score += 2; break;
        default: continue;
        }

        if (score > bestSurfaceFormatScore) {
            bestSurfaceFormatScore = score;
            bestSurfaceFormat = format;
        }
    }
    score += bestSurfaceFormatScore;

    if (bestSurfaceFormatScore == 0) {
        LOG_INFO(" - Does not have a suitable surface format.");
        return 0;
    }

    LOG_INFO(" - Surface present modes:");
    uint32_t bestSurfacePresentModeScore = 0;
    auto presentModes = physicalIntrinsic.getSurfacePresentModesKHR(window->intrinsic);
    for (auto presentMode: presentModes) {
        uint32_t score = 0;

        LOG_INFO("    * presentMode=%s") % vk::to_string(presentMode);

        switch (presentMode) {
        case vk::PresentModeKHR::eImmediate: score += 1; break;
        case vk::PresentModeKHR::eFifoRelaxed: score += 2; break;
        case vk::PresentModeKHR::eFifo: score += 3; break;
        case vk::PresentModeKHR::eMailbox: score += 1; break; // mailbox does not wait for vsync.
        default: continue;
        }

        if (score > bestSurfacePresentModeScore) {
            bestSurfacePresentModeScore = score;
            bestSurfacePresentMode = presentMode;
        }
    }
    score += bestSurfacePresentModeScore;

    if (score < bestSurfacePresentModeScore) {
        LOG_INFO(" - Does not have a suitable surface present mode.");
        return 0;
    }

    // Give score based on the perfomance of the device.
    auto properties = physicalIntrinsic.getProperties();
    LOG_INFO(" - Type of device: %s") % vk::to_string(properties.deviceType);
    switch (properties.deviceType) {
    case vk::PhysicalDeviceType::eCpu: score += 1; break;
    case vk::PhysicalDeviceType::eOther: score += 1; break;
    case vk::PhysicalDeviceType::eVirtualGpu: score += 2; break;
    case vk::PhysicalDeviceType::eIntegratedGpu: score += 3; break;
    case vk::PhysicalDeviceType::eDiscreteGpu: score += 4; break;
    }

    return score;
}

void Device::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    if (stateMutex.try_lock()) {
        if (state == DeviceState::READY_TO_DRAW) {
            for (auto window: windows) {
                window->updateAndRender(nowTimestamp, outputTimestamp, blockOnVSync);
                blockOnVSync = false;
            }
        }
        stateMutex.unlock();
    }
}

void Device::maintance()
{
    for (auto window: windows) {
        window->maintenance();
    }
}

uint32_t Device::findMemoryType(uint32_t validMemoryTypeMask, vk::MemoryPropertyFlags properties)
{
    for (uint32_t typeIndex = 0; typeIndex < memoryProperties.memoryTypeCount; typeIndex++) {
        if ((validMemoryTypeMask & (typeIndex << typeIndex)) && ((memoryProperties.memoryTypes[typeIndex].propertyFlags & properties) == properties)) {
            return typeIndex;
        }
    }
    BOOST_THROW_EXCEPTION(Device::AllocateMemoryError());
}


vk::DeviceMemory Device::allocateDeviceMemory(size_t size, uint32_t validMemoryTypeMask, vk::MemoryPropertyFlags properties)
{
    auto memoryTypeIndex = findMemoryType(validMemoryTypeMask, properties);

    vk::MemoryAllocateInfo memoryAllocateInfo = {size, memoryTypeIndex};
    return intrinsic.allocateMemory(memoryAllocateInfo, nullptr);
}

vk::DeviceMemory Device::allocateDeviceMemory(vk::Buffer buffer, vk::MemoryPropertyFlags properties)
{
    auto memoryRequirements = intrinsic.getBufferMemoryRequirements(buffer);
    return allocateDeviceMemory(memoryRequirements.size, memoryRequirements.memoryTypeBits, properties);
}

vk::DeviceMemory Device::allocateDeviceMemoryAndBind(vk::Buffer buffer, vk::MemoryPropertyFlags properties)
{
    auto memory = allocateDeviceMemory(buffer, properties);
    intrinsic.bindBufferMemory(buffer, memory, 0);
    return memory;
}

}}
