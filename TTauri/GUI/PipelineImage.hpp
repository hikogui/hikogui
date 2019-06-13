// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Pipeline_vulkan.hpp"
#include "PipelineImage_PushConstants.hpp"
#include "PipelineImage_Vertex.hpp"
#include "globals.hpp"
#include "Device_forward.hpp"
#include <vma/vk_mem_alloc.h>
#include <gsl/gsl>

namespace TTauri::GUI::PipelineImage {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlasses and sharing for all views.
 */
class PipelineImage : public Pipeline_vulkan {
public:
    static constexpr size_t maximumNumberOfVertices = 65536;
    static constexpr size_t maximumNumberOfSquares = maximumNumberOfVertices / 4;
    static constexpr size_t maximumNumberOfTriangles = maximumNumberOfSquares * 2;
    static constexpr size_t maximumNumberOfIndices = maximumNumberOfTriangles * 3;
    
    PipelineImage(const std::shared_ptr<Window> window);
    ~PipelineImage() {};

    PipelineImage(const PipelineImage &) = delete;
    PipelineImage &operator=(const PipelineImage &) = delete;
    PipelineImage(PipelineImage &&) = delete;
    PipelineImage &operator=(PipelineImage &&) = delete;

    vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore) override;

protected:
    PushConstants pushConstants;
    size_t numberOfAtlasImagesInDescriptor = 0;

    size_t numberOfVertices = 0;
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<VmaAllocation> vertexBuffersAllocation;
    std::vector<gsl::span<Vertex>> vertexBuffersData;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t imageIndex) override;

    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet(uint32_t imageIndex) const override;
    virtual uint64_t getDescriptorSetVersion() const override;
    std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;

private:
    void buildVertexBuffers(size_t nrFrameBuffers) override;
    void teardownVertexBuffers() override;
};

}
