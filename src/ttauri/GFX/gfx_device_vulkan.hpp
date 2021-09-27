// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_device.hpp"
#include "gfx_system_globals.hpp"
#include "gfx_queue_vulkan.hpp"
#include "pipeline_image_device_shared.hpp"
#include "pipeline_box_device_shared.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "pipeline_tone_mapper_device_shared.hpp"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace tt {
class URL;
}

namespace tt {

class gfx_device_vulkan final : public gfx_device {
public:
    
    vk::PhysicalDeviceType deviceType = vk::PhysicalDeviceType::eOther;
    vk::PhysicalDeviceProperties physicalProperties;

    std::vector<gfx_queue_vulkan> _queues;
    
    /** Get a graphics queue.
     * Always returns the first queue that can handle graphics.
     */
    [[nodiscard]] gfx_queue_vulkan const &get_graphics_queue() const noexcept;

    /** Get a graphics queue.
     * Always returns the first queue that can handle both graphics and presenting;
     * or as fallback the first graphics queue.
     */
    [[nodiscard]] gfx_queue_vulkan const &get_graphics_queue(gfx_surface const &surface) const noexcept;

    /** Get a present queue.
     * Always returns the first queue that can handle both graphics and presenting;
     * or as fallback the first present queue.
     */
    [[nodiscard]] gfx_queue_vulkan const &get_present_queue(gfx_surface const &surface) const noexcept;

    /** Get the surface format.
     * Always returns the best suitable surface format.
     * 
     * Prioritizes HDR, followed by sRGB.
     * 
     * @param surface The surface to determine the surface format for.
     * @param [out]score Optional return parameter for the quality of the surface format.
     */
    [[nodiscard]] vk::SurfaceFormatKHR get_surface_format(gfx_surface const &surface, int *score = nullptr) const noexcept;

    /** Get the present mode.
     * Always returns the best suitable present mode.
     * 
     * Prioritized a double buffering mode.
     *
     * @param surface The surface to determine the present mode for.
     * @param [out]score Optional return parameter for the quality of the present mode.
     */
    [[nodiscard]] vk::PresentModeKHR get_present_mode(gfx_surface const &surface, int *score = nullptr) const noexcept;

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

    std::unique_ptr<pipeline_box::device_shared> boxPipeline;
    std::unique_ptr<pipeline_image::device_shared> imagePipeline;
    std::unique_ptr<pipeline_SDF::device_shared> SDFPipeline;
    std::unique_ptr<pipeline_tone_mapper::device_shared> toneMapperPipeline;

    /*! List if extension required on this device.
     */
    std::vector<const char *> requiredExtensions;

    bool supportsLazyTransientImages = false;
    vk::ImageUsageFlags transientImageUsageFlags = vk::ImageUsageFlags{};
    VmaMemoryUsage lazyMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;


    gfx_device_vulkan(gfx_system &system, vk::PhysicalDevice physicalDevice);
    ~gfx_device_vulkan();

    gfx_device_vulkan(const gfx_device_vulkan &) = delete;
    gfx_device_vulkan &operator=(const gfx_device_vulkan &) = delete;
    gfx_device_vulkan(gfx_device_vulkan &&) = delete;
    gfx_device_vulkan &operator=(gfx_device_vulkan &&) = delete;

    int score(vk::SurfaceKHR surface) const;

    int score(gfx_surface const &surface) const override;

    /*! Find the minimum number of queue families to instantiate for a window.
     * This will give priority for having the Graphics and Present in the same
     * queue family.
     *
     * It is possible this method returns an incomplete queue family set. For
     * example without Present.
     */
    std::vector<std::pair<uint32_t, uint8_t>> find_best_queue_family_indices(vk::SurfaceKHR surface) const;

    std::pair<vk::Buffer, VmaAllocation>
    createBuffer(const vk::BufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const;

    void destroyBuffer(const vk::Buffer &buffer, const VmaAllocation &allocation) const;

    std::pair<vk::Image, VmaAllocation>
    createImage(const vk::ImageCreateInfo &imageCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) const;
    void destroyImage(const vk::Image &image, const VmaAllocation &allocation) const;

    vk::CommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer) const;

    static void transition_layout(
        vk::CommandBuffer command_buffer,
        vk::Image image,
        vk::Format format,
        vk::ImageLayout src_layout,
        vk::ImageLayout dst_layout);
    void transition_layout(vk::Image image, vk::Format format, vk::ImageLayout src_layout, vk::ImageLayout dst_layout) const;

    void copyImage(
        vk::Image srcImage,
        vk::ImageLayout srcLayout,
        vk::Image dstImage,
        vk::ImageLayout dstLayout,
        vk::ArrayProxy<vk::ImageCopy const> regions) const;
    void clearColorImage(
        vk::Image image,
        vk::ImageLayout layout,
        vk::ClearColorValue const &color,
        vk::ArrayProxy<const vk::ImageSubresourceRange> ranges) const;

    template<typename T>
    std::span<T> mapMemory(const VmaAllocation &allocation) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());

        void *mapping;
        ttlet result = static_cast<vk::Result>(vmaMapMemory(allocator, allocation, &mapping));

        VmaAllocationInfo allocationInfo;
        vmaGetAllocationInfo(allocator, allocation, &allocationInfo);

