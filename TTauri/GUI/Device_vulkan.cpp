// Copyright 2019 Pokitec
// All rights reserved.

#include "Device_vulkan.hpp"
#include "Instance.hpp"
#include "PipelineImage.hpp"
#include "PipelineImage_DeviceShared.hpp"
#include "Window.hpp"
#include <gsl/gsl>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace TTauri::GUI {

using namespace std;
using namespace gsl;

#define QUEUE_CAPABILITY_GRAPHICS 1
#define QUEUE_CAPABILITY_COMPUTE 2
#define QUEUE_CAPABILITY_PRESENT 4
#define QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT (QUEUE_CAPABILITY_GRAPHICS | QUEUE_CAPABILITY_PRESENT)
#define QUEUE_CAPABILITY_ALL (QUEUE_CAPABILITY_GRAPHICS | QUEUE_CAPABILITY_COMPUTE | QUEUE_CAPABILITY_PRESENT)

static bool hasRequiredExtensions(const vk::PhysicalDevice &physicalDevice, const std::vector<const char *> &requiredExtensions)
{
    auto availableExtensions = unordered_set<string>();
    for (auto availableExtensionProperties : physicalDevice.enumerateDeviceExtensionProperties()) {
        availableExtensions.insert(string(availableExtensionProperties.extensionName));
    }

    for (auto requiredExtension : requiredExtensions) {
        if (availableExtensions.count(requiredExtension) == 0) {
            return false;
        }
    }
    return true;
}

static bool meetsRequiredLimits(const vk::PhysicalDevice& physicalDevice, const vk::PhysicalDeviceLimits& requiredLimits)
{
    return true;
}

