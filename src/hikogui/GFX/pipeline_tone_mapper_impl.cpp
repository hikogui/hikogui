// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_tone_mapper.hpp"
#include "pipeline_tone_mapper_device_shared.hpp"
#include "gfx_surface_vulkan.hpp"
#include "gfx_device_vulkan_impl.hpp"
#include "../macros.hpp"

namespace hi::inline v1::pipeline_tone_mapper {

void pipeline_tone_mapper::draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context)
{
    pipeline::draw_in_command_buffer(commandBuffer, context);

    hi_axiom_not_null(device());
    device()->tone_mapper_pipeline->drawInCommandBuffer(commandBuffer);

    _push_constants.saturation = context.saturation;
    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(push_constants), &_push_constants);

    device()->cmdBeginDebugUtilsLabelEXT(commandBuffer, "tone mapping");
    commandBuffer.draw(3, 1, 0, 0);
    device()->cmdEndDebugUtilsLabelEXT(commandBuffer);
}

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_tone_mapper::createShaderStages() const
{
    hi_axiom_not_null(device());
    return device()->tone_mapper_pipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_tone_mapper::createDescriptorSetLayoutBindings() const
{
    return {
        {0, // binding
         vk::DescriptorType::eInputAttachment,
         1, // descriptorCount
         vk::ShaderStageFlagBits::eFragment}};
}

std::vector<vk::WriteDescriptorSet> pipeline_tone_mapper::createWriteDescriptorSet() const
{
    return {{
        descriptorSet,
        0, // destBinding
        0, // arrayElement
        1, // descriptorCount
        vk::DescriptorType::eInputAttachment,
        &surface->colorDescriptorImageInfos[0],
        nullptr, // bufferInfo
        nullptr // texelBufferView
    }};
}

size_t pipeline_tone_mapper::getDescriptorSetVersion() const
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

} // namespace hi::inline v1::pipeline_tone_mapper
