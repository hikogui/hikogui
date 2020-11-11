// Copyright 2019 Pokitec
// All rights reserved.

#include "PipelineToneMapper.hpp"
#include "PipelineToneMapper_DeviceShared.hpp"
#include "Window.hpp"

namespace tt::PipelineToneMapper {

using namespace tt;
using namespace std;

PipelineToneMapper::PipelineToneMapper(Window const &window) :
    pipeline_vulkan(window)
{
}

void PipelineToneMapper::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    pipeline_vulkan::drawInCommandBuffer(commandBuffer);


    vulkan_device().toneMapperPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.draw(
        3,
        1,
        0,
        0
    );
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineToneMapper::createShaderStages() const {
    return vulkan_device().toneMapperPipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> PipelineToneMapper::createDescriptorSetLayoutBindings() const {
    return {
        {
            0, // binding
            vk::DescriptorType::eInputAttachment,
            1, // descriptorCount
            vk::ShaderStageFlagBits::eFragment
        }
    };
}

vector<vk::WriteDescriptorSet> PipelineToneMapper::createWriteDescriptorSet() const
{
    return {
        {
            descriptorSet,
            0, // destBinding
            0, // arrayElement
            1, // descriptorCount
            vk::DescriptorType::eInputAttachment,
            &(window.colorDescriptorImageInfo),
            nullptr, // bufferInfo
            nullptr // texelBufferView
        } 
    };
}

ssize_t PipelineToneMapper::getDescriptorSetVersion() const
{
    return 1;
}

vk::PipelineDepthStencilStateCreateInfo PipelineToneMapper::getPipelineDepthStencilStateCreateInfo() const
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

}
