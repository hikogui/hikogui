// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_device_vulkan.hpp"
#include "gui_system_vulkan.hpp"
#include "pipeline_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "gui_window.hpp"
#include "../application.hpp"
#include <span>

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
        availableExtensions.insert(std::string(availableExtensionProperties.extensionName.data()));
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
    ttlet availableFeatures = physicalDevice.getFeatures();
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

gui_device_vulkan::gui_device_vulkan(gui_system &system, vk::PhysicalDevice physicalDevice) :
    gui_device(system),
    physicalIntrinsic(std::move(physicalDevice))
{
    auto result = physicalIntrinsic.getProperties2KHR<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceIDProperties>(
        narrow_cast<gui_system_vulkan &>(system).loader()
    );

    auto resultDeviceProperties2 = result.get<vk::PhysicalDeviceProperties2>();
    auto resultDeviceIDProperties = result.get<vk::PhysicalDeviceIDProperties>();

    requiredExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);

    deviceID = resultDeviceProperties2.properties.deviceID;
    vendorID = resultDeviceProperties2.properties.vendorID;
    deviceName = std::string(resultDeviceProperties2.properties.deviceName.data());
    deviceUUID = uuid::fromBigEndian(resultDeviceIDProperties.deviceUUID);

    physicalProperties = physicalIntrinsic.getProperties();
}

gui_device_vulkan::~gui_device_vulkan()
{
    try {
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

        destroy_quad_index_buffer();

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

    } catch (std::exception const &e) {
        tt_log_fatal("Could not properly destruct gui_device_vulkan. '{}'", e.what());
    }
}

void gui_device_vulkan::initialize_device(gui_window const &window)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    const float defaultQueuePriority = 1.0;

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    for (auto queueFamilyIndexAndCapabilities : queueFamilyIndicesAndCapabilities) {
        auto index = queueFamilyIndexAndCapabilities.first;
        deviceQueueCreateInfos.push_back({ vk::DeviceQueueCreateFlags(), index, 1, &defaultQueuePriority });
    }

    intrinsic = physicalIntrinsic.createDevice({
        vk::DeviceCreateFlags(),
        narrow_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(),
        0, nullptr,
        narrow_cast<uint32_t>(requiredExtensions.size()), requiredExtensions.data(),
         &(narrow_cast<gui_system_vulkan&>(system).requiredFeatures)
    });

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = physicalIntrinsic;
    allocatorCreateInfo.device = intrinsic;
    allocatorCreateInfo.instance = narrow_cast<gui_system_vulkan &>(system).intrinsic;
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
        ttlet familyIndex = queueFamilyIndexAndCapabilities.first;
        ttlet capabilities = queueFamilyIndexAndCapabilities.second;
        ttlet queue = this->intrinsic.getQueue(familyIndex, index);
        ttlet commandPool = this->intrinsic.createCommandPool({ vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, familyIndex });

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

    initialize_quad_index_buffer();

    flatPipeline = std::make_unique<pipeline_flat::device_shared>(*this);
    boxPipeline = std::make_unique<pipeline_box::device_shared>(*this);
    imagePipeline = std::make_unique<pipeline_image::device_shared>(*this);
    SDFPipeline = std::make_unique<pipeline_SDF::device_shared>(*this);
    toneMapperPipeline = std::make_unique<pipeline_tone_mapper::device_shared>(*this);

    gui_device::initialize_device(window);
}

void gui_device_vulkan::initialize_quad_index_buffer()
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

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
        ttlet [stagingvertexIndexBuffer, stagingvertexIndexBufferAllocation] = createBuffer(bufferCreateInfo, allocationCreateInfo);

        // Initialize indices.
        ttlet stagingvertexIndexBufferData = mapMemory<vertex_index_type>(stagingvertexIndexBufferAllocation);
        for (size_t i = 0; i < maximum_number_of_indices; i++) {
            ttlet vertexInRectangle = i % 6;
            ttlet rectangleNr = i / 6;
            ttlet rectangleBase = rectangleNr * 4;

            switch (vertexInRectangle) {
            case 0: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 0); break;
            case 1: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 1); break;
            case 2: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 2); break;
            case 3: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 2); break;
            case 4: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 1); break;
            case 5: stagingvertexIndexBufferData[i] = narrow_cast<vertex_index_type>(rectangleBase + 3); break;
            default: tt_no_default();
            }
        }
        flushAllocation(stagingvertexIndexBufferAllocation, 0, VK_WHOLE_SIZE);
        unmapMemory(stagingvertexIndexBufferAllocation);

        // Copy indices to vertex index buffer.
        auto commands = allocateCommandBuffers({
            graphicsCommandPool, 
            vk::CommandBufferLevel::ePrimary, 
            1
            }).at(0);
        commands.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        commands.copyBuffer(stagingvertexIndexBuffer, quadIndexBuffer, {{0, 0, sizeof (vertex_index_type) * maximum_number_of_indices}});
        commands.end();

        std::vector<vk::CommandBuffer> const commandBuffersToSubmit = { commands };
        std::vector<vk::SubmitInfo> const submitInfo = { { 0, nullptr, nullptr, narrow_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(), 0, nullptr } };
        graphicsQueue.submit(submitInfo, vk::Fence());
        graphicsQueue.waitIdle();

        freeCommandBuffers(graphicsCommandPool, {commands});
        destroyBuffer(stagingvertexIndexBuffer, stagingvertexIndexBufferAllocation);
    }
}