static bool hasRequiredFeatures(const vk::PhysicalDevice& physicalDevice, const vk::PhysicalDeviceFeatures& requiredFeatures)
{
    let availableFeatures = physicalDevice.getFeatures();
    auto meetsRequirements = true;

    meetsRequirements &= (requiredFeatures.robustBufferAccess == VK_TRUE) ? (availableFeatures.robustBufferAccess == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.fullDrawIndexUint32 == VK_TRUE) ? (availableFeatures.fullDrawIndexUint32 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.imageCubeArray == VK_TRUE) ? (availableFeatures.imageCubeArray == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.independentBlend == VK_TRUE) ? (availableFeatures.independentBlend == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.geometryShader == VK_TRUE) ? (availableFeatures.geometryShader == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.tessellationShader == VK_TRUE) ? (availableFeatures.tessellationShader == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sampleRateShading == VK_TRUE) ? (availableFeatures.sampleRateShading == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.dualSrcBlend == VK_TRUE) ? (availableFeatures.dualSrcBlend == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.logicOp == VK_TRUE) ? (availableFeatures.logicOp == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.multiDrawIndirect == VK_TRUE) ? (availableFeatures.multiDrawIndirect == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.drawIndirectFirstInstance == VK_TRUE) ? (availableFeatures.drawIndirectFirstInstance == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.depthClamp == VK_TRUE) ? (availableFeatures.depthClamp == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.depthBiasClamp == VK_TRUE) ? (availableFeatures.depthBiasClamp == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.fillModeNonSolid == VK_TRUE) ? (availableFeatures.fillModeNonSolid == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.depthBounds == VK_TRUE) ? (availableFeatures.depthBounds == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.wideLines == VK_TRUE) ? (availableFeatures.wideLines == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.largePoints == VK_TRUE) ? (availableFeatures.largePoints == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.alphaToOne == VK_TRUE) ? (availableFeatures.alphaToOne == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.multiViewport == VK_TRUE) ? (availableFeatures.multiViewport == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.samplerAnisotropy == VK_TRUE) ? (availableFeatures.samplerAnisotropy == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.textureCompressionETC2 == VK_TRUE) ? (availableFeatures.textureCompressionETC2 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.textureCompressionASTC_LDR == VK_TRUE) ? (availableFeatures.textureCompressionASTC_LDR == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.textureCompressionBC == VK_TRUE) ? (availableFeatures.textureCompressionBC == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.occlusionQueryPrecise == VK_TRUE) ? (availableFeatures.occlusionQueryPrecise == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.pipelineStatisticsQuery == VK_TRUE) ? (availableFeatures.pipelineStatisticsQuery == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.vertexPipelineStoresAndAtomics == VK_TRUE) ? (availableFeatures.vertexPipelineStoresAndAtomics == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.fragmentStoresAndAtomics == VK_TRUE) ? (availableFeatures.fragmentStoresAndAtomics == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderTessellationAndGeometryPointSize == VK_TRUE) ? (availableFeatures.shaderTessellationAndGeometryPointSize == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderImageGatherExtended == VK_TRUE) ? (availableFeatures.shaderImageGatherExtended == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageExtendedFormats == VK_TRUE) ? (availableFeatures.shaderStorageImageExtendedFormats == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageMultisample == VK_TRUE) ? (availableFeatures.shaderStorageImageMultisample == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageReadWithoutFormat == VK_TRUE) ? (availableFeatures.shaderStorageImageReadWithoutFormat == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageWriteWithoutFormat == VK_TRUE) ? (availableFeatures.shaderStorageImageWriteWithoutFormat == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderUniformBufferArrayDynamicIndexing == VK_TRUE) ? (availableFeatures.shaderUniformBufferArrayDynamicIndexing == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderSampledImageArrayDynamicIndexing == VK_TRUE) ? (availableFeatures.shaderSampledImageArrayDynamicIndexing == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageBufferArrayDynamicIndexing == VK_TRUE) ? (availableFeatures.shaderStorageBufferArrayDynamicIndexing == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageArrayDynamicIndexing == VK_TRUE) ? (availableFeatures.shaderStorageImageArrayDynamicIndexing == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderClipDistance == VK_TRUE) ? (availableFeatures.shaderClipDistance == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderCullDistance == VK_TRUE) ? (availableFeatures.shaderCullDistance == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderFloat64 == VK_TRUE) ? (availableFeatures.shaderFloat64 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderInt64 == VK_TRUE) ? (availableFeatures.shaderInt64 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderInt16 == VK_TRUE) ? (availableFeatures.shaderInt16 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderResourceResidency == VK_TRUE) ? (availableFeatures.shaderResourceResidency == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderResourceMinLod == VK_TRUE) ? (availableFeatures.shaderResourceMinLod == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseBinding == VK_TRUE) ? (availableFeatures.sparseBinding == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidencyBuffer == VK_TRUE) ? (availableFeatures.sparseResidencyBuffer == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidencyImage2D == VK_TRUE) ? (availableFeatures.sparseResidencyImage2D == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidencyImage3D == VK_TRUE) ? (availableFeatures.sparseResidencyImage3D == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidency2Samples == VK_TRUE) ? (availableFeatures.sparseResidency2Samples == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidency4Samples == VK_TRUE) ? (availableFeatures.sparseResidency4Samples == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidency8Samples == VK_TRUE) ? (availableFeatures.sparseResidency8Samples == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidency16Samples == VK_TRUE) ? (availableFeatures.sparseResidency16Samples == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidencyAliased == VK_TRUE) ? (availableFeatures.sparseResidencyAliased == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.variableMultisampleRate == VK_TRUE) ? (availableFeatures.variableMultisampleRate == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.inheritedQueries == VK_TRUE) ? (availableFeatures.inheritedQueries == VK_TRUE) : true;

    return meetsRequirements;
}

