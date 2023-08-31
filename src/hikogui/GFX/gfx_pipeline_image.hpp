// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_pipeline_vulkan.hpp"
#include "gfx_pipeline_image_push_constants.hpp"
#include "gfx_pipeline_image_vertex.hpp"
#include "../container/module.hpp"
#include "../macros.hpp"
#include <vma/vk_mem_alloc.h>

namespace hi::inline v1::gfx_pipeline_image {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlas and sharing for all views.
 */
class gfx_pipeline_image : public gfx_pipeline {
public:
    vector_span<vertex> vertexBufferData;

    ~gfx_pipeline_image() = default;
    gfx_pipeline_image(const gfx_pipeline_image &) = delete;
    gfx_pipeline_image &operator=(const gfx_pipeline_image &) = delete;
    gfx_pipeline_image(gfx_pipeline_image &&) = delete;
    gfx_pipeline_image &operator=(gfx_pipeline_image &&) = delete;

    gfx_pipeline_image(gfx_surface *surface) : gfx_pipeline(surface) {}

    void draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const &context) override;

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

private:
    void build_vertex_buffers() override;
    void teardown_vertex_buffers() override;
};

} // namespace hi::inline v1::gfx_pipeline_image
