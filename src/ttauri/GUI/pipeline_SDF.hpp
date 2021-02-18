// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "pipeline_SDF_push_constants.hpp"
#include "pipeline_SDF_vertex.hpp"
#include "../vspan.hpp"
#include <vk_mem_alloc.h>
#include <span>

namespace tt::pipeline_SDF {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlases and sharing for all views.
 */
class pipeline_SDF : public pipeline_vulkan {
public:
    vspan<vertex> vertexBufferData;

    pipeline_SDF(gui_window const &window);
    ~pipeline_SDF() {};

    pipeline_SDF(const pipeline_SDF &) = delete;
    pipeline_SDF &operator=(const pipeline_SDF &) = delete;
    pipeline_SDF(pipeline_SDF &&) = delete;
    pipeline_SDF &operator=(pipeline_SDF &&) = delete;

    void drawInCommandBuffer(vk::CommandBuffer commandBuffer) override;

protected:
    push_constants pushConstants;
    int numberOfAtlasImagesInDescriptor = 0;

    vk::Buffer vertexBuffer;
    VmaAllocation vertexBufferAllocation;

    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const override;
    ssize_t getDescriptorSetVersion() const override;
    std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;
    std::vector<vk::PipelineColorBlendAttachmentState> getPipelineColorBlendAttachmentStates() const override;

private:
    void buildvertexBuffers() override;
    void teardownvertexBuffers() override;
};

}
