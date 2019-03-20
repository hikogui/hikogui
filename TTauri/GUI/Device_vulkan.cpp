
#include "Device_vulkan.hpp"

#include "Instance_vulkan.hpp"
#include "Window_vulkan.hpp"
#include "vulkan_utils.hpp"

#include "TTauri/Logging.hpp"

#include <boost/range/combine.hpp>
//#include <gsl/gsl>

namespace TTauri { namespace GUI {

using namespace std;

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

Device_vulkan::Device_vulkan(vk::PhysicalDevice physicalDevice) :
    Device(),
    physicalIntrinsic(physicalDevice)
{
    auto result = physicalIntrinsic.getProperties2KHR<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceIDProperties>(get_singleton<Instance_vulkan>()->loader);

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

void Device_vulkan::initializeDevice(std::shared_ptr<Window> window)
{
    const float defaultQueuePriority = 1.0;

    vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    for (auto queueFamilyIndexAndCapabilities : queueFamilyIndicesAndCapabilities) {
        auto index = queueFamilyIndexAndCapabilities.first;

        deviceQueueCreateInfos.push_back({ vk::DeviceQueueCreateFlags(), index, 1, &defaultQueuePriority });
    }

    auto deviceCreateInfo = vk::DeviceCreateInfo();
    deviceCreateInfo.setPEnabledFeatures(&(get_singleton<Instance_vulkan>()->requiredFeatures));
    setQueueCreateInfos(deviceCreateInfo, deviceQueueCreateInfos);
    setExtensionNames(deviceCreateInfo, requiredExtensions);
    setLayerNames(deviceCreateInfo, get_singleton<Instance_vulkan>()->requiredLayers);

    intrinsic = physicalIntrinsic.createDevice(deviceCreateInfo);

    uint32_t index = 0;
    for (auto queueFamilyIndexAndCapabilities : queueFamilyIndicesAndCapabilities) {
        auto const familyIndex = queueFamilyIndexAndCapabilities.first;
        auto const capabilities = queueFamilyIndexAndCapabilities.second;
        auto const queue = this->intrinsic.getQueue(familyIndex, index);
        auto const commandPool = this->intrinsic.createCommandPool({ vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                                     familyIndex });

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

    Device::initializeDevice(window);
}

static bool scoreIsGreater(const tuple<uint32_t, uint8_t, uint32_t> &a, const tuple<uint32_t, uint8_t, uint32_t> &b)
{
    return get<2>(a) > get<2>(b);
}

std::vector<std::pair<uint32_t, uint8_t>> Device_vulkan::findBestQueueFamilyIndices(std::shared_ptr<Window> _window)
{
    auto window = std::dynamic_pointer_cast<Window_vulkan>(_window);
    if (!window) {
        BOOST_THROW_EXCEPTION(NonVulkanWindowError());
    }

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
            if (physicalIntrinsic.getSurfaceSupportKHR(index, window->intrinsic)) {
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

            LOG_INFO("    * %i: capabilities=%03b, score=%i") % index % capabilities % score;

            queueFamilieScores.push_back({ index, capabilities, score });
            index++;
        }
        sort(queueFamilieScores.begin(), queueFamilieScores.end(), scoreIsGreater);
    }

    // Iterativly add indices if it completes the totalQueueCapabilities.
    vector<pair<uint32_t, uint8_t>> queueFamilyIndicesAndQueueCapabilitiess;
    uint8_t totalCapabilities = 0;
    for (auto const &queueFamilyScore : queueFamilieScores) {
        auto const index = get<0>(queueFamilyScore);
        auto const capabilities = get<1>(queueFamilyScore);

        if ((totalCapabilities & capabilities) != capabilities) {
            queueFamilyIndicesAndQueueCapabilitiess.push_back({ index, capabilities & ~totalCapabilities });
            totalCapabilities |= capabilities;
        }
    }

    return queueFamilyIndicesAndQueueCapabilitiess;
}

int Device_vulkan::score(std::shared_ptr<Window> _window)
{
    auto window = std::dynamic_pointer_cast<Window_vulkan>(_window);
    if (!window) {
        BOOST_THROW_EXCEPTION(NonVulkanWindowError());
    }

    LOG_INFO("Scoring device: %s") % str();
    if (!hasRequiredFeatures(physicalIntrinsic, get_singleton<Instance_vulkan>()->requiredFeatures)) {
        LOG_INFO(" - Does not have the required features.");
        return -1;
    }

    if (!meetsRequiredLimits(physicalIntrinsic, get_singleton<Instance_vulkan>()->requiredLimits)) {
        LOG_INFO(" - Does not meet the required limits.");
        return -1;
    }

    if (!hasRequiredExtensions(physicalIntrinsic, requiredExtensions)) {
        LOG_INFO(" - Does not have the required extensions.");
        return -1;
    }

    queueFamilyIndicesAndCapabilities = findBestQueueFamilyIndices(window);
    uint8_t deviceCapabilities = 0;
    for (auto queueFamilyIndexAndCapabilities : queueFamilyIndicesAndCapabilities) {
        deviceCapabilities |= queueFamilyIndexAndCapabilities.second;
    }
    LOG_INFO(" - Capabilities=%03b") % deviceCapabilities;

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
    auto formats = physicalIntrinsic.getSurfaceFormatsKHR(window->intrinsic);
    for (auto format : formats) {
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
    auto totalScore = bestSurfaceFormatScore;

    if (bestSurfaceFormatScore == 0) {
        LOG_INFO(" - Does not have a suitable surface format.");
        return 0;
    }

    LOG_INFO(" - Surface present modes:");
    uint32_t bestSurfacePresentModeScore = 0;
    auto presentModes = physicalIntrinsic.getSurfacePresentModesKHR(window->intrinsic);
    for (auto const &presentMode : presentModes) {
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
    totalScore += bestSurfacePresentModeScore;

    if (totalScore < bestSurfacePresentModeScore) {
        LOG_INFO(" - Does not have a suitable surface present mode.");
        return 0;
    }

    // Give score based on the perfomance of the device.
    auto const properties = physicalIntrinsic.getProperties();
    LOG_INFO(" - Type of device: %s") % vk::to_string(properties.deviceType);
    switch (properties.deviceType) {
    case vk::PhysicalDeviceType::eCpu: totalScore += 1; break;
    case vk::PhysicalDeviceType::eOther: totalScore += 1; break;
    case vk::PhysicalDeviceType::eVirtualGpu: totalScore += 2; break;
    case vk::PhysicalDeviceType::eIntegratedGpu: totalScore += 3; break;
    case vk::PhysicalDeviceType::eDiscreteGpu: totalScore += 4; break;
    }

    return totalScore;
}

bool Device_vulkan::memoryTypeNeedsFlushing(uint32_t typeIndex)
{
    auto const propertyFlags = memoryProperties.memoryTypes[typeIndex].propertyFlags;

    if ((propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlags()) {
        return false; // Non host-visible memory.
    }

    if ((propertyFlags & vk::MemoryPropertyFlagBits::eHostCached) != vk::MemoryPropertyFlags()) {
        return true; // Cached memory needs to be flushed.
    }

    if ((propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent) == vk::MemoryPropertyFlags()) {
        return true; // Non host-coherent memory needs to be flushed.
    }
    return false;
}

uint32_t Device_vulkan::findMemoryType(uint32_t validMemoryTypeMask, vk::MemoryPropertyFlags properties)
{
    // Prefer non-eHostCached when requesting eHostVisible.
    if ((properties & vk::MemoryPropertyFlagBits::eHostVisible) != vk::MemoryPropertyFlags()) {
        for (uint32_t typeIndex = 0; typeIndex < memoryProperties.memoryTypeCount; typeIndex++) {
            if (
                (validMemoryTypeMask & (1 << typeIndex)) &&
                ((memoryProperties.memoryTypes[typeIndex].propertyFlags & properties) == properties) &&
                ((memoryProperties.memoryTypes[typeIndex].propertyFlags & vk::MemoryPropertyFlagBits::eHostCached) == vk::MemoryPropertyFlags())) {
                return typeIndex;
            }
        }
    }

    for (uint32_t typeIndex = 0; typeIndex < memoryProperties.memoryTypeCount; typeIndex++) {
        if ((validMemoryTypeMask & (1 << typeIndex)) && ((memoryProperties.memoryTypes[typeIndex].propertyFlags & properties) == properties)) {
            auto const needsFlushing = !(memoryProperties.memoryTypes[typeIndex].propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent);
            return typeIndex;
        }
    }

    BOOST_THROW_EXCEPTION(AllocateMemoryError());
}

pair<vk::DeviceMemory, bool> Device_vulkan::allocateDeviceMemory(size_t size, uint32_t validMemoryTypeMask, vk::MemoryPropertyFlags properties)
{
    auto const memoryTypeIndex = findMemoryType(validMemoryTypeMask, properties);
    auto const needsFlushing = memoryTypeNeedsFlushing(memoryTypeIndex);

    return { intrinsic.allocateMemory({ size, memoryTypeIndex }, nullptr), needsFlushing };
}

std::tuple<vk::DeviceMemory, bool, std::vector<size_t>, std::vector<size_t>> Device_vulkan::allocateDeviceMemory(std::vector<vk::Buffer> buffers, vk::MemoryPropertyFlags properties)
{
    std::vector<size_t> offsets;
    std::vector<size_t> sizes;

    uint32_t memoryTypeBits = 0;
    size_t offset = 0;
    size_t size = 0;
    for (auto buffer : buffers) {
        auto const memoryRequirements = intrinsic.getBufferMemoryRequirements(buffer);

        // Align next buffer on offset.
        offset = TTauri::align(offset, memoryRequirements.alignment);
        offsets.push_back(offset);

        size = offset + memoryRequirements.size;
        sizes.push_back(memoryRequirements.size);

        memoryTypeBits |= memoryRequirements.memoryTypeBits;
    }
    auto memoryAndNeedsFlushing = allocateDeviceMemory(size, memoryTypeBits, properties);

    return { memoryAndNeedsFlushing.first, memoryAndNeedsFlushing.second, offsets, sizes };
}

std::tuple<vk::DeviceMemory, bool, std::vector<size_t>, std::vector<size_t>> Device_vulkan::allocateDeviceMemoryAndBind(std::vector<vk::Buffer> buffers, vk::MemoryPropertyFlags properties)
{
    auto memoryNeedsFlushingOffsetsAndSizes = allocateDeviceMemory(buffers, properties);
    auto const memory = get<0>(memoryNeedsFlushingOffsetsAndSizes);
    auto const offsets = get<2>(memoryNeedsFlushingOffsetsAndSizes);
    for (auto const &bufferAndOffset : boost::combine(buffers, offsets)) {
        auto const buffer = bufferAndOffset.get<0>();
        auto const offset = bufferAndOffset.get<1>();
        intrinsic.bindBufferMemory(buffer, memory, offset);
    }

    return memoryNeedsFlushingOffsetsAndSizes;
}

}}