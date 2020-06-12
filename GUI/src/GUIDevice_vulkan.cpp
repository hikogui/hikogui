// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/GUIDevice_vulkan.hpp"
#include "TTauri/GUI/GUISystem.hpp"
#include "TTauri/GUI/PipelineImage.hpp"
#include "TTauri/GUI/PipelineImage_DeviceShared.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/Foundation/ResourceView.hpp"
#include <nonstd/span>

namespace tt {

#define QUEUE_CAPABILITY_GRAPHICS 1
#define QUEUE_CAPABILITY_COMPUTE 2
#define QUEUE_CAPABILITY_PRESENT 4
#define QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT (QUEUE_CAPABILITY_GRAPHICS | QUEUE_CAPABILITY_PRESENT)
#define QUEUE_CAPABILITY_ALL (QUEUE_CAPABILITY_GRAPHICS | QUEUE_CAPABILITY_COMPUTE | QUEUE_CAPABILITY_PRESENT)

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

GUIDevice_vulkan::GUIDevice_vulkan(vk::PhysicalDevice physicalDevice) :
    GUIDevice_base(),
    physicalIntrinsic(std::move(physicalDevice))
{
    auto result = physicalIntrinsic.getProperties2KHR<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceIDProperties>(guiSystem->loader());
    auto resultDeviceProperties2 = result.get<vk::PhysicalDeviceProperties2>();
    auto resultDeviceIDProperties = result.get<vk::PhysicalDeviceIDProperties>();

    requiredExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);

    deviceID = resultDeviceProperties2.properties.deviceID;
    vendorID = resultDeviceProperties2.properties.vendorID;
    deviceName = std::string(resultDeviceProperties2.properties.deviceName);
    deviceUUID = uuid::fromBigEndian(resultDeviceIDProperties.deviceUUID);

    physicalProperties = physicalIntrinsic.getProperties();
}