Device_vulkan::Device_vulkan(vk::PhysicalDevice physicalDevice) :
    Device_base(),
    physicalIntrinsic(std::move(physicalDevice))
{
    auto result = physicalIntrinsic.getProperties2KHR<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceIDProperties>(instance->loader());
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

Device_vulkan::~Device_vulkan()
{
    try {
        [[gsl::suppress(f.6)]] {
            imagePipeline->destroy(gsl::make_not_null(this));
            imagePipeline = nullptr;

            vmaDestroyAllocator(allocator);

            for (uint32_t index = 0; index < 3; index++) {
                // Destroy one command pool for each queue index.
                if (graphicsQueueIndex == index) {
                    this->intrinsic.destroy(graphicsCommandPool);
                    continue;
                }
                if (presentQueueIndex == index) {
                    this->intrinsic.destroy(presentCommandPool);
                    continue;
                }
                if (computeQueueIndex == index) {
                    this->intrinsic.destroy(computeCommandPool);
                    continue;
                }
            }

            intrinsic.destroy();
        }

    } catch (...) {
        abort();
    }
}

void Device_vulkan::initializeDevice(Window const &window)
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    const float defaultQueuePriority = 1.0;

    vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    for (auto queueFamilyIndexAndCapabilities : queueFamilyIndicesAndCapabilities) {
        auto index = queueFamilyIndexAndCapabilities.first;
        deviceQueueCreateInfos.push_back({ vk::DeviceQueueCreateFlags(), index, 1, &defaultQueuePriority });
    }

    intrinsic = physicalIntrinsic.createDevice({
        vk::DeviceCreateFlags(),
        boost::numeric_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(),
        0, nullptr,
        boost::numeric_cast<uint32_t>(requiredExtensions.size()), requiredExtensions.data(),
        &(instance->requiredFeatures)
    });

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = physicalIntrinsic;
    allocatorCreateInfo.device = intrinsic;
    vmaCreateAllocator(&allocatorCreateInfo, &allocator);

    uint32_t index = 0;
    for (auto queueFamilyIndexAndCapabilities : queueFamilyIndicesAndCapabilities) {
        let familyIndex = queueFamilyIndexAndCapabilities.first;
        let capabilities = queueFamilyIndexAndCapabilities.second;
        let queue = this->intrinsic.getQueue(familyIndex, index);
        let commandPool = this->intrinsic.createCommandPool({ vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, familyIndex });

        if (capabilities & QUEUE_CAPABILITY_GRAPHICS) {
            graphicsQueueFamilyIndex = familyIndex;
            graphicsQueueIndex = index;
            graphicsQueue = queue;
            graphicsCommandPool = commandPool;
        }
        if (capabilities & QUEUE_CAPABILITY_PRESENT) {
            presentQueueFamilyIndex = familyIndex;
            presentQueueIndex = index;
            presentQueue = queue;
            presentCommandPool = commandPool;
        }
        if (capabilities & QUEUE_CAPABILITY_COMPUTE) {
            computeQueueFamilyIndex = familyIndex;
            computeQueueIndex = index;
            computeQueue = queue;
            graphicsCommandPool = commandPool;
        }
        index++;
    }

    imagePipeline = std::make_unique<PipelineImage::DeviceShared>(dynamic_cast<Device &>(*this));

    Device_base::initializeDevice(window);
}


std::vector<std::pair<uint32_t, uint8_t>> Device_vulkan::findBestQueueFamilyIndices(vk::SurfaceKHR surface) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    LOG_INFO(" - Scoring QueueFamilies");

    // Create a sorted list of queueFamilies depending on the scoring.
    vector<tuple<uint32_t, uint8_t, uint32_t>> queueFamilieScores;
    {
        uint32_t index = 0;
        for (auto queueFamilyProperties : physicalIntrinsic.getQueueFamilyProperties()) {
            uint8_t capabilities = 0;
            if (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
                capabilities |= QUEUE_CAPABILITY_GRAPHICS;
            }
            if (physicalIntrinsic.getSurfaceSupportKHR(index, surface)) {
                capabilities |= QUEUE_CAPABILITY_PRESENT;
            }
            if (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute) {
                capabilities |= QUEUE_CAPABILITY_COMPUTE;
            }

            uint32_t score = 0;
            score += capabilities == QUEUE_CAPABILITY_ALL ? 10 : 0;
            score += capabilities == QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT ? 5 : 0;
            score += capabilities == QUEUE_CAPABILITY_GRAPHICS ? 1 : 0;
            score += capabilities == QUEUE_CAPABILITY_COMPUTE ? 1 : 0;
            score += capabilities == QUEUE_CAPABILITY_PRESENT ? 1 : 0;

            LOG_INFO("    * %i: capabilities=%03b, score=%i", index, capabilities, score);

            queueFamilieScores.push_back({ index, capabilities, score });
            index++;
        }
        sort(queueFamilieScores.begin(), queueFamilieScores.end(), [](const auto &a, const auto &b) {
            return get<2>(a) > get<2>(b);
        });
    }

    // Iterativly add indices if it completes the totalQueueCapabilities.
    vector<pair<uint32_t, uint8_t>> queueFamilyIndicesAndQueueCapabilitiess;
    uint8_t totalCapabilities = 0;
    for (let &[index, capabilities, score] : queueFamilieScores) {
        if ((totalCapabilities & capabilities) != capabilities) {
            queueFamilyIndicesAndQueueCapabilitiess.push_back({ index, capabilities & ~totalCapabilities });
            totalCapabilities |= capabilities;
        }
    }

    return queueFamilyIndicesAndQueueCapabilitiess;
}

