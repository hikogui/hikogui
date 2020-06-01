// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Pipeline_vulkan.hpp"
#include "TTauri/GUI/PipelineSDF_PushConstants.hpp"
#include "TTauri/GUI/PipelineSDF_Vertex.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include <vma/vk_mem_alloc.h>
#include <nonstd/span>

namespace TTauri::GUI::PipelineSDF {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlases and sharing for all views.
 */
class PipelineSDF : public Pipeline_vulkan {
public:
    vspan<Vertex> vertexBufferData;

    PipelineSDF(Window const &window);
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
