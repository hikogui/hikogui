// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "pipeline.hpp"
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

namespace tt {
class gui_device_vulkan;

class pipeline_vulkan : public pipeline {
public:
    vk::Pipeline intrinsic;

    pipeline_vulkan(gui_window const &window);
    ~pipeline_vulkan();

    pipeline_vulkan(const pipeline_vulkan &) = delete;
    pipeline_vulkan &operator=(const pipeline_vulkan &) = delete;
    pipeline_vulkan(pipeline_vulkan &&) = delete;
    pipeline_vulkan &operator=(pipeline_vulkan &&) = delete;

    gui_device_vulkan &vulkan_device() const noexcept;

    virtual void drawInCommandBuffer(vk::CommandBuffer commandBuffer);

    void buildForNewDevice();
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
