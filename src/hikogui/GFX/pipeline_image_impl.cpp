// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "gfx_device_vulkan.hpp"

namespace hi::inline v1::pipeline_image {

pipeline_image::pipeline_image(gfx_surface const &surface) : pipeline_vulkan(surface) {}

void pipeline_image::draw_in_command_buffer(vk::CommandBuffer command_buffer, gfx_draw_context const& context)
{
    pipeline_vulkan::draw_in_command_buffer(command_buffer, context);

    vulkan_device().flushAllocation(vertexBufferAllocation, 0, context.image_vertices.size() * sizeof(vertex));
    vulkan_device().image_pipeline->prepare_atlas_for_rendering();

    std::vector<vk::Buffer> tmp_vertex_buffers = {vertexBuffer};
    std::vector<vk::DeviceSize> tmp_offsets = {0};
    hi_axiom(tmp_vertex_buffers.size() == tmp_offsets.size());

    vulkan_device().image_pipeline->draw_in_command_buffer(command_buffer);

    command_buffer.bindVertexBuffers(0, tmp_vertex_buffers, tmp_offsets);

    pushConstants.windowExtent = extent2{narrow_cast<float>(extent.width), narrow_cast<float>(extent.height)};
    pushConstants.viewportScale = {2.0f / extent.width, 2.0f / extent.height};
    pushConstants.atlasExtent = {device_shared::atlas_image_axis_size, device_shared::atlas_image_axis_size};
    pushConstants.atlasScale = {1.0f / device_shared::atlas_image_axis_size, 1.0f / device_shared::atlas_image_axis_size};
    command_buffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(push_constants),
        &pushConstants);

    hilet number_of_rectangles = context.image_vertices.size() / 4;
    hilet number_of_triangles = number_of_rectangles * 2;

    vulkan_device().cmdBeginDebugUtilsLabelEXT(command_buffer, "draw images");
    command_buffer.drawIndexed(narrow_cast<uint32_t>(number_of_triangles * 3), 1, 0, 0, 0);
    vulkan_device().cmdEndDebugUtilsLabelEXT(command_buffer);
}

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_image::createShaderStages() const
{
    return vulkan_device().image_pipeline->shader_stages;
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_image::createDescriptorSetLayoutBindings() const
{
    return {
        {0, // binding
         vk::DescriptorType::eSampler,
         1, // descriptorCount
         vk::ShaderStageFlagBits::eFragment},
        {1, // binding
         vk::DescriptorType::eSampledImage,
         narrow_cast<uint32_t>(device_shared::atlas_maximum_num_images), // descriptorCount
         vk::ShaderStageFlagBits::eFragment}};
}

std::vector<vk::WriteDescriptorSet> pipeline_image::createWriteDescriptorSet() const
{
    hilet &sharedImagePipeline = vulkan_device().image_pipeline;

    return {
        {
            descriptorSet,
            0, // destBinding
            0, // arrayElement
            1, // descriptorCount
            vk::DescriptorType::eSampler,
            &sharedImagePipeline->atlas_sampler_descriptor_image_info,
            nullptr, // bufferInfo
            nullptr // texelBufferView
        },
        {
            descriptorSet,
            1, // destBinding
            0, // arrayElement
            narrow_cast<uint32_t>(sharedImagePipeline->atlas_descriptor_image_infos.size()), // descriptorCount
            vk::DescriptorType::eSampledImage,
            sharedImagePipeline->atlas_descriptor_image_infos.data(),
            nullptr, // bufferInfo
            nullptr // texelBufferView
        }};
}

ssize_t pipeline_image::getDescriptorSetVersion() const
{
    return ssize(vulkan_device().image_pipeline->atlas_textures);
}

std::vector<vk::PushConstantRange> pipeline_image::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

vk::VertexInputBindingDescription pipeline_image::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> pipeline_image::createVertexInputAttributeDescriptions() const
{
    return vertex::inputAttributeDescriptions();
}

void pipeline_image::build_vertex_buffers()
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
    allocationCreateInfo.pUserData = const_cast<char *>("image-pipeline vertex buffer");
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    std::tie(vertexBuffer, vertexBufferAllocation) = vulkan_device().createBuffer(bufferCreateInfo, allocationCreateInfo);
    vulkan_device().setDebugUtilsObjectNameEXT(vertexBuffer, "image-pipeline vertex buffer");
    vertexBufferData = vulkan_device().mapMemory<vertex>(vertexBufferAllocation);
}

void pipeline_image::teardown_vertex_buffers()
{
    vulkan_device().unmapMemory(vertexBufferAllocation);
    vulkan_device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

} // namespace hi::inline v1::pipeline_image
