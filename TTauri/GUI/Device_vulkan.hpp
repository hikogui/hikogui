#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.hpp>

namespace TTauri { namespace GUI {

class Device_vulkan : public Device {
public:
    struct AllocateMemoryError : virtual Error {};
    struct NonVulkanWindowError : virtual Error {};

    vk::PhysicalDevice physicalIntrinsic;
    vk::Device intrinsic;
    vk::PhysicalDeviceType deviceType = vk::PhysicalDeviceType::eOther;
    vk::PhysicalDeviceMemoryProperties memoryProperties;

    /*! List if extension required on this device.
     */
    std::vector<const char *> requiredExtensions;

    /*! Sorted list of queueFamilies and their capabilities.
     * score(window) must be called before initializeDevice(window);
     */
    std::vector<std::pair<uint32_t, QueueCapabilities>> queueFamilyIndicesAndCapabilities;

    /*! Best surfae format.
     * score(window) must be called before initializeDevice(window);
     */
    vk::SurfaceFormatKHR bestSurfaceFormat = {};

    /*! Best surfae format.
     * score(window) must be called before initializeDevice(window);
     */
    vk::PresentModeKHR bestSurfacePresentMode = vk::PresentModeKHR::eFifo;

    Device_vulkan(vk::PhysicalDevice physicalDevice);
    ~Device_vulkan();

    virtual void initializeDevice(std::shared_ptr<Window> window);

    virtual int score(std::shared_ptr<Window> window);

    /*! Find the minimum number of queue families to instantiate for a window.
     * This will give priority for having the Graphics and Present in the same
     * queue family.
     *
     * It is possible this method returns an incomplete queue family set. For
     * example without Present.
     */
    std::vector<std::pair<uint32_t, QueueCapabilities>> findBestQueueFamilyIndices(std::shared_ptr<Window> window);

    uint32_t findMemoryType(uint32_t validMemoryTypeMask, vk::MemoryPropertyFlags properties);

    vk::DeviceMemory allocateDeviceMemory(size_t size, uint32_t validMemoryTypeMask, vk::MemoryPropertyFlags properties);

    std::tuple<vk::DeviceMemory, std::vector<size_t>, std::vector<size_t>> allocateDeviceMemory(std::vector<vk::Buffer> buffers, vk::MemoryPropertyFlags properties);

    std::tuple<vk::DeviceMemory, std::vector<size_t>, std::vector<size_t>> allocateDeviceMemoryAndBind(std::vector<vk::Buffer> buffers, vk::MemoryPropertyFlags properties);
};

}}