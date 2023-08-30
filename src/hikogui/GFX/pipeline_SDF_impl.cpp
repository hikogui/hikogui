// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_SDF.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "gfx_surface_vulkan.hpp"
#include "gfx_device_vulkan_impl.hpp"
#include "../macros.hpp"

namespace hi::inline v1::pipeline_SDF {

void pipeline_SDF::draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context)
{
    pipeline::draw_in_command_buffer(commandBuffer, context);

    hi_axiom_not_null(device());
    device()->flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof(vertex));

    std::vector<vk::Buffer> tmpvertexBuffers = {vertexBuffer};
    std::vector<vk::DeviceSize> tmpOffsets = {0};
    hi_assert(tmpvertexBuffers.size() == tmpOffsets.size());

    device()->SDF_pipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpvertexBuffers, tmpOffsets);

    pushConstants.window_extent = extent2{narrow_cast<float>(extent.width), narrow_cast<float>(extent.height)};
    pushConstants.viewport_scale = scale2{narrow_cast<float>(2.0f / extent.width), narrow_cast<float>(2.0f / extent.height)};
    pushConstants.has_subpixels = context.subpixel_orientation != subpixel_orientation::unknown;

    constexpr float third = 1.0f / 3.0f;
    switch (context.subpixel_orientation) {
    case subpixel_orientation::unknown:
        pushConstants.red_subpixel_offset = vector2{0.0f, 0.0f};
        pushConstants.blue_subpixel_offset = vector2{0.0f, 0.0f};
        break;
    case subpixel_orientation::horizontal_rgb:
        pushConstants.red_subpixel_offset = vector2{-third, 0.0f};
        pushConstants.blue_subpixel_offset = vector2{third, 0.0f};
        break;
    case subpixel_orientation::horizontal_bgr:
        pushConstants.red_subpixel_offset = vector2{third, 0.0f};
        pushConstants.blue_subpixel_offset = vector2{-third, 0.0f};
        break;
    case subpixel_orientation::vertical_rgb:
        pushConstants.red_subpixel_offset = vector2{0.0f, third};
        pushConstants.blue_subpixel_offset = vector2{0.0f, -third};
        break;
    case subpixel_orientation::vertical_bgr:
        pushConstants.red_subpixel_offset = vector2{0.0f, -third};
        pushConstants.blue_subpixel_offset = vector2{0.0f, third};
        break;
    default:
        hi_no_default();
    }

    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(push_constants),
        &pushConstants);

    hilet numberOfRectangles = vertexBufferData.size() / 4;
    hilet numberOfTriangles = numberOfRectangles * 2;
    device()->cmdBeginDebugUtilsLabelEXT(commandBuffer, "draw glyphs");
    commandBuffer.drawIndexed(narrow_cast<uint32_t>(numberOfTriangles * 3), 1, 0, 0, 0);
    device()->cmdEndDebugUtilsLabelEXT(commandBuffer);
}

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_SDF::createShaderStages() const
{
    hi_axiom_not_null(device());
    return device()->SDF_pipeline->shaderStages;
}

/* Dual-source alpha blending which allows subpixel anti-aliasing.
 */
std::vector<vk::PipelineColorBlendAttachmentState> pipeline_SDF::getPipelineColorBlendAttachmentStates() const
{
    bool has_dual_source_blend = false;
    if (auto device_ = device()) {
        has_dual_source_blend = device_->device_features.dualSrcBlend;
    }

    return {
        {VK_TRUE, // blendEnable
         vk::BlendFactor::eOne, // srcColorBlendFactor
         has_dual_source_blend ? vk::BlendFactor::eOneMinusSrc1Color : vk::BlendFactor::eOneMinusSrcAlpha, // dstColorBlendFactor
         vk::BlendOp::eAdd, // colorBlendOp
         vk::BlendFactor::eOne, // srcAlphaBlendFactor
         has_dual_source_blend ? vk::BlendFactor::eOneMinusSrc1Alpha : vk::BlendFactor::eOneMinusSrcAlpha, // dstAlphaBlendFactor
         vk::BlendOp::eAdd, // aphaBlendOp
         vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
             vk::ColorComponentFlagBits::eA}};
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_SDF::createDescriptorSetLayoutBindings() const
{
    return {
        {0, // binding
         vk::DescriptorType::eSampler,
         1, // descriptorCount
         vk::ShaderStageFlagBits::eFragment},
        {1, // binding
         vk::DescriptorType::eSampledImage,
         narrow_cast<uint32_t>(device_shared::atlasMaximumNrImages), // descriptorCount
         vk::ShaderStageFlagBits::eFragment}};
}

std::vector<vk::WriteDescriptorSet> pipeline_SDF::createWriteDescriptorSet() const
{
    hi_axiom_not_null(device());
    hilet& sharedImagePipeline = device()->SDF_pipeline;

    return {
        {
            descriptorSet,
            0, // destBinding
            0, // arrayElement
            1, // descriptorCount
            vk::DescriptorType::eSampler,
            &sharedImagePipeline->atlasSamplerDescriptorImageInfo,
            nullptr, // bufferInfo
            nullptr // texelBufferView
        },
        {
            descriptorSet,
            1, // destBinding
            0, // arrayElement
            narrow_cast<uint32_t>(device_shared::atlasMaximumNrImages), // descriptorCount
            vk::DescriptorType::eSampledImage,
            sharedImagePipeline->atlasDescriptorImageInfos.data(),
            nullptr, // bufferInfo
            nullptr // texelBufferView
        },
    };
}

size_t pipeline_SDF::getDescriptorSetVersion() const
{
    hi_axiom_not_null(device());
    return device()->SDF_pipeline->atlasTextures.size();
}

std::vector<vk::PushConstantRange> pipeline_SDF::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

vk::VertexInputBindingDescription pipeline_SDF::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> pipeline_SDF::createVertexInputAttributeDescriptions() const
{
    return vertex::inputAttributeDescriptions();
}

void pipeline_SDF::build_vertex_buffers()
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
    allocationCreateInfo.pUserData = const_cast<char *>("sdf-pipeline vertex buffer");
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    hi_axiom_not_null(device());
    std::tie(vertexBuffer, vertexBufferAllocation) = device()->createBuffer(bufferCreateInfo, allocationCreateInfo);
    device()->setDebugUtilsObjectNameEXT(vertexBuffer, "sdf-pipeline vertex buffer");
    vertexBufferData = device()->mapMemory<vertex>(vertexBufferAllocation);
}

void pipeline_SDF::teardown_vertex_buffers()
{
    hi_axiom_not_null(device());
    device()->unmapMemory(vertexBufferAllocation);
    device()->destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

} // namespace hi::inline v1::pipeline_SDF
