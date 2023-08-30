// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_alpha.hpp"
#include "pipeline_alpha_device_shared.hpp"
#include "gfx_device_vulkan_impl.hpp"
#include "../macros.hpp"

namespace hi::inline v1::pipeline_alpha {

/* Do not blend, simply use just the alpha channel and overwrite the pixels in the color attachment directly.
 */
std::vector<vk::PipelineColorBlendAttachmentState> pipeline_alpha::getPipelineColorBlendAttachmentStates() const
{
    return {
        {VK_FALSE, // blendEnable
         vk::BlendFactor::eOne, // srcColorBlendFactor
         vk::BlendFactor::eZero, // dstColorBlendFactor
         vk::BlendOp::eAdd, // colorBlendOp
         vk::BlendFactor::eOne, // srcAlphaBlendFactor
         vk::BlendFactor::eZero, // dstAlphaBlendFactor
         vk::BlendOp::eAdd, // aphaBlendOp
         vk::ColorComponentFlagBits::eA}};
}

void pipeline_alpha::draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context)
{
    pipeline::draw_in_command_buffer(commandBuffer, context);

    hi_axiom_not_null(device());
    device()->flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof(vertex));

    std::vector<vk::Buffer> tmpvertexBuffers = {vertexBuffer};
    std::vector<vk::DeviceSize> tmpOffsets = {0};
    hi_assert(tmpvertexBuffers.size() == tmpOffsets.size());

    device()->alpha_pipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpvertexBuffers, tmpOffsets);

    pushConstants.windowExtent = extent2{narrow_cast<float>(extent.width), narrow_cast<float>(extent.height)};
    pushConstants.viewportScale = scale2{2.0f / extent.width, 2.0f / extent.height};
    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(push_constants),
        &pushConstants);

    hilet numberOfRectangles = vertexBufferData.size() / 4;
    hilet numberOfTriangles = numberOfRectangles * 2;

    device()->cmdBeginDebugUtilsLabelEXT(commandBuffer, "draw alpha overlays");
    commandBuffer.drawIndexed(narrow_cast<uint32_t>(numberOfTriangles * 3), 1, 0, 0, 0);
    device()->cmdEndDebugUtilsLabelEXT(commandBuffer);
}

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_alpha::createShaderStages() const
{
    hi_axiom_not_null(device());
    return device()->alpha_pipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_alpha::createDescriptorSetLayoutBindings() const
{
    return {};
}

std::vector<vk::WriteDescriptorSet> pipeline_alpha::createWriteDescriptorSet() const
{
    return {};
}

size_t pipeline_alpha::getDescriptorSetVersion() const
{
    return 0;
}

std::vector<vk::PushConstantRange> pipeline_alpha::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

vk::VertexInputBindingDescription pipeline_alpha::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> pipeline_alpha::createVertexInputAttributeDescriptions() const
{
    return vertex::inputAttributeDescriptions();
}

void pipeline_alpha::build_vertex_buffers()
{
    using vertexIndexType = uint16_t;
    constexpr ssize_t numberOfVertices = 1 << (sizeof(vertexIndexType) * CHAR_BIT);

    vk::BufferCreateInfo const bufferCreateInfo = {
        vk::BufferCreateFlags(),
        sizeof(vertex) * numberOfVertices,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive};
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    allocationCreateInfo.pUserData = const_cast<char *>("alpha-pipeline vertex buffer");
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    hi_axiom_not_null(device());
    std::tie(vertexBuffer, vertexBufferAllocation) = device()->createBuffer(bufferCreateInfo, allocationCreateInfo);
    device()->setDebugUtilsObjectNameEXT(vertexBuffer, "alpha-pipeline vertex buffer");
    vertexBufferData = device()->mapMemory<vertex>(vertexBufferAllocation);
}

void pipeline_alpha::teardown_vertex_buffers()
{
    hi_axiom_not_null(device());
    device()->unmapMemory(vertexBufferAllocation);
    device()->destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

} // namespace hi::inline v1::pipeline_alpha
