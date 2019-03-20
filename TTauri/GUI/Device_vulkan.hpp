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

    uint32_t graphicsQueueFamilyIndex = 0;
    uint32_t presentQueueFamilyIndex = 0;
    uint32_t computeQueueFamilyIndex = 0;
    uint32_t graphicsQueueIndex = 0;
    uint32_t presentQueueIndex = 0;
    uint32_t computeQueueIndex = 0;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::Queue computeQueue;
    vk::CommandPool graphicsCommandPool;
    vk::CommandPool presentCommandPool;
    vk::CommandPool computeCommandPool;

    /*! List if extension required on this device.
     */
    std::vector<const char *> requiredExtensions;

    /*! Sorted list of queueFamilies and their capabilities.
     * score(window) must be called before initializeDevice(window);
     */
    std::vector<std::pair<uint32_t, uint8_t>> queueFamilyIndicesAndCapabilities;

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

    Device_vulkan(const Device_vulkan &) = delete;
    Device_vulkan &operator=(const Device_vulkan &) = delete;
    Device_vulkan(Device_vulkan &&) = delete;
    Device_vulkan &operator=(Device_vulkan &&) = delete;

    void initializeDevice(std::shared_ptr<Window> window) override;

    int score(std::shared_ptr<Window> window) override;

    /*! Find the minimum number of queue families to instantiate for a window.
     * This will give priority for having the Graphics and Present in the same
     * queue family.
     *
     * It is possible this method returns an incomplete queue family set. For
     * example without Present.
     */
    std::vector<std::pair<uint32_t, uint8_t>> findBestQueueFamilyIndices(std::shared_ptr<Window> window);

    bool memoryTypeNeedsFlushing(uint32_t typeIndex);

    uint32_t findMemoryType(uint32_t validMemoryTypeMask, vk::MemoryPropertyFlags properties);

    /*! Allocate memory on the GPU.
     * \param size number of bytes to allocate.
     * \param validMemoryTypeMask from which memory types to allocate memory from.
     * \param properties what kind of memory properties are needed.
     * \return A block of device memory, and true if this memory requires flushing.
     */
    std::pair<vk::DeviceMemory, bool> allocateDeviceMemory(size_t size, uint32_t validMemoryTypeMask, vk::MemoryPropertyFlags properties);

    /*! Allocate memory on the GPU.
     * \param buffers Buffers to allocate memory for.
     * \param properties what kind of memory properties are needed.
     * \return A block of device memory, and true if this memory requires flushing, offsets and size of each buffer.
     */
    std::tuple<vk::DeviceMemory, bool, std::vector<std::pair<size_t, size_t>>> allocateDeviceMemory(std::vector<vk::Buffer> buffers, vk::MemoryPropertyFlags properties);

    /*! Allocate memory on the GPU and bind to buffers.
     * \param buffers Buffers to allocate and bind memory for.
     * \param properties what kind of memory properties are needed.
     * \return A block of device memory, and true if this memory requires flushing, offsets and size of each buffer.
     */
    std::tuple<vk::DeviceMemory, bool, std::vector<std::pair<size_t, size_t>>> allocateDeviceMemoryAndBind(std::vector<vk::Buffer> buffers, vk::MemoryPropertyFlags properties);
};

}}