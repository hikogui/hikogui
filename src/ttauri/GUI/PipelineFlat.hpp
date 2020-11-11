// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "pipeline_vulkan.hpp"
#include "PipelineFlat_PushConstants.hpp"
#include "PipelineFlat_Vertex.hpp"
#include "../vspan.hpp"
#include <vma/vk_mem_alloc.h>
#include <span>

namespace tt::PipelineFlat {

/*! Pipeline for rendering simple flat shaded quats.
 */
class PipelineFlat : public pipeline_vulkan {
public:
    vspan<Vertex> vertexBufferData;

    PipelineFlat(Window const &window);
    ~PipelineFlat() {};

    PipelineFlat(const PipelineFlat &) = delete;
    PipelineFlat &operator=(const PipelineFlat &) = delete;
    PipelineFlat(PipelineFlat &&) = delete;
    PipelineFlat &operator=(PipelineFlat &&) = delete;

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
