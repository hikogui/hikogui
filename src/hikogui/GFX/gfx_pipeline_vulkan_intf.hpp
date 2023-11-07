// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

hi_export_module(hikogui.GFX : gfx_pipeline_intf);

hi_export namespace hi::inline v1 {
class gfx_device;
class gfx_surface;
class draw_context;

class gfx_pipeline {
public:
    vk::Pipeline intrinsic;
    gfx_surface *surface = nullptr;

    gfx_pipeline(gfx_surface *surface) : surface(surface) {}

    virtual ~gfx_pipeline() = default;
    gfx_pipeline(const gfx_pipeline &) = delete;
    gfx_pipeline &operator=(const gfx_pipeline &) = delete;
    gfx_pipeline(gfx_pipeline &&) = delete;
    gfx_pipeline &operator=(gfx_pipeline &&) = delete;

    virtual void draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const &context);

    void build_for_new_device();
    void teardown_for_device_lost();
    void build_for_new_swapchain(vk::RenderPass renderPass, uint32_t renderSubpass, vk::Extent2D extent);
    void teardown_for_swapchain_lost();

protected:
    vk::DescriptorSet descriptorSet;
    size_t descriptorSetVersion = 0;
    vk::Extent2D extent;
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorPool descriptorPool;

    [[nodiscard]] gfx_device *device() const noexcept;

    [[nodiscard]] virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const = 0;
    [[nodiscard]] virtual std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const = 0;
    [[nodiscard]] virtual std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const = 0;
    [[nodiscard]] virtual size_t getDescriptorSetVersion() const = 0;
    [[nodiscard]] virtual std::vector<vk::PushConstantRange> createPushConstantRanges() const
    {
        return {};
    }
    [[nodiscard]] virtual vk::VertexInputBindingDescription createVertexInputBindingDescription() const
    {
        return {};
    }
    [[nodiscard]] virtual std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const
    {
        return {};
    }

    [[nodiscard]] virtual vk::PipelineDepthStencilStateCreateInfo getPipelineDepthStencilStateCreateInfo() const;
    [[nodiscard]] virtual std::vector<vk::PipelineColorBlendAttachmentState> getPipelineColorBlendAttachmentStates() const;

    virtual void build_vertex_buffers(){};
    virtual void teardown_vertex_buffers(){};
    virtual void build_descriptor_sets();
    virtual void teardown_descriptor_sets();
    virtual void build_pipeline(vk::RenderPass renderPass, uint32_t renderSubpass, vk::Extent2D extent);
    virtual void teardown_pipeline();
};

} // namespace hi::inline v1
