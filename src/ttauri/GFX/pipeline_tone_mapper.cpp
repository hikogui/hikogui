// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_tone_mapper.hpp"
#include "pipeline_tone_mapper_device_shared.hpp"
#include "gfx_surface_vulkan.hpp"
#include "gfx_device_vulkan.hpp"

namespace tt::inline v1::pipeline_tone_mapper {

pipeline_tone_mapper::pipeline_tone_mapper(gfx_surface const &surface) : pipeline_vulkan(surface) {}

void pipeline_tone_mapper::drawInCommandBuffer(vk::CommandBuffer commandBuffer, draw_context const &context)
{
    pipeline_vulkan::drawInCommandBuffer(commandBuffer, context);

    vulkan_device().toneMapperPipeline->drawInCommandBuffer(commandBuffer);

    _push_constants.saturation = context.saturation;
    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(push_constants), &_push_constants);

    vulkan_device().cmdBeginDebugUtilsLabelEXT(commandBuffer, "tone mapping");
    commandBuffer.draw(3, 1, 0, 0);
    vulkan_device().cmdEndDebugUtilsLabelEXT(commandBuffer);
}

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_tone_mapper::createShaderStages() const
{
    return vulkan_device().toneMapperPipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_tone_mapper::createDescriptorSetLayoutBindings() const
{
    // ttlet &color_descriptor_image_infos = narrow_cast<gfx_surface_vulkan const &>(window).colorDescriptorImageInfos;

    return {
        {0, // binding
         vk::DescriptorType::eInputAttachment,
         1, // descriptorCount
         vk::ShaderStageFlagBits::eFragment}};
}

std::vector<vk::WriteDescriptorSet> pipeline_tone_mapper::createWriteDescriptorSet() const
{
    ttlet &color_descriptor_image_infos = down_cast<gfx_surface_vulkan const &>(surface).colorDescriptorImageInfos;

    return {{
        descriptorSet,
        0, // destBinding
        0, // arrayElement
        1, // descriptorCount
        vk::DescriptorType::eInputAttachment,
        &color_descriptor_image_infos[0],
        nullptr, // bufferInfo
        nullptr // texelBufferView
    }};
}

ssize_t pipeline_tone_mapper::getDescriptorSetVersion() const
{
    return 1;
}

std::vector<vk::PushConstantRange> pipeline_tone_mapper::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

vk::PipelineDepthStencilStateCreateInfo pipeline_tone_mapper::getPipelineDepthStencilStateCreateInfo() const
{
    // No depth buffering in the Tone Mapper
    return {
        vk::PipelineDepthStencilStateCreateFlags(),
        VK_FALSE, // depthTestEnable;
        VK_FALSE, // depthWriteEnable;
        vk::CompareOp::eAlways, // depthCompareOp
        VK_FALSE, // depthBoundsTestEnable
        VK_FALSE, // stencilTestEnable,
        vk::StencilOpState(), // front
        vk::StencilOpState(), // back
        0.0f, // minDepthBounds
        1.0f, // maxDepthBounds
    };
}

} // namespace tt::inline v1::pipeline_tone_mapper
