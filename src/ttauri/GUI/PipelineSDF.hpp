// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "PipelineSDF_PushConstants.hpp"
#include "PipelineSDF_Vertex.hpp"
#include "../vspan.hpp"
#include <vma/vk_mem_alloc.h>
#include <span>

namespace tt::PipelineSDF {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlases and sharing for all views.
 */
class PipelineSDF : public pipeline_vulkan {
public:
    vspan<Vertex> vertexBufferData;

    PipelineSDF(gui_window const &window);
    ~PipelineSDF() {};

    PipelineSDF(const PipelineSDF &) = delete;
    PipelineSDF &operator=(const PipelineSDF &) = delete;
    PipelineSDF(PipelineSDF &&) = delete;
    PipelineSDF &operator=(PipelineSDF &&) = delete;

    void drawInCommandBuffer(vk::CommandBuffer commandBuffer) override;

protected:
    PushConstants pushConstants;
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
    void buildVertexBuffers() override;
    void teardownVertexBuffers() override;
};

}
