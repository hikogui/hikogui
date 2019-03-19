//
//  BackingPipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "BackingPipeline_vulkan.hpp"
#include "Window.hpp"
#include "Device_vulkan.hpp"
#include "TTauri/Application.hpp"
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri {
namespace GUI {

using namespace TTauri;

BackingPipeline_vulkan::BackingPipeline_vulkan(const std::shared_ptr<Window> &window) :
    Pipeline_vulkan(window)
{
}

BackingPipeline_vulkan::~BackingPipeline_vulkan()
{
}

vk::Semaphore BackingPipeline_vulkan::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    auto vertexDataOffset = vertexBufferOffsets[imageIndex];
    auto vertexDataSize = vertexBufferSizes[imageIndex];
    auto vertices = reinterpret_cast<Vertex *>(reinterpret_cast<char *>(vertexBufferData) + vertexDataOffset);

    auto tmpNumberOfVertices = window.lock()->view->BackingPipelineRender(vertices, 0, maximumNumberOfVertices());

    device<Device_vulkan>()->intrinsic.flushMappedMemoryRanges({ { vertexBufferMemory, vertexDataOffset, vertexDataSize } });

    if (tmpNumberOfVertices != numberOfVertices) {
        invalidateCommandBuffers();
    }
    numberOfVertices = tmpNumberOfVertices;

    return Pipeline_vulkan::render(imageIndex, inputSemaphore);
}

void BackingPipeline_vulkan::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    pushConstants.windowExtent = { scissors[0].extent.width , scissors[0].extent.height };
    pushConstants.viewportScale = { 2.0 / scissors[0].extent.width, 2.0 / scissors[0].extent.height };
    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants), static_cast<const void *>(&pushConstants));

    commandBuffer.draw(
        boost::numeric_cast<uint32_t>(numberOfVertices),
        1,
        0,
        0
    );
}

std::vector<vk::ShaderModule> BackingPipeline_vulkan::createShaderModules() const
{
    return {
        loadShader(get_singleton<Application>()->resourceDir / "BackingPipeline_vulkan.vert.spv"),
        loadShader(get_singleton<Application>()->resourceDir / "BackingPipeline_vulkan.frag.spv")
    };
}

std::vector<vk::PipelineShaderStageCreateInfo> BackingPipeline_vulkan::createShaderStages(const std::vector<vk::ShaderModule> &shaders) const
{
    return {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, shaders[0], "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, shaders[1], "main"}
    };
}

std::vector<vk::PushConstantRange> BackingPipeline_vulkan::createPushConstantRanges() const
{
    return PushConstants::pushConstantRanges();
}


vk::VertexInputBindingDescription BackingPipeline_vulkan::createVertexInputBindingDescription() const
{
    return Vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> BackingPipeline_vulkan::createVertexInputAttributeDescriptions() const
{
    return Vertex::inputAttributeDescriptions();
}

}}
