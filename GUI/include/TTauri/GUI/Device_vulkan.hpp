// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Device_base.hpp"
#include "TTauri/GUI/PipelineImage_DeviceShared.hpp"
#include "TTauri/GUI/PipelineFlat_DeviceShared.hpp"
#include "TTauri/GUI/PipelineBox_DeviceShared.hpp"
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

private:
    void initializeQuadIndexBuffer();
    void destroyQuadIndexBuffer();

public:
    vk::PhysicalDeviceType deviceType = vk::PhysicalDeviceType::eOther;
    vk::PhysicalDeviceProperties physicalProperties;

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

    /** Shared index buffer containing indices for drawing quads.
     * The index buffer uses the following index order: 0, 1, 2, 2, 1, 3
     * ```
     * 2<--3
     * |\  ^
     * | \ |
     * v  \|
     * 0-->1
     * ```
     */
    vk::Buffer quadIndexBuffer;
    VmaAllocation quadIndexBufferAllocation = {};

    std::unique_ptr<PipelineImage::DeviceShared> imagePipeline;
    std::unique_ptr<PipelineFlat::DeviceShared> flatPipeline;
    std::unique_ptr<PipelineBox::DeviceShared> boxPipeline;
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
        auto lock = std::scoped_lock(guiMutex);

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
        auto lock = std::scoped_lock(guiMutex);

        let alignment = physicalProperties.limits.nonCoherentAtomSize;

        let alignedOffset = (offset / alignment) * alignment;
        let adjustedSize = size + (offset - alignedOffset);
        let alignedSize = ((adjustedSize + (alignment - 1)) / alignment) * alignment;

        vmaFlushAllocation(allocator, allocation, alignedOffset, alignedSize);
    }

    vk::ShaderModule loadShader(uint32_t const *data, size_t size) const;

    vk::ShaderModule loadShader(gsl::span<std::byte const> shaderObjectBytes) const;

    vk::ShaderModule loadShader(URL const &shaderObjectLocation) const;


    void waitIdle() const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.waitIdle();
    }

    vk::Result waitForFences(vk::ArrayProxy<const vk::Fence> fences, vk::Bool32 waitAll, uint64_t timeout) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.waitForFences(fences, waitAll, timeout);
    }

    vk::Result acquireNextImageKHR(vk::SwapchainKHR swapchain, uint64_t timeout, vk::Semaphore semaphore, vk::Fence fence, uint32_t* pImageIndex) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.acquireNextImageKHR(swapchain, timeout, semaphore, fence, pImageIndex);
    }

    void resetFences(vk::ArrayProxy<const vk::Fence> fences) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.resetFences(fences);
    }

    vk::Result createSwapchainKHR(const vk::SwapchainCreateInfoKHR* pCreateInfo, const vk::AllocationCallbacks* pAllocator, vk::SwapchainKHR* pSwapchain) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createSwapchainKHR(pCreateInfo, pAllocator, pSwapchain);
    }

    std::vector<vk::Image> getSwapchainImagesKHR(vk::SwapchainKHR swapchain) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.getSwapchainImagesKHR(swapchain);
    }

    vk::ImageView createImageView(const vk::ImageViewCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createImageView(createInfo);
    }

    vk::Framebuffer createFramebuffer(const vk::FramebufferCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createFramebuffer(createInfo);
    }

    vk::RenderPass createRenderPass(const vk::RenderPassCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createRenderPass(createInfo);
    }

    vk::Semaphore createSemaphore(const vk::SemaphoreCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createSemaphore(createInfo);
    }

    vk::Fence createFence(const vk::FenceCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createFence(createInfo);
    }

    vk::DescriptorSetLayout createDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createDescriptorSetLayout(createInfo);
    }

    vk::DescriptorPool createDescriptorPool(const vk::DescriptorPoolCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createDescriptorPool(createInfo);
    }

    vk::PipelineLayout createPipelineLayout(const vk::PipelineLayoutCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createPipelineLayout(createInfo);
    }

    vk::Pipeline createGraphicsPipeline(vk::PipelineCache pipelineCache, const vk::GraphicsPipelineCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createGraphicsPipeline(pipelineCache, createInfo);
    }

    vk::Sampler createSampler(const vk::SamplerCreateInfo& createInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.createSampler(createInfo);
    }

    std::vector<vk::DescriptorSet> allocateDescriptorSets(const vk::DescriptorSetAllocateInfo& allocateInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.allocateDescriptorSets(allocateInfo);
    }

    std::vector<vk::CommandBuffer> allocateCommandBuffers(const vk::CommandBufferAllocateInfo& allocateInfo) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.allocateCommandBuffers(allocateInfo);
    }

    void updateDescriptorSets(vk::ArrayProxy<const vk::WriteDescriptorSet> descriptorWrites, vk::ArrayProxy<const vk::CopyDescriptorSet> descriptorCopies) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.updateDescriptorSets(descriptorWrites, descriptorCopies);
    }

    void freeCommandBuffers(vk::CommandPool commandPool, vk::ArrayProxy<const vk::CommandBuffer> commandBuffers) const {
        auto lock = std::scoped_lock(guiMutex);
        return intrinsic.freeCommandBuffers(commandPool, commandBuffers);
    }

    template<typename T>
    void destroy(T x) const {
        auto lock = std::scoped_lock(guiMutex);
        intrinsic.destroy(x);
    }

    vk::SurfaceCapabilitiesKHR getSurfaceCapabilitiesKHR(vk::SurfaceKHR surface) const {
        auto lock = std::scoped_lock(guiMutex);
        return physicalIntrinsic.getSurfaceCapabilitiesKHR(surface);
    }


};

}
