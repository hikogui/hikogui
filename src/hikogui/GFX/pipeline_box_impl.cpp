// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_box.hpp"
#include "pipeline_box_device_shared.hpp"
#include "gfx_device_vulkan.hpp"

namespace hi::inline v1::pipeline_box {

pipeline_box::pipeline_box(gfx_surface const &surface) : pipeline_vulkan(surface) {}

void pipeline_box::draw_in_command_buffer(vk::CommandBuffer command_buffer, gfx_draw_context const& context)
{
    pipeline_vulkan::draw_in_command_buffer(command_buffer, context);

    vulkan_device().flushAllocation(vertexBufferAllocation, 0, context.box_vertices.size() * sizeof(vertex));

    std::vector<vk::Buffer> tmp_vertex_buffers = {vertexBuffer};
    std::vector<vk::DeviceSize> tmp_offsets = {0};
    hi_axiom(tmp_vertex_buffers.size() == tmp_offsets.size());

    vulkan_device().box_pipeline->drawInCommandBuffer(command_buffer);

    command_buffer.bindVertexBuffers(0, tmp_vertex_buffers, tmp_offsets);

    pushConstants.windowExtent = extent2{narrow_cast<float>(extent.width), narrow_cast<float>(extent.height)};
    pushConstants.viewportScale = scale2{2.0f / extent.width, 2.0f / extent.height};
    command_buffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(push_constants),
        &pushConstants);

    hilet number_of_rectangles = context.box_vertices.size() / 4;
    hilet number_of_triangles = number_of_rectangles * 2;

    vulkan_device().cmdBeginDebugUtilsLabelEXT(command_buffer, "draw boxes");
    command_buffer.drawIndexed(narrow_cast<uint32_t>(number_of_triangles * 3), 1, 0, 0, 0);
    vulkan_device().cmdEndDebugUtilsLabelEXT(command_buffer);
}

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_box::createShaderStages() const
{
    return vulkan_device().box_pipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_box::createDescriptorSetLayoutBindings() const
{
    return {};
}

std::vector<vk::WriteDescriptorSet> pipeline_box::createWriteDescriptorSet() const
{
    return {};
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

std::vector<vk::VertexInputAttributeDescription> pipeline_box::createVertexInputAttributeDescriptions() const
{
    return vertex::inputAttributeDescriptions();
}

void pipeline_box::build_vertex_buffers()
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
    allocationCreateInfo.pUserData = const_cast<char *>("box-pipeline vertex buffer");
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    std::tie(vertexBuffer, vertexBufferAllocation) = vulkan_device().createBuffer(bufferCreateInfo, allocationCreateInfo);
    vulkan_device().setDebugUtilsObjectNameEXT(vertexBuffer, "box-pipeline vertex buffer");
    vertexBufferData = vulkan_device().mapMemory<vertex>(vertexBufferAllocation);
}

void pipeline_box::teardown_vertex_buffers()
{
    vulkan_device().unmapMemory(vertexBufferAllocation);
    vulkan_device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

} // namespace hi::inline v1::pipeline_box
