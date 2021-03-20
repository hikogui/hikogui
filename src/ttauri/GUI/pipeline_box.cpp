// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_box.hpp"
#include "pipeline_box_device_shared.hpp"
#include "gui_device_vulkan.hpp"

namespace tt::pipeline_box {

using namespace tt;
using namespace std;

pipeline_box::pipeline_box(gui_window const &window) :
    pipeline_vulkan(window)
{
}

void pipeline_box::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    pipeline_vulkan::drawInCommandBuffer(commandBuffer);

    vulkan_device().flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof (vertex));

    std::vector<vk::Buffer> tmpvertexBuffers = { vertexBuffer };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    tt_axiom(tmpvertexBuffers.size() == tmpOffsets.size());

    vulkan_device().boxPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpvertexBuffers, tmpOffsets);

    pushConstants.windowExtent = extent2{ narrow_cast<float>(extent.width) , narrow_cast<float>(extent.height) };
    pushConstants.viewportScale = scale2{ 2.0f / extent.width, 2.0f / extent.height };
    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, 
        sizeof(push_constants), 
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

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_box::createShaderStages() const {
    return vulkan_device().boxPipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_box::createDescriptorSetLayoutBindings() const {
    return { };
}

vector<vk::WriteDescriptorSet> pipeline_box::createWriteDescriptorSet() const
{
    return { };
}

ssize_t pipeline_box::getDescriptorSetVersion() const
{
    return 0;
}

std::vector<vk::PushConstantRange> pipeline_box::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

vk::VertexInputBindingDescription pipeline_box::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> pipeline_box::createVertexInputAttributeDescriptions() const {
    return vertex::inputAttributeDescriptions();
}

void pipeline_box::buildvertexBuffers()
{
    using vertexIndexType = uint16_t;
    constexpr ssize_t numberOfVertices = 1 << (sizeof(vertexIndexType) * CHAR_BIT);

    vk::BufferCreateInfo const bufferCreateInfo = {
        vk::BufferCreateFlags(),
        sizeof (vertex) * numberOfVertices,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive
    };
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    std::tie(vertexBuffer, vertexBufferAllocation) = vulkan_device().createBuffer(bufferCreateInfo, allocationCreateInfo);
    vertexBufferData = vulkan_device().mapMemory<vertex>(vertexBufferAllocation);
}

void pipeline_box::teardownvertexBuffers()
{
    vulkan_device().unmapMemory(vertexBufferAllocation);
    vulkan_device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

}