void gui_device_vulkan::destroy_quad_index_buffer()
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    destroyBuffer(quadIndexBuffer, quadIndexBufferAllocation);
}

std::vector<std::pair<uint32_t, uint8_t>> gui_device_vulkan::find_best_queue_family_indices(vk::SurfaceKHR surface) const
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    tt_log_info(" - Scoring QueueFamilies");

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

            tt_log_info("    * {}: capabilities={:03b}, score={}", index, capabilities, score);

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
    for (ttlet &[index, capabilities, score] : queueFamilieScores) {
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

int gui_device_vulkan::score(vk::SurfaceKHR surface) const
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    auto formats = physicalIntrinsic.getSurfaceFormatsKHR(surface);
    auto presentModes = physicalIntrinsic.getSurfacePresentModesKHR(surface);
    queueFamilyIndicesAndCapabilities = find_best_queue_family_indices(surface);

    tt_log_info("Scoring device: {}", string());
    if (!hasRequiredFeatures(physicalIntrinsic, narrow_cast<gui_system_vulkan &>(system).requiredFeatures)) {
        tt_log_info(" - Does not have the required features.");
        return -1;
    }

    if (!meetsRequiredLimits(physicalIntrinsic, narrow_cast<gui_system_vulkan &>(system).requiredLimits)) {
        tt_log_info(" - Does not meet the required limits.");
        return -1;
    }

    if (!hasRequiredExtensions(physicalIntrinsic, requiredExtensions)) {
        tt_log_info(" - Does not have the required extensions.");
        return -1;
    }

    uint8_t deviceCapabilities = 0;
    for (auto queueFamilyIndexAndCapabilities : queueFamilyIndicesAndCapabilities) {
        deviceCapabilities |= queueFamilyIndexAndCapabilities.second;
    }
    tt_log_info(" - Capabilities={:03b}", deviceCapabilities);

    if ((deviceCapabilities & QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT) != QUEUE_CAPABILITY_GRAPHICS_AND_PRESENT) {
        tt_log_info(" - Does not have both the graphics and compute queues.");
        return -1;

    } else if (!(deviceCapabilities & QUEUE_CAPABILITY_PRESENT)) {
        tt_log_info(" - Does not have a present queue.");
        return 0;
    }

    // Give score based on color quality.
    tt_log_info(" - Surface formats:");
    uint32_t bestSurfaceFormatScore = 0;
    for (auto format : formats) {
        uint32_t score = 0;

        tt_log_info("    * Found colorSpace={}, format={}", vk::to_string(format.colorSpace), vk::to_string(format.format));

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

        tt_log_info("    * Valid colorSpace={}, format={}, score={}", vk::to_string(format.colorSpace), vk::to_string(format.format), score);

        if (score > bestSurfaceFormatScore) {
            bestSurfaceFormatScore = score;
            bestSurfaceFormat = format;
        }
    }
    auto totalScore = bestSurfaceFormatScore;
    tt_log_info("    * bestColorSpace={}, bestFormat={}, score={}",
        vk::to_string(bestSurfaceFormat.colorSpace),
        vk::to_string(bestSurfaceFormat.format),
        bestSurfaceFormatScore
    );



    if (bestSurfaceFormatScore == 0) {
        tt_log_info(" - Does not have a suitable surface format.");
        return 0;
    }

    tt_log_info(" - Surface present modes:");
    uint32_t bestSurfacePresentModeScore = 0;
    for (ttlet &presentMode : presentModes) {
        uint32_t score = 0;

        tt_log_info("    * presentMode={}", vk::to_string(presentMode));

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
        tt_log_info(" - Does not have a suitable surface present mode.");
        return 0;
    }

    // Give score based on the performance of the device.
    ttlet properties = physicalIntrinsic.getProperties();
    tt_log_info(" - Type of device: {}", vk::to_string(properties.deviceType));
    switch (properties.deviceType) {
    case vk::PhysicalDeviceType::eCpu: totalScore += 1; break;
    case vk::PhysicalDeviceType::eOther: totalScore += 1; break;
    case vk::PhysicalDeviceType::eVirtualGpu: totalScore += 2; break;
    case vk::PhysicalDeviceType::eIntegratedGpu: totalScore += 3; break;
    case vk::PhysicalDeviceType::eDiscreteGpu: totalScore += 4; break;
    }

    return totalScore;
}

int gui_device_vulkan::score(gui_window const &window) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    auto surface = narrow_cast<gui_window_vulkan const&>(window).getSurface();
    ttlet s = score(surface);
    narrow_cast<gui_system_vulkan &>(system).destroySurfaceKHR(surface);
    return s;
}