int Device_vulkan::score(vk::SurfaceKHR surface) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    auto formats = physicalIntrinsic.getSurfaceFormatsKHR(surface);
    auto presentModes = physicalIntrinsic.getSurfacePresentModesKHR(surface);
    queueFamilyIndicesAndCapabilities = findBestQueueFamilyIndices(surface);

    LOG_INFO("Scoring device: %s", string());
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

    uint8_t deviceCapabilities = 0;
    for (auto queueFamilyIndexAndCapabilities : queueFamilyIndicesAndCapabilities) {
        deviceCapabilities |= queueFamilyIndexAndCapabilities.second;
    }
    LOG_INFO(" - Capabilities=%03b", deviceCapabilities);

    if ((deviceCapabilities & QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT) != QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT) {
        LOG_INFO(" - Does not have both the graphics and compute queues.");
        return -1;

    } else if (!(deviceCapabilities & QUEUE_CAPABILITY_PRESENT)) {
        LOG_INFO(" - Does not have a present queue.");
        return 0;
    }

    // Give score based on colour quality.
    LOG_INFO(" - Surface formats:");
    uint32_t bestSurfaceFormatScore = 0;
    for (auto format : formats) {
        uint32_t score = 0;

        LOG_INFO("    * Found colorSpace=%s, format=%s", vk::to_string(format.colorSpace), vk::to_string(format.format));

        switch (format.colorSpace) {
        case vk::ColorSpaceKHR::eSrgbNonlinear: score += 1; break;
        case vk::ColorSpaceKHR::eExtendedSrgbNonlinearEXT: score += 100; break;
        default: continue;
        }

        switch (format.format) {
        case vk::Format::eR16G16B16A16Sfloat: score += 12; break;
        case vk::Format::eR16G16B16Sfloat: score += 11; break;

        case vk::Format::eR8G8B8A8Srgb: score += 4; break;
        case vk::Format::eB8G8R8A8Srgb: score += 4; break;
        case vk::Format::eR8G8B8Srgb: score += 3; break;
        case vk::Format::eB8G8R8Srgb: score += 3; break;

        case vk::Format::eB8G8R8A8Unorm: score += 2; break;
        case vk::Format::eR8G8B8A8Unorm: score += 2; break;
        case vk::Format::eB8G8R8Unorm: score += 1; break;
        case vk::Format::eR8G8B8Unorm: score += 1; break;
        default: continue;
        }

        LOG_INFO("    * Valid colorSpace=%s, format=%s, score=%d", vk::to_string(format.colorSpace), vk::to_string(format.format), score);

        if (score > bestSurfaceFormatScore) {
            bestSurfaceFormatScore = score;
            bestSurfaceFormat = format;
        }
    }
    auto totalScore = bestSurfaceFormatScore;
    LOG_INFO("    * bestColorSpace=%s, bestFormat=%s, score=%d",
        vk::to_string(bestSurfaceFormat.colorSpace),
        vk::to_string(bestSurfaceFormat.format),
        bestSurfaceFormatScore
    );



    if (bestSurfaceFormatScore == 0) {
        LOG_INFO(" - Does not have a suitable surface format.");
        return 0;
    }

    LOG_INFO(" - Surface present modes:");
    uint32_t bestSurfacePresentModeScore = 0;
    for (let &presentMode : presentModes) {
        uint32_t score = 0;

        LOG_INFO("    * presentMode=%s", vk::to_string(presentMode));

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
    totalScore += bestSurfacePresentModeScore;

    if (totalScore < bestSurfacePresentModeScore) {
        LOG_INFO(" - Does not have a suitable surface present mode.");
        return 0;
    }

    // Give score based on the perfomance of the device.
    let properties = physicalIntrinsic.getProperties();
    LOG_INFO(" - Type of device: %s", vk::to_string(properties.deviceType));
    switch (properties.deviceType) {
    case vk::PhysicalDeviceType::eCpu: totalScore += 1; break;
    case vk::PhysicalDeviceType::eOther: totalScore += 1; break;
    case vk::PhysicalDeviceType::eVirtualGpu: totalScore += 2; break;
    case vk::PhysicalDeviceType::eIntegratedGpu: totalScore += 3; break;
    case vk::PhysicalDeviceType::eDiscreteGpu: totalScore += 4; break;
    }

    return totalScore;
}

int Device_vulkan::score(Window const &window) const {
    auto surface = window.getSurface();
    let s = score(surface);
    instance->destroySurfaceKHR(surface);
    return s;
}

std::pair<vk::Buffer, VmaAllocation> Device_vulkan::createBuffer(const vk::BufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    VkBuffer buffer;
    VmaAllocation allocation;

    let result = static_cast<vk::Result>(vmaCreateBuffer(allocator, &(static_cast<VkBufferCreateInfo>(bufferCreateInfo)), &allocationCreateInfo, &buffer, &allocation, nullptr));

    std::pair<vk::Buffer, VmaAllocation> const value = {buffer, allocation};

    return vk::createResultValue(result, value, "TTauri::GUI::Device_vulkan::createBuffer");
}

