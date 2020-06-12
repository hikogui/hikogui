// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineImage.hpp"
#include "TTauri/GUI/PipelineImage_DeviceShared.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/GUIDevice.hpp"

namespace tt::PipelineImage {

using namespace tt;
using namespace std;

PipelineImage::PipelineImage(Window const &window) :
    Pipeline_vulkan(window)
{
}


void PipelineImage::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    Pipeline_vulkan::drawInCommandBuffer(commandBuffer);

    device().flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof (Vertex));
    device().imagePipeline->prepareAtlasForRendering();

    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffer };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    tt_assume(tmpVertexBuffers.size() == tmpOffsets.size());

    device().imagePipeline->drawInCommandBuffer(commandBuffer);


    commandBuffer.bindVertexBuffers(0, tmpVertexBuffers, tmpOffsets);

    pushConstants.windowExtent = { extent.width , extent.height };
    pushConstants.viewportScale = { 2.0 / extent.width, 2.0 / extent.height };
    pushConstants.atlasExtent = { DeviceShared::atlasImageWidth, DeviceShared::atlasImageHeight };
    pushConstants.atlasScale = { 1.0 / DeviceShared::atlasImageWidth, 1.0 / DeviceShared::atlasImageHeight };
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
        numeric_cast<uint32_t>(numberOfTriangles * 3),
        1,
        0,
        0,
        0
    );
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineImage::createShaderStages() const {
    return device().imagePipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> PipelineImage::createDescriptorSetLayoutBindings() const {
    return { {
        0, // binding
        vk::DescriptorType::eSampler,
        1, // descriptorCount
        vk::ShaderStageFlagBits::eFragment
    }, {
        1, // binding
        vk::DescriptorType::eSampledImage,
        numeric_cast<uint32_t>(DeviceShared::atlasMaximumNrImages), // descriptorCount
        vk::ShaderStageFlagBits::eFragment
    } };
}

vector<vk::WriteDescriptorSet> PipelineImage::createWriteDescriptorSet() const
{
    ttlet &sharedImagePipeline = device().imagePipeline;

    return { {
        descriptorSet,
        0, // destBinding
        0, // arrayElement
        1, // descriptorCount
        vk::DescriptorType::eSampler,
        &sharedImagePipeline->atlasSamplerDescriptorImageInfo,
        nullptr,  // bufferInfo
        nullptr // texelBufferView
    }, {
        descriptorSet,
        1, // destBinding
        0, // arrayElement
        numeric_cast<uint32_t>(sharedImagePipeline->atlasDescriptorImageInfos.size()), // descriptorCount
        vk::DescriptorType::eSampledImage,
        sharedImagePipeline->atlasDescriptorImageInfos.data(),
        nullptr, // bufferInfo
        nullptr // texelBufferView
    } };
}

ssize_t PipelineImage::getDescriptorSetVersion() const
{
    return ssize(device().imagePipeline->atlasTextures);
}

std::vector<vk::PushConstantRange> PipelineImage::createPushConstantRanges() const
{
    return PushConstants::pushConstantRanges();
}

vk::VertexInputBindingDescription PipelineImage::createVertexInputBindingDescription() const
{
    return Vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> PipelineImage::createVertexInputAttributeDescriptions() const {
    return Vertex::inputAttributeDescriptions();
}

void PipelineImage::buildVertexBuffers()
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

    std::tie(vertexBuffer, vertexBufferAllocation) = device().createBuffer(bufferCreateInfo, allocationCreateInfo);
    vertexBufferData = device().mapMemory<Vertex>(vertexBufferAllocation);
}

void PipelineImage::teardownVertexBuffers()
{
    device().unmapMemory(vertexBufferAllocation);
    device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

}
