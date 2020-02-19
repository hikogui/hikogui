// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineBox.hpp"
#include "TTauri/GUI/PipelineBox_DeviceShared.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/Device.hpp"

namespace TTauri::GUI::PipelineBox {

using namespace TTauri;
using namespace std;
using namespace gsl;

PipelineBox::PipelineBox(Window const &window) :
    Pipeline_vulkan(window)
{
}

vk::Semaphore PipelineBox::render(vk::Framebuffer frameBuffer, vk::Semaphore inputSemaphore)
{
    device().flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof (Vertex));

    return Pipeline_vulkan::render(frameBuffer, inputSemaphore);
}

void PipelineBox::drawInCommandBuffer()
{
    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffer };
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

    let numberOfRectangles = vertexBufferData.size() / 4;
    let numberOfTriangles = numberOfRectangles * 2;
    commandBuffer.drawIndexed(
        numeric_cast<uint32_t>(numberOfTriangles * 3),
        1,
        0,
        0,
        0
    );
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineBox::createShaderStages() const {
    return device().flatPipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> PipelineBox::createDescriptorSetLayoutBindings() const {
    return { };
}

vector<vk::WriteDescriptorSet> PipelineBox::createWriteDescriptorSet() const
{
    return { };
}

ssize_t PipelineBox::getDescriptorSetVersion() const
{
    return 0;
}

std::vector<vk::PushConstantRange> PipelineBox::createPushConstantRanges() const
{
    return PushConstants::pushConstantRanges();
}

vk::VertexInputBindingDescription PipelineBox::createVertexInputBindingDescription() const
{
    return Vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> PipelineBox::createVertexInputAttributeDescriptions() const {
    return Vertex::inputAttributeDescriptions();
}

void PipelineBox::buildVertexBuffers()
{
    vk::BufferCreateInfo const bufferCreateInfo = {
        vk::BufferCreateFlags(),
        sizeof (Vertex) * PipelineBox::maximumNumberOfVertices,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive
    };
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    std::tie(vertexBuffer, vertexBufferAllocation) = device().createBuffer(bufferCreateInfo, allocationCreateInfo);
    vertexBufferData = device().mapMemory<Vertex>(vertexBufferAllocation);
}

void PipelineBox::teardownVertexBuffers()
{
    device().unmapMemory(vertexBufferAllocation);
    device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

}