void Device_vulkan::destroyBuffer(const vk::Buffer &buffer, const VmaAllocation &allocation) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    vmaDestroyBuffer(allocator, buffer, allocation);
}

std::pair<vk::Image, VmaAllocation> Device_vulkan::createImage(const vk::ImageCreateInfo &imageCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    VkImage image;
    VmaAllocation allocation;

    let result = static_cast<vk::Result>(vmaCreateImage(allocator, &(static_cast<VkImageCreateInfo>(imageCreateInfo)), &allocationCreateInfo, &image, &allocation, nullptr));

    std::pair<vk::Image, VmaAllocation> const value = {image, allocation};

    return vk::createResultValue(result, value, "TTauri::GUI::Device_vulkan::createImage");
}

void Device_vulkan::destroyImage(const vk::Image &image, const VmaAllocation &allocation) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    vmaDestroyImage(allocator, image, allocation);
}

void Device_vulkan::unmapMemory(const VmaAllocation &allocation) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    vmaUnmapMemory(allocator, allocation);
}

vk::CommandBuffer Device_vulkan::beginSingleTimeCommands() const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    let commandBuffers = intrinsic.allocateCommandBuffers({ graphicsCommandPool, vk::CommandBufferLevel::ePrimary, 1 });
    let commandBuffer = commandBuffers.at(0);

    commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    return commandBuffer;
}

void Device_vulkan::endSingleTimeCommands(vk::CommandBuffer commandBuffer) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    commandBuffer.end();

    vector<vk::CommandBuffer> const commandBuffers = {commandBuffer};

    graphicsQueue.submit({{
        0, nullptr, nullptr, // wait semaphores, wait stages
        boost::numeric_cast<uint32_t>(commandBuffers.size()), commandBuffers.data(),
        0, nullptr // signal semaphores
    }}, vk::Fence());

    graphicsQueue.waitIdle();
    intrinsic.freeCommandBuffers(graphicsCommandPool, commandBuffers);
}

static pair<vk::AccessFlags, vk::PipelineStageFlags> accessAndStageFromLayout(vk::ImageLayout layout)
{
    switch (layout) {
    case vk::ImageLayout::eUndefined:
        return { vk::AccessFlags(), vk::PipelineStageFlagBits::eTopOfPipe };

    // GPU Texure Maps
    case vk::ImageLayout::eTransferDstOptimal:
        return { vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTransfer };

    case vk::ImageLayout::eShaderReadOnlyOptimal:
        return { vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eFragmentShader };

    // CPU Staging texture maps
    case vk::ImageLayout::eGeneral:
        return { vk::AccessFlagBits::eHostWrite, vk::PipelineStageFlagBits::eHost };

    case vk::ImageLayout::eTransferSrcOptimal:
        return { vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eTransfer };

    default:
        BOOST_THROW_EXCEPTION(Device_vulkan::ImageLayoutTransitionError());
    }
}

void Device_vulkan::transitionLayout(vk::Image image, vk::Format format, vk::ImageLayout srcLayout, vk::ImageLayout dstLayout) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    let commandBuffer = beginSingleTimeCommands();

    let [srcAccessMask, srcStage] = accessAndStageFromLayout(srcLayout);
    let [dstAccessMask, dstStage] = accessAndStageFromLayout(dstLayout);

    vector<vk::ImageMemoryBarrier> barriers = {{
        srcAccessMask,
        dstAccessMask,
        srcLayout,
        dstLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        image, {
            vk::ImageAspectFlagBits::eColor,
            0, // baseMipLevel
            1, // levelCount
            0, // baseArrayLayer
            1 // layerCount
        }
    }};

    commandBuffer.pipelineBarrier(
        srcStage, dstStage,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        boost::numeric_cast<uint32_t>(barriers.size()), barriers.data()
    );

    endSingleTimeCommands(commandBuffer);
}

void Device_vulkan::copyImage(vk::Image srcImage, vk::ImageLayout srcLayout, vk::Image dstImage, vk::ImageLayout dstLayout, std::vector<vk::ImageCopy> regions) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    let commandBuffer = beginSingleTimeCommands();

    commandBuffer.copyImage(
        srcImage, srcLayout,
        dstImage, dstLayout,
        regions
    );

    endSingleTimeCommands(commandBuffer);
}

vk::ShaderModule Device_vulkan::loadShader(const uint32_t *data, size_t size) const
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    LOG_INFO("Loading shader");

    // Check uint32_t alignment of pointer.
    BOOST_ASSERT((reinterpret_cast<std::uintptr_t>(data) & 3) == 0);

    return intrinsic.createShaderModule({vk::ShaderModuleCreateFlags(), size, data});
}

}
