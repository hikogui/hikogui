// Copyright 2019 Pokitec
// All rights reserved.

#include "PipelineFlat.hpp"
#include "PipelineFlat_DeviceShared.hpp"
#include "gui_device.hpp"

namespace tt::PipelineFlat {

using namespace tt;
using namespace std;

PipelineFlat::PipelineFlat(gui_window const &window) :
    pipeline_vulkan(window)
{
}

void PipelineFlat::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    pipeline_vulkan::drawInCommandBuffer(commandBuffer);

    vulkan_device().flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof (Vertex));

    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffer };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    tt_assume(tmpVertexBuffers.size() == tmpOffsets.size());

    vulkan_device().flatPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpVertexBuffers, tmpOffsets);

    pushConstants.windowExtent = { narrow_cast<int>(extent.width) , narrow_cast<int>(extent.height) };
    pushConstants.viewportScale = { 2.0 / extent.width, 2.0 / extent.height };
    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, 
        sizeof(PushConstants), 
        &pushConstants
    );

    ttlet numberOfRectangles = vertexBufferData.size() / 4;
    ttlet numberOfTriangles = numberOfRectangles * 2;
    commandBuffer.drawIndexed(
        narrow_cast<uint32_t>(numberOfTriangles * 3),
        1,
        0,
        0,
        0
    );
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineFlat::createShaderStages() const {
    return vulkan_device().flatPipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> PipelineFlat::createDescriptorSetLayoutBindings() const {
    return { };
}

vector<vk::WriteDescriptorSet> PipelineFlat::createWriteDescriptorSet() const
{
    return { };
}

ssize_t PipelineFlat::getDescriptorSetVersion() const
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

void PipelineFlat::buildVertexBuffers()
{
    using vertexIndexType = uint16_t;
    constexpr ssize_t numberOfVertices = 1 << (sizeof(vertexIndexType) * CHAR_BIT);

    vk::BufferCreateInfo const bufferCreateInfo = {
        vk::BufferCreateFlags(),
        sizeof (Vertex) * numberOfVertices,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive
    };
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    std::tie(vertexBuffer, vertexBufferAllocation) = vulkan_device().createBuffer(bufferCreateInfo, allocationCreateInfo);
    vertexBufferData = vulkan_device().mapMemory<Vertex>(vertexBufferAllocation);
}

void PipelineFlat::teardownVertexBuffers()
{
    vulkan_device().unmapMemory(vertexBufferAllocation);
    vulkan_device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

}
