// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineSDF.hpp"
#include "TTauri/GUI/PipelineSDF_DeviceShared.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/Device.hpp"

namespace TTauri::GUI::PipelineSDF {

using namespace TTauri;
using namespace std;
using namespace gsl;

PipelineSDF::PipelineSDF(Window const &window) :
    Pipeline_vulkan(window)
{
}

vk::Semaphore PipelineSDF::render(uint32_t frameBufferIndex, vk::Semaphore inputSemaphore)
{
    ssize_t tmpNumberOfVertices = 0;
    window.widget->pipelineSDFPlaceVertices(vertexBuffersData.at(frameBufferIndex), tmpNumberOfVertices);

    device().flushAllocation(vertexBuffersAllocation.at(frameBufferIndex), 0, tmpNumberOfVertices * sizeof (Vertex));

    device().SDFPipeline->prepareAtlasForRendering();
   
    if (tmpNumberOfVertices != numberOfVertices) {
        invalidateCommandBuffers();
        numberOfVertices = tmpNumberOfVertices;
    }

    return Pipeline_vulkan::render(frameBufferIndex, inputSemaphore);
}

void PipelineSDF::drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t frameBufferIndex)
{
    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffers.at(frameBufferIndex) };
    std::vector<vk::DeviceSize> tmpOffsets = { 0 };
    BOOST_ASSERT(tmpVertexBuffers.size() == tmpOffsets.size());

    device().SDFPipeline->drawInCommandBuffer(commandBuffer);


    commandBuffer.bindVertexBuffers(0, tmpVertexBuffers, tmpOffsets);

    pushConstants.windowExtent = { extent.width , extent.height };
    pushConstants.viewportScale = { 2.0 / extent.width, 2.0 / extent.height };
    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, 
        sizeof(PushConstants), 
        &pushConstants
    );

    let numberOfRectangles = numberOfVertices / 4;
    let numberOfTriangles = numberOfRectangles * 2;
    commandBuffer.drawIndexed(
        numeric_cast<uint32_t>(numberOfTriangles * 3),
        1,
        0,
        0,
        0
    );
}

std::vector<vk::PipelineShaderStageCreateInfo> PipelineSDF::createShaderStages() const {
    return device().SDFPipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> PipelineSDF::createDescriptorSetLayoutBindings() const {
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

vector<vk::WriteDescriptorSet> PipelineSDF::createWriteDescriptorSet(uint32_t frameBufferIndex) const
{
    let &sharedImagePipeline = device().SDFPipeline;
    let &frameBufferObject = frameBufferObjects.at(frameBufferIndex);

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
        numeric_cast<uint32_t>(sharedImagePipeline->atlasDescriptorImageInfos.size()), // descriptorCount
        vk::DescriptorType::eSampledImage,
        sharedImagePipeline->atlasDescriptorImageInfos.data(),
        nullptr, // bufferInfo
        nullptr // texelBufferView
    } };
}

ssize_t PipelineSDF::getDescriptorSetVersion() const
{
    return to_signed(device().SDFPipeline->atlasTextures.size());
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

void PipelineSDF::buildVertexBuffers(int nrFrameBuffers)
{
    BOOST_ASSERT(vertexBuffers.size() == 0);
    BOOST_ASSERT(vertexBuffersAllocation.size() == 0);
    BOOST_ASSERT(vertexBuffersData.size() == 0);
    for (size_t i = 0; i < nrFrameBuffers; i++) {
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (Vertex) * PipelineSDF::maximumNumberOfVertices,
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        let [vertexBuffer, vertexBufferAllocation] = device().createBuffer(bufferCreateInfo, allocationCreateInfo);
        let vertexBufferData = device().mapMemory<Vertex>(vertexBufferAllocation);

        vertexBuffers.push_back(vertexBuffer);
        vertexBuffersAllocation.push_back(vertexBufferAllocation);
        vertexBuffersData.push_back(vertexBufferData);
    }
}

void PipelineSDF::teardownVertexBuffers()
{
    BOOST_ASSERT(vertexBuffers.size() == vertexBuffersAllocation.size());
    for (size_t i = 0; i < vertexBuffers.size(); i++) {
        auto vertexBuffer = vertexBuffers.at(i);
        auto vertexBufferAllocation = vertexBuffersAllocation.at(i);

        device().unmapMemory(vertexBufferAllocation);
        device().destroyBuffer(vertexBuffer, vertexBufferAllocation);
    }
    vertexBuffers.clear();
    vertexBuffersAllocation.clear();
    vertexBuffersData.clear();
}

}
