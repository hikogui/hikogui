// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline.hpp"
#include "draw_context.hpp"
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

namespace tt::inline v1 {
class gfx_device_vulkan;

class pipeline_vulkan : public pipeline {
public:
    vk::Pipeline intrinsic;

    pipeline_vulkan(gfx_surface const &surface);
    ~pipeline_vulkan();

    pipeline_vulkan(const pipeline_vulkan &) = delete;
    pipeline_vulkan &operator=(const pipeline_vulkan &) = delete;
    pipeline_vulkan(pipeline_vulkan &&) = delete;
    pipeline_vulkan &operator=(pipeline_vulkan &&) = delete;

    gfx_device_vulkan &vulkan_device() const noexcept;

    virtual void drawInCommandBuffer(vk::CommandBuffer commandBuffer, draw_context const &context);

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
    virtual std::vector<vk::PushConstantRange> createPushConstantRanges() const
    {
        return {};
    }
    virtual vk::VertexInputBindingDescription createVertexInputBindingDescription() const
    {
        return {};
    }
    virtual std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const
    {
        return {};
    }

    virtual vk::PipelineDepthStencilStateCreateInfo getPipelineDepthStencilStateCreateInfo() const;
    virtual std::vector<vk::PipelineColorBlendAttachmentState> getPipelineColorBlendAttachmentStates() const;

    virtual void buildvertexBuffers(){};
    virtual void teardownvertexBuffers(){};
    virtual void buildDescriptorSets();
    virtual void teardownDescriptorSets();
    virtual void buildPipeline(vk::RenderPass renderPass, uint32_t renderSubpass, vk::Extent2D extent);
    virtual void teardownPipeline();
};

} // namespace tt::inline v1
