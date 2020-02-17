// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Device_base.hpp"
#include "TTauri/GUI/PipelineImage_DeviceShared.hpp"
#include "TTauri/GUI/PipelineFlat_DeviceShared.hpp"
#include "TTauri/GUI/PipelineSDF_DeviceShared.hpp"
#include "TTauri/GUI/globals.hpp"
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

namespace TTauri {
class URL;
}

namespace TTauri::GUI {

class Device_vulkan final : public Device_base {
protected:
    vk::PhysicalDevice physicalIntrinsic;
    vk::Device intrinsic;
    VmaAllocator allocator;

public:
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

    std::unique_ptr<PipelineImage::DeviceShared> imagePipeline;
    std::unique_ptr<PipelineFlat::DeviceShared> flatPipeline;
    std::unique_ptr<PipelineSDF::DeviceShared> SDFPipeline;

    /*! List if extension required on this device.
     */
    std::vector<const char *> requiredExtensions;

    /*! Sorted list of queueFamilies and their capabilities.
     * score(window) must be called before initializeDevice(window);
     */
    mutable std::vector<std::pair<uint32_t, uint8_t>> queueFamilyIndicesAndCapabilities;

    /*! Best surfae format.
     * score(window) must be called before initializeDevice(window);
     */
    mutable vk::SurfaceFormatKHR bestSurfaceFormat = {};

    /*! Best surfae format.
     * score(window) must be called before initializeDevice(window);
     */
    mutable vk::PresentModeKHR bestSurfacePresentMode = vk::PresentModeKHR::eFifo;

    Device_vulkan(vk::PhysicalDevice physicalDevice);
    ~Device_vulkan();

    Device_vulkan(const Device_vulkan &) = delete;
    Device_vulkan &operator=(const Device_vulkan &) = delete;
    Device_vulkan(Device_vulkan &&) = delete;
    Device_vulkan &operator=(Device_vulkan &&) = delete;

    void initializeDevice(Window const &window) override;


    int score(vk::SurfaceKHR surface) const;

    int score(Window const &window) const override;

    /*! Find the minimum number of queue families to instantiate for a window.
     * This will give priority for having the Graphics and Present in the same
     * queue family.
     *
     * It is possible this method returns an incomplete queue family set. For
     * example without Present.
     */
    std::vector<std::pair<uint32_t, uint8_t>> findBestQueueFamilyIndices(vk::SurfaceKHR surface) const;

    std::pair<vk::Buffer, VmaAllocation> createBuffer(const vk::BufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const;

    void destroyBuffer(const vk::Buffer &buffer, const VmaAllocation &allocation) const;

    std::pair<vk::Image, VmaAllocation> createImage(const vk::ImageCreateInfo &imageCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const;
    void destroyImage(const vk::Image &image, const VmaAllocation &allocation) const;

    vk::CommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer) const;

    void transitionLayout(vk::Image image, vk::Format format, vk::ImageLayout srcLayout, vk::ImageLayout dstLayout) const;
    void copyImage(vk::Image srcImage, vk::ImageLayout srcLayout, vk::Image dstImage, vk::ImageLayout dstLayout, vk::ArrayProxy<vk::ImageCopy const> regions) const;

    template <typename T>
    gsl::span<T> mapMemory(const VmaAllocation &allocation) const {
        std::scoped_lock lock(GUI_globals->mutex);

        void *mapping;
        let result = static_cast<vk::Result>(vmaMapMemory(allocator, allocation, &mapping));

        VmaAllocationInfo allocationInfo;
        vmaGetAllocationInfo(allocator, allocation, &allocationInfo);

        // Should we launder the pointer? The GPU has created the objects, not the C++ application.
        T *mappingT = reinterpret_cast<T *>(mapping);
        let mappingSpan = gsl::span<T>(mappingT, allocationInfo.size / sizeof (T));

        return vk::createResultValue(result, mappingSpan, "TTauri::GUI::Device_vulkan::mapMemory");
    }

    void unmapMemory(const VmaAllocation &allocation) const;

    void flushAllocation(const VmaAllocation &allocation, VkDeviceSize offset, VkDeviceSize size) const {
        std::scoped_lock lock(GUI_globals->mutex);
        vmaFlushAllocation(allocator, allocation, offset, size);
    }

    vk::ShaderModule loadShader(uint32_t const *data, size_t size) const;

    vk::ShaderModule loadShader(gsl::span<std::byte const> shaderObjectBytes) const;

    vk::ShaderModule loadShader(URL const &shaderObjectLocation) const;


    void waitIdle() const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.waitIdle();
    }

