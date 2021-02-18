// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "pipeline_image_push_constants.hpp"
#include "pipeline_image_vertex.hpp"
#include "../vspan.hpp"
#include <vk_mem_alloc.h>

namespace tt::pipeline_image {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlas and sharing for all views.
 */
class pipeline_image : public pipeline_vulkan {
public:
    vspan<vertex> vertexBufferData;

    pipeline_image(gui_window const &window);
    ~pipeline_image() {};

    pipeline_image(const pipeline_image &) = delete;
    pipeline_image &operator=(const pipeline_image &) = delete;
    pipeline_image(pipeline_image &&) = delete;
    pipeline_image &operator=(pipeline_image &&) = delete;

    void drawInCommandBuffer(vk::CommandBuffer commandBuffer) override;

protected:
    push_constants pushConstants;
    int numberOfAtlasImagesInDescriptor = 0;

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
    void buildvertexBuffers() override;
    void teardownvertexBuffers() override;
};

}
