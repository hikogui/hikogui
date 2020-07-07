// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/GUI/Pipeline_base.hpp"
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

namespace tt {

class Pipeline_vulkan : public Pipeline_base {
public:
    vk::Pipeline intrinsic;

    Pipeline_vulkan(Window const &window);
    ~Pipeline_vulkan();

    Pipeline_vulkan(const Pipeline_vulkan &) = delete;
    Pipeline_vulkan &operator=(const Pipeline_vulkan &) = delete;
    Pipeline_vulkan(Pipeline_vulkan &&) = delete;
    Pipeline_vulkan &operator=(Pipeline_vulkan &&) = delete;

    virtual void drawInCommandBuffer(vk::CommandBuffer commandBuffer);

    void buildForNewDevice(GUIDevice *device);
    void teardownForDeviceLost();
    void buildForNewSurface();
    void teardownForSurfaceLost();
    void buildForNewSwapchain(vk::RenderPass renderPass, uint32_t renderSubpass, vk::Extent2D extent);
    void teardownForSwapchainLost();
    void teardownForWindowLost();

protected:
    bool buffersInitialized = false;
    vk::DescriptorSet descriptorSet;
    ssize_t descriptorSetVersion = 0;
    vk::Extent2D extent;
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorPool descriptorPool;

    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const = 0;
    virtual std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const = 0;
    virtual std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const = 0;
    virtual ssize_t getDescriptorSetVersion() const = 0;
    virtual std::vector<vk::PushConstantRange> createPushConstantRanges() const { return {}; }
    virtual vk::VertexInputBindingDescription createVertexInputBindingDescription() const { return{}; }
    virtual std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const { return {}; }

    virtual vk::PipelineDepthStencilStateCreateInfo getPipelineDepthStencilStateCreateInfo() const;
    virtual std::vector<vk::PipelineColorBlendAttachmentState> getPipelineColorBlendAttachmentStates() const;

    virtual void buildVertexBuffers() {};
    virtual void teardownVertexBuffers() {};
    virtual void buildDescriptorSets();
    virtual void teardownDescriptorSets();
    virtual void buildPipeline(vk::RenderPass renderPass, uint32_t renderSubpass, vk::Extent2D extent);
    virtual void teardownPipeline();
};

}
