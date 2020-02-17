// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Pipeline_vulkan.hpp"
#include "TTauri/GUI/PipelineSDF_PushConstants.hpp"
#include "TTauri/GUI/PipelineSDF_Vertex.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include <vma/vk_mem_alloc.h>
#include <gsl/gsl>

namespace TTauri::GUI::PipelineSDF {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlases and sharing for all views.
 */
class PipelineSDF : public Pipeline_vulkan {
public:
    static constexpr int maximumNumberOfVertices = 65536;
    static constexpr int maximumNumberOfSquares = maximumNumberOfVertices / 4;
    static constexpr int maximumNumberOfTriangles = maximumNumberOfSquares * 2;
    static constexpr int maximumNumberOfIndices = maximumNumberOfTriangles * 3;
    
    PipelineSDF(Window const &window);
    ~PipelineSDF() {};

    PipelineSDF(const PipelineSDF &) = delete;
    PipelineSDF &operator=(const PipelineSDF &) = delete;
    PipelineSDF(PipelineSDF &&) = delete;
    PipelineSDF &operator=(PipelineSDF &&) = delete;

    vk::Semaphore render(uint32_t frameBufferIndex, vk::Semaphore inputSemaphore) override;

protected:
    PushConstants pushConstants;
    int numberOfAtlasImagesInDescriptor = 0;

    ssize_t numberOfVertices = 0;
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<VmaAllocation> vertexBuffersAllocation;
    std::vector<gsl::span<Vertex>> vertexBuffersData;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t frameBufferIndex) override;

    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet(uint32_t frameBufferIndex) const override;
    virtual ssize_t getDescriptorSetVersion() const override;
    std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;

private:
    void buildVertexBuffers(int nrFrameBuffers) override;
    void teardownVertexBuffers() override;
};

}
