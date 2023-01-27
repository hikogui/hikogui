// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "pipeline_image_push_constants.hpp"
#include "pipeline_image_vertex.hpp"
#include "../vector_span.hpp"
#include <vma/vk_mem_alloc.h>

namespace hi::inline v1::pipeline_image {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlas and sharing for all views.
 */
class pipeline_image : public pipeline_vulkan {
public:
    vector_span<vertex> vertexBufferData;

    pipeline_image(gfx_surface const &surface);
    ~pipeline_image(){};

    pipeline_image(const pipeline_image &) = delete;
    pipeline_image &operator=(const pipeline_image &) = delete;
    pipeline_image(pipeline_image &&) = delete;
    pipeline_image &operator=(pipeline_image &&) = delete;

    void draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const &context) override;

protected:
    push_constants pushConstants;
    int numberOfAtlasImagesInDescriptor = 0;

    vk::Buffer vertexBuffer;
    VmaAllocation vertexBufferAllocation;

    [[nodiscard]] std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    [[nodiscard]] std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    [[nodiscard]] std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const override;
    [[nodiscard]] ssize_t getDescriptorSetVersion() const override;
    [[nodiscard]] std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    [[nodiscard]] vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    [[nodiscard]] std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;

private:
    void build_vertex_buffers() override;
    void teardown_vertex_buffers() override;
};

} // namespace hi::inline v1::pipeline_image
