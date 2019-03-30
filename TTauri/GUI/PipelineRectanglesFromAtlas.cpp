//
//  BackingPipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "PipelineRectanglesFromAtlas.hpp"
#include "Window.hpp"
#include "Device_vulkan.hpp"
#include "TTauri/Application.hpp"
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri {
namespace GUI {

using namespace TTauri;
using namespace std;
using namespace gsl;




PipelineRectanglesFromAtlas::PipelineRectanglesFromAtlas(const std::shared_ptr<Window> &window) :
    Pipeline_vulkan(window)
{
}

vk::Semaphore PipelineRectanglesFromAtlas::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    auto const tmpNumberOfVertices = window.lock()->view->piplineRectangledFromAtlasPlaceVertices(vertexBuffersData.at(imageIndex), 0);

    vmaFlushAllocation(device<Device_vulkan>()->allocator, vertexBuffersAllocation.at(imageIndex), 0, VK_WHOLE_SIZE);

    if (tmpNumberOfVertices != numberOfVertices) {
        invalidateCommandBuffers(false);
    }
    numberOfVertices = tmpNumberOfVertices;

    return Pipeline_vulkan::render(imageIndex, inputSemaphore);
}

void PipelineRectanglesFromAtlas::drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t imageIndex)
{
    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffers.at(imageIndex) };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    BOOST_ASSERT(tmpVertexBuffers.size() == tmpOffsets.size());

    commandBuffer.bindVertexBuffers(0, tmpVertexBuffers, tmpOffsets);
    commandBuffer.bindIndexBuffer(vertexIndexBuffer, 0, vk::IndexType::eUint16);

    pushConstants.windowExtent = { scissors.at(0).extent.width , scissors.at(0).extent.height };
    pushConstants.viewportScale = { 2.0 / scissors.at(0).extent.width, 2.0 / scissors.at(0).extent.height };
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

std::vector<vk::ShaderModule> PipelineRectanglesFromAtlas::createShaderModules() const
{
    return {
        loadShader(get_singleton<Application>()->resourceDir / "PipelineRectanglesFromAtlas.vert.spv"),
        loadShader(get_singleton<Application>()->resourceDir / "PipelineRectanglesFromAtlas.frag.spv")
    };
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineRectanglesFromAtlas::createShaderStages(const std::vector<vk::ShaderModule> &shaders) const
{
    return {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, shaders.at(0), "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, shaders.at(1), "main"}
    };
}

std::vector<vk::PushConstantRange> PipelineRectanglesFromAtlas::createPushConstantRanges() const
{
    return PushConstants::pushConstantRanges();
}


vk::VertexInputBindingDescription PipelineRectanglesFromAtlas::createVertexInputBindingDescription() const
{
    return Vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> PipelineRectanglesFromAtlas::createVertexInputAttributeDescriptions() const
{
    return Vertex::inputAttributeDescriptions();
}

void PipelineRectanglesFromAtlas::buildVertexBuffers(size_t nrFrameBuffers)
{
    auto vulkanDevice = device<Device_vulkan>();

    // Create vertex index buffer
    {
        vk::BufferCreateInfo bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (uint16_t) * maximumNumberOfVertexIndices(),
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        tie(vertexIndexBuffer, vertexIndexBufferAllocation) = vulkanDevice->createBuffer(bufferCreateInfo, allocationCreateInfo);
    }

    // Fill in the vertex index buffer, using a staging buffer, then copying.
    {
        // Create staging vertex index buffer.
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (uint16_t) * maximumNumberOfVertexIndices(),
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        vk::Buffer stagingVertexIndexBuffer;
        VmaAllocation stagingVertexIndexBufferAllocation;
        tie(stagingVertexIndexBuffer, stagingVertexIndexBufferAllocation) = vulkanDevice->createBuffer(bufferCreateInfo, allocationCreateInfo);

        // Initialize indices.
        auto const stagingVertexIndexBufferData = vulkanDevice->mapMemory<uint16_t>(stagingVertexIndexBufferAllocation);
        for (size_t i = 0; i < maximumNumberOfVertexIndices(); i++) {
            auto const vertexInRectangle = i % 6;
            auto const rectangleNr = i / 6;
            auto const rectangleBase = rectangleNr * 4;

            switch (vertexInRectangle) {
            case 0: gsl::at(stagingVertexIndexBufferData, i) = rectangleBase + 0; break;
            case 1: gsl::at(stagingVertexIndexBufferData, i) = rectangleBase + 1; break;
            case 2: gsl::at(stagingVertexIndexBufferData, i) = rectangleBase + 2; break;
            case 3: gsl::at(stagingVertexIndexBufferData, i) = rectangleBase + 2; break;
            case 4: gsl::at(stagingVertexIndexBufferData, i) = rectangleBase + 1; break;
            case 5: gsl::at(stagingVertexIndexBufferData, i) = rectangleBase + 3; break;
            }
        }
        vmaFlushAllocation(vulkanDevice->allocator, stagingVertexIndexBufferAllocation, 0, VK_WHOLE_SIZE);
        vulkanDevice->unmapMemory(stagingVertexIndexBufferAllocation);

        // Copy indices to vertex index buffer.
        auto commands = vulkanDevice->intrinsic.allocateCommandBuffers({
            vulkanDevice->graphicsCommandPool, 
            vk::CommandBufferLevel::ePrimary, 
            1
        }).at(0);
        commands.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        commands.copyBuffer(stagingVertexIndexBuffer, vertexIndexBuffer, {{0, 0, sizeof (uint16_t) * maximumNumberOfVertexIndices()}});
        commands.end();

        vector<vk::CommandBuffer> const commandBuffersToSubmit = { commands };
        vector<vk::SubmitInfo> const submitInfo = { { 0, nullptr, nullptr, boost::numeric_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(), 0, nullptr } };
        vulkanDevice->graphicsQueue.submit(submitInfo, vk::Fence());
        vulkanDevice->graphicsQueue.waitIdle();

        vulkanDevice->intrinsic.freeCommandBuffers(vulkanDevice->graphicsCommandPool, {commands});
        vulkanDevice->destroyBuffer(stagingVertexIndexBuffer, stagingVertexIndexBufferAllocation);
    }

    BOOST_ASSERT(vertexBuffers.size() == 0);
    BOOST_ASSERT(vertexBuffersAllocation.size() == 0);
    BOOST_ASSERT(vertexBuffersData.size() == 0);
    for (size_t i = 0; i < nrFrameBuffers; i++) {
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (uint16_t) * maximumNumberOfVertexIndices(),
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        vk::Buffer vertexBuffer;
        VmaAllocation vertexBufferAllocation;
        tie(vertexBuffer, vertexBufferAllocation) = vulkanDevice->createBuffer(bufferCreateInfo, allocationCreateInfo);
        auto const vertexBufferData = vulkanDevice->mapMemory<Vertex>(vertexBufferAllocation);

        vertexBuffers.push_back(vertexBuffer);
        vertexBuffersAllocation.push_back(vertexBufferAllocation);
        vertexBuffersData.push_back(vertexBufferData);
    }
}

void PipelineRectanglesFromAtlas::teardownVertexBuffers()
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

    vulkanDevice->destroyBuffer(vertexIndexBuffer, vertexIndexBufferAllocation);
}

}}
