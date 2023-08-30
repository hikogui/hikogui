// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "pipeline_SDF_push_constants.hpp"
#include "pipeline_SDF_vertex.hpp"
#include "../container/module.hpp"
#include "../macros.hpp"
#include <vma/vk_mem_alloc.h>
#include <span>

namespace hi::inline v1::pipeline_SDF {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlases and sharing for all views.
 */
class pipeline_SDF : public pipeline {
public:
    vector_span<vertex> vertexBufferData;

    ~pipeline_SDF() = default;
    pipeline_SDF(const pipeline_SDF &) = delete;
    pipeline_SDF &operator=(const pipeline_SDF &) = delete;
    pipeline_SDF(pipeline_SDF &&) = delete;
    pipeline_SDF &operator=(pipeline_SDF &&) = delete;

    pipeline_SDF(gfx_surface *surface) : pipeline(surface) {}

    void draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context) override;

protected:
    push_constants pushConstants;
    int numberOfAtlasImagesInDescriptor = 0;

    vk::Buffer vertexBuffer;
    VmaAllocation vertexBufferAllocation;

    [[nodiscard]] std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    [[nodiscard]] std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    [[nodiscard]] std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const override;
    [[nodiscard]] size_t getDescriptorSetVersion() const override;
    [[nodiscard]] std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    [[nodiscard]] vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    [[nodiscard]] std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;
    [[nodiscard]] std::vector<vk::PipelineColorBlendAttachmentState> getPipelineColorBlendAttachmentStates() const override;

private:
    void build_vertex_buffers() override;
    void teardown_vertex_buffers() override;
};

} // namespace hi::inline v1::pipeline_SDF
