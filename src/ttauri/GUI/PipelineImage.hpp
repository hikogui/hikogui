// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "PipelineImage_PushConstants.hpp"
#include "PipelineImage_Vertex.hpp"
#include "../vspan.hpp"
#include <vma/vk_mem_alloc.h>

namespace tt::PipelineImage {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlas and sharing for all views.
 */
class PipelineImage : public pipeline_vulkan {
public:
    vspan<Vertex> vertexBufferData;

    PipelineImage(gui_window const &window);
    ~PipelineImage() {};

    PipelineImage(const PipelineImage &) = delete;
    PipelineImage &operator=(const PipelineImage &) = delete;
    PipelineImage(PipelineImage &&) = delete;
    PipelineImage &operator=(PipelineImage &&) = delete;

    void drawInCommandBuffer(vk::CommandBuffer commandBuffer) override;

protected:
    PushConstants pushConstants;
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
    void buildVertexBuffers() override;
    void teardownVertexBuffers() override;
};

}