    vk::Result waitForFences(vk::ArrayProxy<const vk::Fence> fences, vk::Bool32 waitAll, uint64_t timeout) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.waitForFences(fences, waitAll, timeout);
    }

    vk::Result acquireNextImageKHR(vk::SwapchainKHR swapchain, uint64_t timeout, vk::Semaphore semaphore, vk::Fence fence, uint32_t* pImageIndex) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.acquireNextImageKHR(swapchain, timeout, semaphore, fence, pImageIndex);
    }

    void resetFences(vk::ArrayProxy<const vk::Fence> fences) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.resetFences(fences);
    }

    vk::Result createSwapchainKHR(const vk::SwapchainCreateInfoKHR* pCreateInfo, const vk::AllocationCallbacks* pAllocator, vk::SwapchainKHR* pSwapchain) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createSwapchainKHR(pCreateInfo, pAllocator, pSwapchain);
    }

    std::vector<vk::Image> getSwapchainImagesKHR(vk::SwapchainKHR swapchain) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.getSwapchainImagesKHR(swapchain);
    }

    vk::ImageView createImageView(const vk::ImageViewCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createImageView(createInfo);
    }

    vk::Framebuffer createFramebuffer(const vk::FramebufferCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createFramebuffer(createInfo);
    }

    vk::RenderPass createRenderPass(const vk::RenderPassCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createRenderPass(createInfo);
    }

    vk::Semaphore createSemaphore(const vk::SemaphoreCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createSemaphore(createInfo);
    }

    vk::Fence createFence(const vk::FenceCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createFence(createInfo);
    }

    vk::DescriptorSetLayout createDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createDescriptorSetLayout(createInfo);
    }

    vk::DescriptorPool createDescriptorPool(const vk::DescriptorPoolCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createDescriptorPool(createInfo);
    }

    vk::PipelineLayout createPipelineLayout(const vk::PipelineLayoutCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createPipelineLayout(createInfo);
    }

    vk::Pipeline createGraphicsPipeline(vk::PipelineCache pipelineCache, const vk::GraphicsPipelineCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createGraphicsPipeline(pipelineCache, createInfo);
    }

    vk::Sampler createSampler(const vk::SamplerCreateInfo& createInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.createSampler(createInfo);
    }

    std::vector<vk::DescriptorSet> allocateDescriptorSets(const vk::DescriptorSetAllocateInfo& allocateInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.allocateDescriptorSets(allocateInfo);
    }

    std::vector<vk::CommandBuffer> allocateCommandBuffers(const vk::CommandBufferAllocateInfo& allocateInfo) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.allocateCommandBuffers(allocateInfo);
    }

    void updateDescriptorSets(vk::ArrayProxy<const vk::WriteDescriptorSet> descriptorWrites, vk::ArrayProxy<const vk::CopyDescriptorSet> descriptorCopies) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.updateDescriptorSets(descriptorWrites, descriptorCopies);
    }

    void freeCommandBuffers(vk::CommandPool commandPool, vk::ArrayProxy<const vk::CommandBuffer> commandBuffers) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return intrinsic.freeCommandBuffers(commandPool, commandBuffers);
    }

    template<typename T>
    void destroy(T x) const {
        std::scoped_lock lock(GUI_globals->mutex);
        intrinsic.destroy(x);
    }

    vk::SurfaceCapabilitiesKHR getSurfaceCapabilitiesKHR(vk::SurfaceKHR surface) const {
        std::scoped_lock lock(GUI_globals->mutex);
        return physicalIntrinsic.getSurfaceCapabilitiesKHR(surface);
    }


};

}