        // Should we launder the pointer? The GPU has created the objects, not the C++ application.
        T *mappingT = reinterpret_cast<T *>(mapping);
        ttlet mappingSpan = std::span<T>(mappingT, allocationInfo.size / sizeof(T));

        return vk::createResultValue(result, mappingSpan, "tt::gfx_device_vulkan::mapMemory");
    }

    void unmapMemory(const VmaAllocation &allocation) const;

    void flushAllocation(const VmaAllocation &allocation, VkDeviceSize offset, VkDeviceSize size) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());

        ttlet alignment = physicalProperties.limits.nonCoherentAtomSize;

        ttlet alignedOffset = (offset / alignment) * alignment;
        ttlet adjustedSize = size + (offset - alignedOffset);
        ttlet alignedSize = ((adjustedSize + (alignment - 1)) / alignment) * alignment;

        vmaFlushAllocation(allocator, allocation, alignedOffset, alignedSize);
    }

    vk::ShaderModule loadShader(uint32_t const *data, size_t size) const;

    vk::ShaderModule loadShader(std::span<std::byte const> shaderObjectBytes) const;

    vk::ShaderModule loadShader(URL const &shaderObjectLocation) const;

    void waitIdle() const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.waitIdle();
    }

    vk::Result waitForFences(vk::ArrayProxy<const vk::Fence> fences, vk::Bool32 waitAll, uint64_t timeout) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.waitForFences(fences, waitAll, timeout);
    }

    vk::Result acquireNextImageKHR(
        vk::SwapchainKHR swapchain,
        uint64_t timeout,
        vk::Semaphore semaphore,
        vk::Fence fence,
        uint32_t *pImageIndex) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.acquireNextImageKHR(swapchain, timeout, semaphore, fence, pImageIndex);
    }

    void resetFences(vk::ArrayProxy<const vk::Fence> fences) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.resetFences(fences);
    }

    vk::Result createSwapchainKHR(
        const vk::SwapchainCreateInfoKHR *pCreateInfo,
        const vk::AllocationCallbacks *pAllocator,
        vk::SwapchainKHR *pSwapchain) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createSwapchainKHR(pCreateInfo, pAllocator, pSwapchain);
    }

    std::vector<vk::Image> getSwapchainImagesKHR(vk::SwapchainKHR swapchain) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.getSwapchainImagesKHR(swapchain);
    }

    vk::ImageView createImageView(const vk::ImageViewCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createImageView(createInfo);
    }

    vk::Framebuffer createFramebuffer(const vk::FramebufferCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createFramebuffer(createInfo);
    }

    vk::RenderPass createRenderPass(const vk::RenderPassCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createRenderPass(createInfo);
    }

    vk::Semaphore createSemaphore(const vk::SemaphoreCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createSemaphore(createInfo);
    }

    vk::Fence createFence(const vk::FenceCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createFence(createInfo);
    }

    vk::DescriptorSetLayout createDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createDescriptorSetLayout(createInfo);
    }

    vk::DescriptorPool createDescriptorPool(const vk::DescriptorPoolCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createDescriptorPool(createInfo);
    }

    vk::PipelineLayout createPipelineLayout(const vk::PipelineLayoutCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createPipelineLayout(createInfo);
    }

    vk::Pipeline createGraphicsPipeline(vk::PipelineCache pipelineCache, const vk::GraphicsPipelineCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createGraphicsPipeline(pipelineCache, createInfo).value;
    }

    vk::Sampler createSampler(const vk::SamplerCreateInfo &createInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.createSampler(createInfo);
    }

    std::vector<vk::DescriptorSet> allocateDescriptorSets(const vk::DescriptorSetAllocateInfo &allocateInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.allocateDescriptorSets(allocateInfo);
    }

    std::vector<vk::CommandBuffer> allocateCommandBuffers(const vk::CommandBufferAllocateInfo &allocateInfo) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.allocateCommandBuffers(allocateInfo);
    }

    void updateDescriptorSets(
        vk::ArrayProxy<const vk::WriteDescriptorSet> descriptorWrites,
        vk::ArrayProxy<const vk::CopyDescriptorSet> descriptorCopies) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.updateDescriptorSets(descriptorWrites, descriptorCopies);
    }

    void freeCommandBuffers(vk::CommandPool commandPool, vk::ArrayProxy<const vk::CommandBuffer> commandBuffers) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return intrinsic.freeCommandBuffers(commandPool, commandBuffers);
    }

    template<typename T>
    void destroy(T x) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        intrinsic.destroy(x);
    }

    vk::SurfaceCapabilitiesKHR getSurfaceCapabilitiesKHR(vk::SurfaceKHR surface) const
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return physicalIntrinsic.getSurfaceCapabilitiesKHR(surface);
    }

protected:
    vk::PhysicalDevice physicalIntrinsic;
    vk::Device intrinsic;
    VmaAllocator allocator;

private:
    [[nodiscard]] std::vector<vk::DeviceQueueCreateInfo> make_device_queue_create_infos() const noexcept;
    void initialize_queues(std::vector<vk::DeviceQueueCreateInfo> const &device_queue_create_infos) noexcept;
    void initialize_device();
    void initialize_quad_index_buffer();
    void destroy_quad_index_buffer();
};

} // namespace tt
