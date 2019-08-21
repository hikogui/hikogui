// Copyright 2019 Pokitec
// All rights reserved.

#include "PipelineFlat.hpp"
#include "PipelineFlat_DeviceShared.hpp"
#include "Window.hpp"
#include "Device.hpp"

namespace TTauri::GUI::PipelineFlat {

using namespace TTauri;
using namespace std;
using namespace gsl;

PipelineFlat::PipelineFlat(Window const &window) :
    Pipeline_vulkan(window)
{
}

vk::Semaphore PipelineFlat::render(uint32_t frameBufferIndex, vk::Semaphore inputSemaphore)
{
    int tmpNumberOfVertices = 0;
    window.widget->pipelineFlatPlaceVertices(vertexBuffersData.at(frameBufferIndex), tmpNumberOfVertices);

    device().flushAllocation(vertexBuffersAllocation.at(frameBufferIndex), 0, tmpNumberOfVertices * sizeof (Vertex));

    if (tmpNumberOfVertices != numberOfVertices) {
        invalidateCommandBuffers();
        numberOfVertices = tmpNumberOfVertices;
    }

    return Pipeline_vulkan::render(frameBufferIndex, inputSemaphore);
}

void PipelineFlat::drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t frameBufferIndex)
{
    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffers.at(frameBufferIndex) };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    BOOST_ASSERT(tmpVertexBuffers.size() == tmpOffsets.size());

    device().flatPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpVertexBuffers, tmpOffsets);

    pushConstants.windowExtent = { extent.width , extent.height };
    pushConstants.viewportScale = { 2.0 / extent.width, 2.0 / extent.height };
    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, 
        sizeof(PushConstants), 
        &pushConstants
    );

    let numberOfRectangles = numberOfVertices / 4;
    let numberOfTriangles = numberOfRectangles * 2;
    commandBuffer.drawIndexed(
        numeric_cast<uint32_t>(numberOfTriangles * 3),
        1,
        0,
        0,
        0
    );
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineFlat::createShaderStages() const {
    return device().flatPipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> PipelineFlat::createDescriptorSetLayoutBindings() const {
    return { };
}

vector<vk::WriteDescriptorSet> PipelineFlat::createWriteDescriptorSet(uint32_t frameBufferIndex) const
{
    return { };
}

int PipelineFlat::getDescriptorSetVersion() const
{
    return 0;
}

std::vector<vk::PushConstantRange> PipelineFlat::createPushConstantRanges() const
{
    return PushConstants::pushConstantRanges();
}

vk::VertexInputBindingDescription PipelineFlat::createVertexInputBindingDescription() const
{
    return Vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> PipelineFlat::createVertexInputAttributeDescriptions() const {
    return Vertex::inputAttributeDescriptions();
}

void PipelineFlat::buildVertexBuffers(int nrFrameBuffers)
{
    BOOST_ASSERT(vertexBuffers.size() == 0);
    BOOST_ASSERT(vertexBuffersAllocation.size() == 0);
    BOOST_ASSERT(vertexBuffersData.size() == 0);
    for (size_t i = 0; i < nrFrameBuffers; i++) {
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (Vertex) * PipelineFlat::maximumNumberOfVertices,
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        let [vertexBuffer, vertexBufferAllocation] = device().createBuffer(bufferCreateInfo, allocationCreateInfo);
        let vertexBufferData = device().mapMemory<Vertex>(vertexBufferAllocation);

        vertexBuffers.push_back(vertexBuffer);
        vertexBuffersAllocation.push_back(vertexBufferAllocation);
        vertexBuffersData.push_back(vertexBufferData);
    }
}

void PipelineFlat::teardownVertexBuffers()
{
    BOOST_ASSERT(vertexBuffers.size() == vertexBuffersAllocation.size());
    for (size_t i = 0; i < vertexBuffers.size(); i++) {
        auto vertexBuffer = vertexBuffers.at(i);
        auto vertexBufferAllocation = vertexBuffersAllocation.at(i);

        device().unmapMemory(vertexBufferAllocation);
        device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
    }
    vertexBuffers.clear();
    vertexBuffersAllocation.clear();
    vertexBuffersData.clear();
}

}
