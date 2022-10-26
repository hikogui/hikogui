// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "pipeline_box_push_constants.hpp"
#include "pipeline_box_vertex.hpp"
#include "../vector_span.hpp"
#include <vma/vk_mem_alloc.h>
#include <span>

namespace hi::inline v1::pipeline_box {

/*! Pipeline for rendering simple box shaded quats.
 */
class pipeline_box : public pipeline_vulkan {
public:
    vector_span<vertex> vertexBufferData;

    pipeline_box(gfx_surface const &surface);
    ~pipeline_box(){};

    pipeline_box(const pipeline_box &) = delete;
    pipeline_box &operator=(const pipeline_box &) = delete;
    pipeline_box(pipeline_box &&) = delete;
    pipeline_box &operator=(pipeline_box &&) = delete;

    void draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context) override;

protected:
    push_constants pushConstants;

    vk::Buffer vertexBuffer;
    VmaAllocation vertexBufferAllocation;

    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const override;
    virtual ssize_t getDescriptorSetVersion() const override;
    std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;

private:
    void build_vertex_buffers() override;
    void teardown_vertex_buffers() override;
};

} // namespace hi::inline v1::pipeline_box
