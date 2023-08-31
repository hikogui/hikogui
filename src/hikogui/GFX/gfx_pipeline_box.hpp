// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_pipeline_vulkan.hpp"
#include "gfx_pipeline_box_push_constants.hpp"
#include "gfx_pipeline_box_vertex.hpp"
#include "../container/module.hpp"
#include "../macros.hpp"
#include <vma/vk_mem_alloc.h>
#include <span>

namespace hi::inline v1::gfx_pipeline_box {

/*! Pipeline for rendering simple box shaded quats.
 */
class gfx_pipeline_box : public gfx_pipeline {
public:
    vector_span<vertex> vertexBufferData;

    ~gfx_pipeline_box() = default;
    gfx_pipeline_box(const gfx_pipeline_box &) = delete;
    gfx_pipeline_box &operator=(const gfx_pipeline_box &) = delete;
    gfx_pipeline_box(gfx_pipeline_box &&) = delete;
    gfx_pipeline_box &operator=(gfx_pipeline_box &&) = delete;

    gfx_pipeline_box(gfx_surface *surface) : gfx_pipeline(surface) {}

    void draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context) override;

protected:
    push_constants pushConstants;

    vk::Buffer vertexBuffer;
    VmaAllocation vertexBufferAllocation;

    [[nodiscard]] std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    [[nodiscard]] std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    [[nodiscard]] std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const override;
    [[nodiscard]] size_t getDescriptorSetVersion() const override;
    [[nodiscard]] std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    [[nodiscard]] vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    [[nodiscard]] std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;

private:
    void build_vertex_buffers() override;
    void teardown_vertex_buffers() override;
};

} // namespace hi::inline v1::gfx_pipeline_box