GUIDevice_vulkan::~GUIDevice_vulkan()
{
    try {
        gsl_suppress(f.6) {
            toneMapperPipeline->destroy(this);
            toneMapperPipeline = nullptr;
            SDFPipeline->destroy(this);
            SDFPipeline = nullptr;
            imagePipeline->destroy(this);
            imagePipeline = nullptr;
            boxPipeline->destroy(this);
            boxPipeline = nullptr;
            flatPipeline->destroy(this);
            flatPipeline = nullptr;

            destroyQuadIndexBuffer();

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

void GUIDevice_vulkan::initializeDevice(Window const &window)
{
    auto lock = std::scoped_lock(guiMutex);

    const float defaultQueuePriority = 1.0;

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    for (auto queueFamilyIndexAndCapabilities : queueFamilyIndicesAndCapabilities) {
        auto index = queueFamilyIndexAndCapabilities.first;
        deviceQueueCreateInfos.push_back({ vk::DeviceQueueCreateFlags(), index, 1, &defaultQueuePriority });
    }

    intrinsic = physicalIntrinsic.createDevice({
        vk::DeviceCreateFlags(),
        numeric_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(),
        0, nullptr,
        numeric_cast<uint32_t>(requiredExtensions.size()), requiredExtensions.data(),
        &(guiSystem->requiredFeatures)
    });

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = physicalIntrinsic;
    allocatorCreateInfo.device = intrinsic;
    allocatorCreateInfo.instance = guiSystem->intrinsic;
    vmaCreateAllocator(&allocatorCreateInfo, &allocator);

    VmaAllocationCreateInfo lazyAllocationInfo = {};
    lazyAllocationInfo.usage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
    uint32_t typeIndexOut = 0;
    supportsLazyTransientImages =
        vmaFindMemoryTypeIndex(allocator, 0, &lazyAllocationInfo, &typeIndexOut) != VK_ERROR_FEATURE_NOT_PRESENT;
    
    if (supportsLazyTransientImages) {
        lazyMemoryUsage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
        transientImageUsageFlags = vk::ImageUsageFlagBits::eTransientAttachment;
    }

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

    initializeQuadIndexBuffer();

    flatPipeline = std::make_unique<PipelineFlat::DeviceShared>(static_cast<GUIDevice &>(*this));
    boxPipeline = std::make_unique<PipelineBox::DeviceShared>(static_cast<GUIDevice &>(*this));
    imagePipeline = std::make_unique<PipelineImage::DeviceShared>(static_cast<GUIDevice &>(*this));
    SDFPipeline = std::make_unique<PipelineSDF::DeviceShared>(static_cast<GUIDevice &>(*this));
    toneMapperPipeline = std::make_unique<PipelineToneMapper::DeviceShared>(static_cast<GUIDevice &>(*this));

    GUIDevice_base::initializeDevice(window);
}

void GUIDevice_vulkan::initializeQuadIndexBuffer()
{
    using vertex_index_type = uint16_t;
    constexpr ssize_t maximum_number_of_vertices = 1 << (sizeof(vertex_index_type) * CHAR_BIT);
    constexpr ssize_t maximum_number_of_quads = maximum_number_of_vertices / 4;
    constexpr ssize_t maximum_number_of_triangles = maximum_number_of_quads * 2;
    constexpr ssize_t maximum_number_of_indices = maximum_number_of_triangles * 3;

    // Create vertex index buffer
    {
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (vertex_index_type) * maximum_number_of_indices,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        std::tie(quadIndexBuffer, quadIndexBufferAllocation) = createBuffer(bufferCreateInfo, allocationCreateInfo);
    }

    // Fill in the vertex index buffer, using a staging buffer, then copying.
    {
        // Create staging vertex index buffer.
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (vertex_index_type) * maximum_number_of_indices,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        let [stagingVertexIndexBuffer, stagingVertexIndexBufferAllocation] = createBuffer(bufferCreateInfo, allocationCreateInfo);

        // Initialize indices.
        let stagingVertexIndexBufferData = mapMemory<vertex_index_type>(stagingVertexIndexBufferAllocation);
        for (size_t i = 0; i < maximum_number_of_indices; i++) {
            let vertexInRectangle = i % 6;
            let rectangleNr = i / 6;
            let rectangleBase = rectangleNr * 4;

            switch (vertexInRectangle) {
            case 0: stagingVertexIndexBufferData[i] = numeric_cast<vertex_index_type>(rectangleBase + 0); break;
            case 1: stagingVertexIndexBufferData[i] = numeric_cast<vertex_index_type>(rectangleBase + 1); break;
            case 2: stagingVertexIndexBufferData[i] = numeric_cast<vertex_index_type>(rectangleBase + 2); break;
            case 3: stagingVertexIndexBufferData[i] = numeric_cast<vertex_index_type>(rectangleBase + 2); break;
            case 4: stagingVertexIndexBufferData[i] = numeric_cast<vertex_index_type>(rectangleBase + 1); break;
            case 5: stagingVertexIndexBufferData[i] = numeric_cast<vertex_index_type>(rectangleBase + 3); break;
            default: no_default;
            }
        }
        flushAllocation(stagingVertexIndexBufferAllocation, 0, VK_WHOLE_SIZE);
        unmapMemory(stagingVertexIndexBufferAllocation);

        // Copy indices to vertex index buffer.
        auto commands = allocateCommandBuffers({
            graphicsCommandPool, 
            vk::CommandBufferLevel::ePrimary, 
            1
            }).at(0);
        commands.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        commands.copyBuffer(stagingVertexIndexBuffer, quadIndexBuffer, {{0, 0, sizeof (vertex_index_type) * maximum_number_of_indices}});
        commands.end();

        std::vector<vk::CommandBuffer> const commandBuffersToSubmit = { commands };
        std::vector<vk::SubmitInfo> const submitInfo = { { 0, nullptr, nullptr, numeric_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(), 0, nullptr } };
        graphicsQueue.submit(submitInfo, vk::Fence());
        graphicsQueue.waitIdle();

        freeCommandBuffers(graphicsCommandPool, {commands});
        destroyBuffer(stagingVertexIndexBuffer, stagingVertexIndexBufferAllocation);
    }
}

void GUIDevice_vulkan::destroyQuadIndexBuffer()
{
    destroyBuffer(quadIndexBuffer, quadIndexBufferAllocation);
}

std::vector<std::pair<uint32_t, uint8_t>> GUIDevice_vulkan::findBestQueueFamilyIndices(vk::SurfaceKHR surface) const
{
    auto lock = std::scoped_lock(guiMutex);

    LOG_INFO(" - Scoring QueueFamilies");

    // Create a sorted list of queueFamilies depending on the scoring.
    std::vector<std::tuple<uint32_t, uint8_t, uint32_t>> queueFamilieScores;
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

            LOG_INFO("    * {}: capabilities={:03b}, score={}", index, capabilities, score);

            queueFamilieScores.push_back({ index, capabilities, score });
            index++;
        }
        sort(queueFamilieScores.begin(), queueFamilieScores.end(), [](const auto &a, const auto &b) {
            return std::get<2>(a) > std::get<2>(b);
        });
    }

    // Iteratively add indices if it completes the totalQueueCapabilities.
    std::vector<std::pair<uint32_t, uint8_t>> queueFamilyIndicesAndQueueCapabilitiess;
    uint8_t totalCapabilities = 0;
    for (let &[index, capabilities, score] : queueFamilieScores) {
        if ((totalCapabilities & capabilities) != capabilities) {
            queueFamilyIndicesAndQueueCapabilitiess.emplace_back(
                index,
                static_cast<uint8_t>(capabilities & ~totalCapabilities)
            );
            totalCapabilities |= capabilities;
        }
    }

    return queueFamilyIndicesAndQueueCapabilitiess;
}

int GUIDevice_vulkan::score(vk::SurfaceKHR surface) const
{
    auto lock = std::scoped_lock(guiMutex);

    auto formats = physicalIntrinsic.getSurfaceFormatsKHR(surface);
    auto presentModes = physicalIntrinsic.getSurfacePresentModesKHR(surface);
    queueFamilyIndicesAndCapabilities = findBestQueueFamilyIndices(surface);

    LOG_INFO("Scoring device: {}", string());
    if (!hasRequiredFeatures(physicalIntrinsic, guiSystem->requiredFeatures)) {
        LOG_INFO(" - Does not have the required features.");
        return -1;
    }

    if (!meetsRequiredLimits(physicalIntrinsic, guiSystem->requiredLimits)) {
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
    LOG_INFO(" - Capabilities={:03b}", deviceCapabilities);

    if ((deviceCapabilities & QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT) != QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT) {
        LOG_INFO(" - Does not have both the graphics and compute queues.");
        return -1;

    } else if (!(deviceCapabilities & QUEUE_CAPABILITY_PRESENT)) {
        LOG_INFO(" - Does not have a present queue.");
        return 0;
    }

    // Give score based on color quality.
    LOG_INFO(" - Surface formats:");
    uint32_t bestSurfaceFormatScore = 0;
    for (auto format : formats) {
        uint32_t score = 0;

        LOG_INFO("    * Found colorSpace={}, format={}", vk::to_string(format.colorSpace), vk::to_string(format.format));

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

        LOG_INFO("    * Valid colorSpace={}, format={}, score={}", vk::to_string(format.colorSpace), vk::to_string(format.format), score);

        if (score > bestSurfaceFormatScore) {
            bestSurfaceFormatScore = score;
            bestSurfaceFormat = format;
        }
    }
    auto totalScore = bestSurfaceFormatScore;
    LOG_INFO("    * bestColorSpace={}, bestFormat={}, score={}",
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

        LOG_INFO("    * presentMode={}", vk::to_string(presentMode));

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

    // Give score based on the performance of the device.
    let properties = physicalIntrinsic.getProperties();
    LOG_INFO(" - Type of device: {}", vk::to_string(properties.deviceType));
    switch (properties.deviceType) {
    case vk::PhysicalDeviceType::eCpu: totalScore += 1; break;
    case vk::PhysicalDeviceType::eOther: totalScore += 1; break;
    case vk::PhysicalDeviceType::eVirtualGpu: totalScore += 2; break;
    case vk::PhysicalDeviceType::eIntegratedGpu: totalScore += 3; break;
    case vk::PhysicalDeviceType::eDiscreteGpu: totalScore += 4; break;
    }

    return totalScore;
}

int GUIDevice_vulkan::score(Window const &window) const {
    auto surface = window.getSurface();
    let s = score(surface);
    guiSystem->destroySurfaceKHR(surface);
    return s;
}

std::pair<vk::Buffer, VmaAllocation> GUIDevice_vulkan::createBuffer(const vk::BufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const
{
    auto lock = std::scoped_lock(guiMutex);

    VkBuffer buffer;
    VmaAllocation allocation;

    let bufferCreateInfo_ = static_cast<VkBufferCreateInfo>(bufferCreateInfo);
    let result = static_cast<vk::Result>(vmaCreateBuffer(allocator, &bufferCreateInfo_, &allocationCreateInfo, &buffer, &allocation, nullptr));

    std::pair<vk::Buffer, VmaAllocation> const value = {buffer, allocation};

    return vk::createResultValue(result, value, "tt::GUIDevice_vulkan::createBuffer");
}

void GUIDevice_vulkan::destroyBuffer(const vk::Buffer &buffer, const VmaAllocation &allocation) const
{
    auto lock = std::scoped_lock(guiMutex);

    vmaDestroyBuffer(allocator, buffer, allocation);
}

std::pair<vk::Image, VmaAllocation> GUIDevice_vulkan::createImage(const vk::ImageCreateInfo &imageCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const
{
    auto lock = std::scoped_lock(guiMutex);

    VkImage image;
    VmaAllocation allocation;

    let imageCreateInfo_ = static_cast<VkImageCreateInfo>(imageCreateInfo);
    let result = static_cast<vk::Result>(vmaCreateImage(allocator, &imageCreateInfo_, &allocationCreateInfo, &image, &allocation, nullptr));

    std::pair<vk::Image, VmaAllocation> const value = {image, allocation};

    return vk::createResultValue(result, value, "tt::GUIDevice_vulkan::createImage");
}

void GUIDevice_vulkan::destroyImage(const vk::Image &image, const VmaAllocation &allocation) const
{
    auto lock = std::scoped_lock(guiMutex);

    vmaDestroyImage(allocator, image, allocation);
}

void GUIDevice_vulkan::unmapMemory(const VmaAllocation &allocation) const
{
    auto lock = std::scoped_lock(guiMutex);

    vmaUnmapMemory(allocator, allocation);
}

vk::CommandBuffer GUIDevice_vulkan::beginSingleTimeCommands() const
{
    auto lock = std::scoped_lock(guiMutex);

    let commandBuffers = intrinsic.allocateCommandBuffers({ graphicsCommandPool, vk::CommandBufferLevel::ePrimary, 1 });
    let commandBuffer = commandBuffers.at(0);

    commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    return commandBuffer;
}

void GUIDevice_vulkan::endSingleTimeCommands(vk::CommandBuffer commandBuffer) const
{
    auto lock = std::scoped_lock(guiMutex);

    commandBuffer.end();

    std::vector<vk::CommandBuffer> const commandBuffers = {commandBuffer};

    graphicsQueue.submit({{
        0, nullptr, nullptr, // wait semaphores, wait stages
        numeric_cast<uint32_t>(commandBuffers.size()), commandBuffers.data(),
        0, nullptr // signal semaphores
    }}, vk::Fence());

    graphicsQueue.waitIdle();
    intrinsic.freeCommandBuffers(graphicsCommandPool, commandBuffers);
}

static std::pair<vk::AccessFlags, vk::PipelineStageFlags> accessAndStageFromLayout(vk::ImageLayout layout) noexcept
{
    switch (layout) {
    case vk::ImageLayout::eUndefined:
        return { vk::AccessFlags(), vk::PipelineStageFlagBits::eTopOfPipe };

    // GPU Texture Maps
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
        no_default;
    }
}

void GUIDevice_vulkan::transitionLayout(vk::Image image, vk::Format format, vk::ImageLayout srcLayout, vk::ImageLayout dstLayout) const
{
    auto lock = std::scoped_lock(guiMutex);

    let commandBuffer = beginSingleTimeCommands();

    let [srcAccessMask, srcStage] = accessAndStageFromLayout(srcLayout);
    let [dstAccessMask, dstStage] = accessAndStageFromLayout(dstLayout);

    std::vector<vk::ImageMemoryBarrier> barriers = {{
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
        numeric_cast<uint32_t>(barriers.size()), barriers.data()
    );

    endSingleTimeCommands(commandBuffer);
}

void GUIDevice_vulkan::copyImage(vk::Image srcImage, vk::ImageLayout srcLayout, vk::Image dstImage, vk::ImageLayout dstLayout, vk::ArrayProxy<vk::ImageCopy const> regions) const
{
    auto lock = std::scoped_lock(guiMutex);

    let commandBuffer = beginSingleTimeCommands();

    commandBuffer.copyImage(
        srcImage, srcLayout,
        dstImage, dstLayout,
        regions
    );

    endSingleTimeCommands(commandBuffer);
}

void GUIDevice_vulkan::clearColorImage(vk::Image image, vk::ImageLayout layout, vk::ClearColorValue const &color, vk::ArrayProxy<const vk::ImageSubresourceRange> ranges) const
{
    auto lock = std::scoped_lock(guiMutex);

    let commandBuffer = beginSingleTimeCommands();

    commandBuffer.clearColorImage(
        image,
        layout,
        color,
        ranges
    );

    endSingleTimeCommands(commandBuffer);
}


vk::ShaderModule GUIDevice_vulkan::loadShader(uint32_t const *data, size_t size) const
{
    auto lock = std::scoped_lock(guiMutex);

    LOG_INFO("Loading shader");

    // Check uint32_t alignment of pointer.
    ttauri_assume((reinterpret_cast<std::uintptr_t>(data) & 3) == 0);

    return intrinsic.createShaderModule({vk::ShaderModuleCreateFlags(), size, data});
}

vk::ShaderModule GUIDevice_vulkan::loadShader(nonstd::span<std::byte const> shaderObjectBytes) const
{
    // Make sure the address is aligned to uint32_t;
    let address = reinterpret_cast<uintptr_t>(shaderObjectBytes.data());
    ttauri_assert((address & 2) == 0);

    let shaderObjectBytes32 = reinterpret_cast<uint32_t const *>(shaderObjectBytes.data());
    return loadShader(shaderObjectBytes32, shaderObjectBytes.size());
}

vk::ShaderModule GUIDevice_vulkan::loadShader(URL const &shaderObjectLocation) const
{
    auto shaderObjectView = ResourceView::loadView(shaderObjectLocation);
    return loadShader(shaderObjectView->bytes());
}

}
