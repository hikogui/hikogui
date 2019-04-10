//
//  BackingPipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "PipelineImage.hpp"
#include "PipelineImage_DeviceShared.hpp"
#include "Window.hpp"
#include "Device_vulkan.hpp"
#include "TTauri/Application.hpp"
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri::GUI {

using namespace TTauri;
using namespace std;
using namespace gsl;

PipelineImage::PipelineImage(const std::shared_ptr<Window> window) :
    Pipeline_vulkan(move(window))
{
}

vk::Semaphore PipelineImage::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    size_t tmpNumberOfVertices = 0;
    window.lock()->view->pipelineImagePlaceVertices(vertexBuffersData.at(imageIndex), tmpNumberOfVertices);

    vmaFlushAllocation(device<Device_vulkan>()->allocator, vertexBuffersAllocation.at(imageIndex), 0, tmpNumberOfVertices * sizeof (PipelineImage::Vertex));

    auto const vulkanDevice = device<Device_vulkan>();
    auto const sharedImagePipeline = vulkanDevice->imagePipeline;
    auto const nrAtlasImages = sharedImagePipeline->atlasImages.size();
    

    if (tmpNumberOfVertices != numberOfVertices) {
        invalidateCommandBuffers(false);
        numberOfVertices = tmpNumberOfVertices;
    }

    return Pipeline_vulkan::render(imageIndex, inputSemaphore);
}

void PipelineImage::drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t imageIndex)
{
    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffers.at(imageIndex) };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    BOOST_ASSERT(tmpVertexBuffers.size() == tmpOffsets.size());

    device<Device_vulkan>()->imagePipeline->drawInCommandBuffer(commandBuffer);


    commandBuffer.bindVertexBuffers(0, tmpVertexBuffers, tmpOffsets);

    pushConstants.windowExtent = { extent.width , extent.height };
    pushConstants.viewportScale = { 2.0 / extent.width, 2.0 / extent.height };
    pushConstants.atlasExtent = { PipelineImage::DeviceShared::atlasImageWidth, PipelineImage::DeviceShared::atlasImageHeight };
    pushConstants.atlasScale = { 1.0 / PipelineImage::DeviceShared::atlasImageWidth, 1.0 / PipelineImage::DeviceShared::atlasImageHeight };
    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, 
        sizeof(PushConstants), 
        &pushConstants
    );

    auto const numberOfRectangles = numberOfVertices / 4;
    auto const numberOfTriangles = numberOfRectangles * 2;
    commandBuffer.drawIndexed(
        boost::numeric_cast<uint32_t>(numberOfTriangles * 3),
        1,
        0,
        0,
        0
    );
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineImage::createShaderStages() const {
    return device<Device_vulkan>()->imagePipeline->shaderStages;
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
        boost::numeric_cast<uint32_t>(PipelineImage::DeviceShared::atlasMaximumNrImages), // descriptorCount
        vk::ShaderStageFlagBits::eFragment
    } };
}

vector<vk::WriteDescriptorSet> PipelineImage::createWriteDescriptorSet(uint32_t imageIndex) const
{
    auto const vulkanDevice = device<Device_vulkan>();
    auto const sharedImagePipeline = vulkanDevice->imagePipeline;
    auto const &frameBufferObject = frameBufferObjects.at(imageIndex);

    return { {
        frameBufferObject.descriptorSet,
        0, // destBinding
        0, // arrayElement
        1, // descriptorCount
        vk::DescriptorType::eSampler,
        &sharedImagePipeline->atlasSamplerDescriptorImageInfo,
        nullptr,  // bufferInfo
        nullptr // texelBufferView
    }, {
        frameBufferObject.descriptorSet,
        1, // destBinding
        0, // arrayElement
        boost::numeric_cast<uint32_t>(sharedImagePipeline->atlasDescriptorImageInfos.size()), // descriptorCount
        vk::DescriptorType::eSampledImage,
        sharedImagePipeline->atlasDescriptorImageInfos.data(),
        nullptr, // bufferInfo
        nullptr // texelBufferView
    } };
}

uint64_t PipelineImage::getDescriptorSetVersion() const
{
    return device<Device_vulkan>()->imagePipeline->atlasImages.size();
}

void PipelineImage::buildVertexBuffers(size_t nrFrameBuffers)
{
    auto vulkanDevice = device<Device_vulkan>();

    BOOST_ASSERT(vertexBuffers.size() == 0);
    BOOST_ASSERT(vertexBuffersAllocation.size() == 0);
    BOOST_ASSERT(vertexBuffersData.size() == 0);
    for (size_t i = 0; i < nrFrameBuffers; i++) {
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (Vertex) * PipelineImage::maximumNumberOfVertices,
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        auto [vertexBuffer, vertexBufferAllocation] = vulkanDevice->createBuffer(bufferCreateInfo, allocationCreateInfo);
        auto const vertexBufferData = vulkanDevice->mapMemory<Vertex>(vertexBufferAllocation);

        vertexBuffers.push_back(vertexBuffer);
        vertexBuffersAllocation.push_back(vertexBufferAllocation);
        vertexBuffersData.push_back(vertexBufferData);
    }
}

void PipelineImage::teardownVertexBuffers()
{
    auto vulkanDevice = device<Device_vulkan>();

    BOOST_ASSERT(vertexBuffers.size() == vertexBuffersAllocation.size());
    for (size_t i = 0; i < vertexBuffers.size(); i++) {
        auto vertexBuffer = vertexBuffers.at(i);
        auto vertexBufferAllocation = vertexBuffersAllocation.at(i);

        vulkanDevice->unmapMemory(vertexBufferAllocation);
        vulkanDevice->destroyBuffer(vertexBuffer, vertexBufferAllocation);
    }
    vertexBuffers.clear();
    vertexBuffersAllocation.clear();
    vertexBuffersData.clear();
}

}