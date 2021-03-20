// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_SDF.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "gui_window_vulkan.hpp"
#include "gui_device_vulkan.hpp"

namespace tt::pipeline_SDF {

using namespace tt;
using namespace std;

pipeline_SDF::pipeline_SDF(gui_window const &window) :
    pipeline_vulkan(window)
{
}

void pipeline_SDF::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    pipeline_vulkan::drawInCommandBuffer(commandBuffer);

    vulkan_device().flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof (vertex));
    vulkan_device().SDFPipeline->prepareAtlasForRendering();

    std::vector<vk::Buffer> tmpvertexBuffers = { vertexBuffer };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    tt_axiom(tmpvertexBuffers.size() == tmpOffsets.size());

    vulkan_device().SDFPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpvertexBuffers, tmpOffsets);

    pushConstants.windowExtent = extent2{ narrow_cast<float>(extent.width) , narrow_cast<float>(extent.height) };
    pushConstants.viewportScale = scale2{ narrow_cast<float>(2.0f / extent.width), narrow_cast<float>(2.0f / extent.height)};
    pushConstants.subpixel_orientation = static_cast<int>(window.subpixel_orientation);

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

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_SDF::createShaderStages() const {
    return vulkan_device().SDFPipeline->shaderStages;
}

/* No alpha blending as SDF fragment shader does this manually.
*/
std::vector<vk::PipelineColorBlendAttachmentState> pipeline_SDF::getPipelineColorBlendAttachmentStates() const
{
    return { {
            VK_FALSE, // blendEnable
            vk::BlendFactor::eOne, // srcColorBlendFactor
            vk::BlendFactor::eOneMinusSrcAlpha, // dstColorBlendFactor
            vk::BlendOp::eAdd, // colorBlendOp
            vk::BlendFactor::eOne, // srcAlphaBlendFactor
            vk::BlendFactor::eZero, // dstAlphaBlendFactor
            vk::BlendOp::eAdd, // aphaBlendOp
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        } };
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_SDF::createDescriptorSetLayoutBindings() const {
    return {
        {
            0, // binding
            vk::DescriptorType::eInputAttachment,
            1, // descriptorCount
            vk::ShaderStageFlagBits::eFragment
        }, {
            1, // binding
            vk::DescriptorType::eSampler,
            1, // descriptorCount
            vk::ShaderStageFlagBits::eFragment
        }, {
            2, // binding
            vk::DescriptorType::eSampledImage,
            narrow_cast<uint32_t>(device_shared::atlasMaximumNrImages), // descriptorCount
            vk::ShaderStageFlagBits::eFragment
        } 
    };
}

vector<vk::WriteDescriptorSet> pipeline_SDF::createWriteDescriptorSet() const
{
    ttlet &sharedImagePipeline = vulkan_device().SDFPipeline;

    return {
        {
            descriptorSet,
            0, // destBinding
            0, // arrayElement
            1, // descriptorCount
            vk::DescriptorType::eInputAttachment,
            &(narrow_cast<gui_window_vulkan const&>(window).colorDescriptorImageInfos[0]),
            nullptr, // bufferInfo
            nullptr // texelBufferView
        }, {
            descriptorSet,
            1, // destBinding
            0, // arrayElement
            1, // descriptorCount
            vk::DescriptorType::eSampler,
            &sharedImagePipeline->atlasSamplerDescriptorImageInfo,
            nullptr,  // bufferInfo
            nullptr // texelBufferView
        }, {
            descriptorSet,
            2, // destBinding
            0, // arrayElement
            narrow_cast<uint32_t>(sharedImagePipeline->atlasDescriptorImageInfos.size()), // descriptorCount
            vk::DescriptorType::eSampledImage,
            sharedImagePipeline->atlasDescriptorImageInfos.data(),
            nullptr, // bufferInfo
            nullptr // texelBufferView
        }, 
    };
}

ssize_t pipeline_SDF::getDescriptorSetVersion() const
{
    return std::ssize(vulkan_device().SDFPipeline->atlasTextures);
}

std::vector<vk::PushConstantRange> pipeline_SDF::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

vk::VertexInputBindingDescription pipeline_SDF::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> pipeline_SDF::createVertexInputAttributeDescriptions() const {
    return vertex::inputAttributeDescriptions();
}

void pipeline_SDF::buildvertexBuffers()
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

void pipeline_SDF::teardownvertexBuffers()
{
    vulkan_device().unmapMemory(vertexBufferAllocation);
    vulkan_device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

}
