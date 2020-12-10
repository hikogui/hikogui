// Copyright 2019 Pokitec
// All rights reserved.

#include "PipelineSDF.hpp"
#include "PipelineSDF_DeviceShared.hpp"
#include "gui_window_vulkan.hpp"

namespace tt::PipelineSDF {

using namespace tt;
using namespace std;

PipelineSDF::PipelineSDF(gui_window const &window) :
    pipeline_vulkan(window)
{
}

void PipelineSDF::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    pipeline_vulkan::drawInCommandBuffer(commandBuffer);

    vulkan_device().flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof (Vertex));
    vulkan_device().SDFPipeline->prepareAtlasForRendering();

    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffer };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    tt_axiom(tmpVertexBuffers.size() == tmpOffsets.size());

    vulkan_device().SDFPipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpVertexBuffers, tmpOffsets);

    pushConstants.windowExtent = f32x4{ narrow_cast<float>(extent.width) , narrow_cast<float>(extent.height) };
    pushConstants.viewportScale = f32x4{ narrow_cast<float>(2.0f / extent.width), narrow_cast<float>(2.0f / extent.height)};
    pushConstants.subpixelOrientation = static_cast<int>(window.subpixelOrientation);

    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, 
        sizeof(PushConstants), 
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

std::vector<vk::PipelineShaderStageCreateInfo> PipelineSDF::createShaderStages() const {
    return vulkan_device().SDFPipeline->shaderStages;
}

/* No alpha blending as SDF fragment shader does this manually.
*/
std::vector<vk::PipelineColorBlendAttachmentState> PipelineSDF::getPipelineColorBlendAttachmentStates() const
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

std::vector<vk::DescriptorSetLayoutBinding> PipelineSDF::createDescriptorSetLayoutBindings() const {
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
            narrow_cast<uint32_t>(DeviceShared::atlasMaximumNrImages), // descriptorCount
            vk::ShaderStageFlagBits::eFragment
        } 
    };
}

vector<vk::WriteDescriptorSet> PipelineSDF::createWriteDescriptorSet() const
{
    ttlet &sharedImagePipeline = vulkan_device().SDFPipeline;

    return {
        {
            descriptorSet,
            0, // destBinding
            0, // arrayElement
            1, // descriptorCount
            vk::DescriptorType::eInputAttachment,
            &(narrow_cast<gui_window_vulkan const&>(window).colorDescriptorImageInfo),
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

ssize_t PipelineSDF::getDescriptorSetVersion() const
{
    return std::ssize(vulkan_device().SDFPipeline->atlasTextures);
}

std::vector<vk::PushConstantRange> PipelineSDF::createPushConstantRanges() const
{
    return PushConstants::pushConstantRanges();
}

vk::VertexInputBindingDescription PipelineSDF::createVertexInputBindingDescription() const
{
    return Vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> PipelineSDF::createVertexInputAttributeDescriptions() const {
    return Vertex::inputAttributeDescriptions();
}

void PipelineSDF::buildVertexBuffers()
{
    using vertexIndexType = uint16_t;
    constexpr ssize_t numberOfVertices = 1 << (sizeof(vertexIndexType) * CHAR_BIT);

    vk::BufferCreateInfo const bufferCreateInfo = {
        vk::BufferCreateFlags(),
        sizeof (Vertex) * numberOfVertices,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive
    };
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    std::tie(vertexBuffer, vertexBufferAllocation) = vulkan_device().createBuffer(bufferCreateInfo, allocationCreateInfo);
    vertexBufferData = vulkan_device().mapMemory<Vertex>(vertexBufferAllocation);
}

void PipelineSDF::teardownVertexBuffers()
{
    vulkan_device().unmapMemory(vertexBufferAllocation);
    vulkan_device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

}
