// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_tone_mapper.hpp"
#include "pipeline_tone_mapper_device_shared.hpp"
#include "gui_window_vulkan.hpp"
#include "gui_device_vulkan.hpp"

namespace tt::pipeline_tone_mapper {

using namespace tt;
using namespace std;

pipeline_tone_mapper::pipeline_tone_mapper(gui_window const &window) : pipeline_vulkan(window) {}

void pipeline_tone_mapper::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    pipeline_vulkan::drawInCommandBuffer(commandBuffer);

    vulkan_device().toneMapperPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.draw(3, 1, 0, 0);
}

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_tone_mapper::createShaderStages() const
{
    return vulkan_device().toneMapperPipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_tone_mapper::createDescriptorSetLayoutBindings() const
{
    // ttlet &color_descriptor_image_infos = narrow_cast<gui_window_vulkan const &>(window).colorDescriptorImageInfos;

    return {
        {0, // binding
         vk::DescriptorType::eInputAttachment,
         1, // descriptorCount
         vk::ShaderStageFlagBits::eFragment},
        {1, // binding
         vk::DescriptorType::eInputAttachment,
         1, // descriptorCount
         vk::ShaderStageFlagBits::eFragment}};
}

vector<vk::WriteDescriptorSet> pipeline_tone_mapper::createWriteDescriptorSet() const
{
    ttlet &color_descriptor_image_infos = narrow_cast<gui_window_vulkan const &>(window).colorDescriptorImageInfos;

    return {
        {
            descriptorSet,
            0, // destBinding
            0, // arrayElement
            1, // descriptorCount
            vk::DescriptorType::eInputAttachment,
            &color_descriptor_image_infos[0],
            nullptr, // bufferInfo
            nullptr // texelBufferView
        },
        {
            descriptorSet,
            1, // destBinding
            0, // arrayElement
            1, // descriptorCount
            vk::DescriptorType::eInputAttachment,
            &color_descriptor_image_infos[1],
            nullptr, // bufferInfo
            nullptr // texelBufferView
        }};
}

ssize_t pipeline_tone_mapper::getDescriptorSetVersion() const
{
    return 1;
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

} // namespace tt::pipeline_tone_mapper
