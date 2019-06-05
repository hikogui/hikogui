// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Pipeline_base.hpp"
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace TTauri::GUI {

class Pipeline_vulkan : public Pipeline_base {
public:
    struct NonVulkanWindowError : virtual Error {};

    vk::Pipeline intrinsic;

    Pipeline_vulkan(const std::shared_ptr<Window> window);
    ~Pipeline_vulkan();

    Pipeline_vulkan(const Pipeline_vulkan &) = delete;
    Pipeline_vulkan &operator=(const Pipeline_vulkan &) = delete;
    Pipeline_vulkan(Pipeline_vulkan &&) = delete;
    Pipeline_vulkan &operator=(Pipeline_vulkan &&) = delete;

    /*! Render
     * This method should be called from sub-classes after completing their own rendering (placing vertices and
     * updating texture maps).
     */
    virtual vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore);

    /*! Invalidate all command buffers.
     * This is used when the command buffer needs to be recreated due to changes in views.
     *
     * \param reset Also reset the command buffer to release resources associated with them.
     */
    void invalidateCommandBuffers();

    /*! Validate/create a command buffer.
     *
     * \param imageIndex The index of the command buffer to validate.
     */
    void validateCommandBuffer(uint32_t imageIndex);

    void buildForNewDevice();
    void teardownForDeviceLost();
    void buildForNewSurface();
    void teardownForSurfaceLost();
    void buildForNewSwapchain(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers);
    void teardownForSwapchainLost();
    void teardownForWindowLost();

protected:
    struct FrameBufferObjects {
        vk::CommandBuffer commandBuffer;
        bool commandBufferValid;
        vk::Semaphore renderFinishedSemaphore;
        vk::DescriptorSet descriptorSet;
        uint64_t descriptorSetVersion = 0;
    };

    std::vector<FrameBufferObjects> frameBufferObjects;

    vk::RenderPass renderPass;
    vk::Extent2D extent;
    vk::Rect2D scissor;
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorPool descriptorPool;

    virtual void drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t imageIndex) = 0;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const = 0;
    virtual std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const = 0;
    virtual std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet(uint32_t imageIndex) const = 0;
    virtual uint64_t getDescriptorSetVersion() const = 0;
    virtual std::vector<vk::PushConstantRange> createPushConstantRanges() const = 0;
    virtual vk::VertexInputBindingDescription createVertexInputBindingDescription() const = 0;
    virtual std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const = 0;

    virtual void buildVertexBuffers(size_t nrFrameBuffers) = 0;
    virtual void teardownVertexBuffers() = 0;
    virtual void buildCommandBuffers();
    virtual void teardownCommandBuffers();
    virtual void buildDescriptorSets();
    virtual void teardownDescriptorSets();
    virtual void buildSemaphores();
    virtual void teardownSemaphores();
    virtual void buildPipeline(vk::RenderPass renderPass, vk::Extent2D extent);
    virtual void teardownPipeline();
};

}
