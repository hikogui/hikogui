// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "gui_device_vulkan.hpp"

namespace tt::pipeline_image {

using namespace tt;
using namespace std;

pipeline_image::pipeline_image(gui_window const &window) :
    pipeline_vulkan(window)
{
}


void pipeline_image::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    pipeline_vulkan::drawInCommandBuffer(commandBuffer);

    vulkan_device().flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof (vertex));
    vulkan_device().imagePipeline->prepareAtlasForRendering();

    std::vector<vk::Buffer> tmpvertexBuffers = { vertexBuffer };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    tt_axiom(tmpvertexBuffers.size() == tmpOffsets.size());

    vulkan_device().imagePipeline->drawInCommandBuffer(commandBuffer);


    commandBuffer.bindVertexBuffers(0, tmpvertexBuffers, tmpOffsets);

    pushConstants.windowExtent = extent2{ narrow_cast<float>(extent.width) , narrow_cast<float>(extent.height) };
    pushConstants.viewportScale = { 2.0f / extent.width, 2.0f / extent.height };
    pushConstants.atlasExtent = { device_shared::atlasImageWidth, device_shared::atlasImageHeight };
    pushConstants.atlasScale = { 1.0f / device_shared::atlasImageWidth, 1.0f / device_shared::atlasImageHeight };
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

std::vector<vk::PipelineShaderStageCreateInfo> pipeline_image::createShaderStages() const {
    return vulkan_device().imagePipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> pipeline_image::createDescriptorSetLayoutBindings() const {
    return { {
        0, // binding
        vk::DescriptorType::eSampler,
        1, // descriptorCount
        vk::ShaderStageFlagBits::eFragment
    }, {
        1, // binding
        vk::DescriptorType::eSampledImage,
        narrow_cast<uint32_t>(device_shared::atlasMaximumNrImages), // descriptorCount
        vk::ShaderStageFlagBits::eFragment
    } };
}

vector<vk::WriteDescriptorSet> pipeline_image::createWriteDescriptorSet() const
{
    ttlet &sharedImagePipeline = vulkan_device().imagePipeline;

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
        narrow_cast<uint32_t>(sharedImagePipeline->atlasDescriptorImageInfos.size()), // descriptorCount
        vk::DescriptorType::eSampledImage,
        sharedImagePipeline->atlasDescriptorImageInfos.data(),
        nullptr, // bufferInfo
        nullptr // texelBufferView
    } };
}

ssize_t pipeline_image::getDescriptorSetVersion() const
{
    return std::ssize(vulkan_device().imagePipeline->atlasTextures);
}

std::vector<vk::PushConstantRange> pipeline_image::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

vk::VertexInputBindingDescription pipeline_image::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> pipeline_image::createVertexInputAttributeDescriptions() const {
    return vertex::inputAttributeDescriptions();
}

void pipeline_image::buildvertexBuffers()
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

void pipeline_image::teardownvertexBuffers()
{
    vulkan_device().unmapMemory(vertexBufferAllocation);
    vulkan_device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

}
