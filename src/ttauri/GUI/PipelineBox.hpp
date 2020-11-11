// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "pipeline_vulkan.hpp"
#include "PipelineBox_PushConstants.hpp"
#include "PipelineBox_Vertex.hpp"
#include "../vspan.hpp"
#include <vma/vk_mem_alloc.h>
#include <span>

namespace tt::PipelineBox {

/*! Pipeline for rendering simple box shaded quats.
 */
class PipelineBox : public pipeline_vulkan {
public:
    vspan<Vertex> vertexBufferData;

    PipelineBox(gui_window const &window);
    ~PipelineBox() {};

    PipelineBox(const PipelineBox &) = delete;
    PipelineBox &operator=(const PipelineBox &) = delete;
    PipelineBox(PipelineBox &&) = delete;
    PipelineBox &operator=(PipelineBox &&) = delete;

    void drawInCommandBuffer(vk::CommandBuffer commandBuffer) override;

protected:
    PushConstants pushConstants;

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