std::pair<vk::Buffer, VmaAllocation> gui_device_vulkan::createBuffer(const vk::BufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    VkBuffer buffer;
    VmaAllocation allocation;

    ttlet bufferCreateInfo_ = static_cast<VkBufferCreateInfo>(bufferCreateInfo);
    ttlet result = static_cast<vk::Result>(vmaCreateBuffer(allocator, &bufferCreateInfo_, &allocationCreateInfo, &buffer, &allocation, nullptr));

    std::pair<vk::Buffer, VmaAllocation> const value = {buffer, allocation};

    return vk::createResultValue(result, value, "tt::gui_device_vulkan::createBuffer");
}

void gui_device_vulkan::destroyBuffer(const vk::Buffer &buffer, const VmaAllocation &allocation) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    vmaDestroyBuffer(allocator, buffer, allocation);
}

std::pair<vk::Image, VmaAllocation> gui_device_vulkan::createImage(const vk::ImageCreateInfo &imageCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    VkImage image;
    VmaAllocation allocation;

    ttlet imageCreateInfo_ = static_cast<VkImageCreateInfo>(imageCreateInfo);
    ttlet result = static_cast<vk::Result>(vmaCreateImage(allocator, &imageCreateInfo_, &allocationCreateInfo, &image, &allocation, nullptr));

    std::pair<vk::Image, VmaAllocation> const value = {image, allocation};

    return vk::createResultValue(result, value, "tt::gui_device_vulkan::createImage");
}

void gui_device_vulkan::destroyImage(const vk::Image &image, const VmaAllocation &allocation) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    vmaDestroyImage(allocator, image, allocation);
}

void gui_device_vulkan::unmapMemory(const VmaAllocation &allocation) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    vmaUnmapMemory(allocator, allocation);
}

vk::CommandBuffer gui_device_vulkan::beginSingleTimeCommands() const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    ttlet commandBuffers = intrinsic.allocateCommandBuffers({ graphicsCommandPool, vk::CommandBufferLevel::ePrimary, 1 });
    ttlet commandBuffer = commandBuffers.at(0);

    commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    return commandBuffer;
}

void gui_device_vulkan::endSingleTimeCommands(vk::CommandBuffer commandBuffer) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    commandBuffer.end();

    std::vector<vk::CommandBuffer> const commandBuffers = {commandBuffer};

    graphicsQueue.submit({{
        0, nullptr, nullptr, // wait semaphores, wait stages
        narrow_cast<uint32_t>(commandBuffers.size()), commandBuffers.data(),
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
        tt_no_default();
    }
}

void gui_device_vulkan::transitionLayout(vk::Image image, vk::Format format, vk::ImageLayout srcLayout, vk::ImageLayout dstLayout) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    ttlet commandBuffer = beginSingleTimeCommands();

    ttlet [srcAccessMask, srcStage] = accessAndStageFromLayout(srcLayout);
    ttlet [dstAccessMask, dstStage] = accessAndStageFromLayout(dstLayout);

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
        narrow_cast<uint32_t>(barriers.size()), barriers.data()
    );

    endSingleTimeCommands(commandBuffer);
}

void gui_device_vulkan::copyImage(vk::Image srcImage, vk::ImageLayout srcLayout, vk::Image dstImage, vk::ImageLayout dstLayout, vk::ArrayProxy<vk::ImageCopy const> regions) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    ttlet commandBuffer = beginSingleTimeCommands();

    commandBuffer.copyImage(
        srcImage, srcLayout,
        dstImage, dstLayout,
        regions
    );

    endSingleTimeCommands(commandBuffer);
}

void gui_device_vulkan::clearColorImage(vk::Image image, vk::ImageLayout layout, vk::ClearColorValue const &color, vk::ArrayProxy<const vk::ImageSubresourceRange> ranges) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    ttlet commandBuffer = beginSingleTimeCommands();

    commandBuffer.clearColorImage(
        image,
        layout,
        color,
        ranges
    );

    endSingleTimeCommands(commandBuffer);
}


vk::ShaderModule gui_device_vulkan::loadShader(uint32_t const *data, size_t size) const
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    tt_log_info("Loading shader");

    // Check uint32_t alignment of pointer.
    tt_axiom((reinterpret_cast<std::uintptr_t>(data) & 3) == 0);

    return intrinsic.createShaderModule({vk::ShaderModuleCreateFlags(), size, data});
}

vk::ShaderModule gui_device_vulkan::loadShader(std::span<std::byte const> shaderObjectBytes) const
{
    // no lock, only local variable.

    // Make sure the address is aligned to uint32_t;
    ttlet address = reinterpret_cast<uintptr_t>(shaderObjectBytes.data());
    tt_assert((address & 2) == 0);

    ttlet shaderObjectBytes32 = reinterpret_cast<uint32_t const *>(shaderObjectBytes.data());
    return loadShader(shaderObjectBytes32, shaderObjectBytes.size());
}

vk::ShaderModule gui_device_vulkan::loadShader(URL const &shaderObjectLocation) const
{
    // no lock, only local variable.

    return loadShader(*shaderObjectLocation.loadView());
}

}
