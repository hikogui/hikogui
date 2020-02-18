// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Pipeline_base.hpp"
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

namespace TTauri::GUI {

class Pipeline_vulkan : public Pipeline_base {
public:
    vk::Pipeline intrinsic;

    Pipeline_vulkan(Window const &window);
    ~Pipeline_vulkan();

    Pipeline_vulkan(const Pipeline_vulkan &) = delete;
    Pipeline_vulkan &operator=(const Pipeline_vulkan &) = delete;
    Pipeline_vulkan(Pipeline_vulkan &&) = delete;
    Pipeline_vulkan &operator=(Pipeline_vulkan &&) = delete;

    /*! Render
     * This method should be called from sub-classes after completing their own rendering (placing vertices and
     * updating texture maps).
     */
    virtual vk::Semaphore render(vk::Framebuffer framebuffer, vk::Semaphore inputSemaphore);

    /*! fill the command buffer.
     */
    void fillCommandBuffer(vk::Framebuffer frameBuffer);

    void buildForNewDevice();
    void teardownForDeviceLost();
    void buildForNewSurface();
    void teardownForSurfaceLost();
    void buildForNewSwapchain(vk::RenderPass renderPass, vk::Extent2D extent);
    void teardownForSwapchainLost();
    void teardownForWindowLost();

protected:
    bool buffersInitialized = false;
    vk::CommandBuffer commandBuffer;
    vk::Semaphore renderFinishedSemaphore;
    vk::DescriptorSet descriptorSet;
    ssize_t descriptorSetVersion = 0;

    vk::RenderPass renderPass;
    vk::Extent2D extent;
    vk::Rect2D scissor;
    bool hasDescriptorSets = false;
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorPool descriptorPool;

    virtual void drawInCommandBuffer() = 0;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const = 0;
    virtual std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const = 0;
    virtual std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const = 0;
    virtual ssize_t getDescriptorSetVersion() const = 0;
    virtual std::vector<vk::PushConstantRange> createPushConstantRanges() const = 0;
    virtual vk::VertexInputBindingDescription createVertexInputBindingDescription() const = 0;
    virtual std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const = 0;

    virtual void buildVertexBuffers() = 0;
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
