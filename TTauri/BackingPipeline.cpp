//
//  BackingPipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "BackingPipeline.hpp"
#include "Window.hpp"
#include "Device.hpp"
#include "Application.hpp"
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri {

using namespace TTauri;

BackingPipeline::BackingPipeline(Window *window) :
    Pipeline(window)
{
}

BackingPipeline::~BackingPipeline()
{
}

vk::Semaphore BackingPipeline::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    auto vertexDataOffset = vertexBufferOffsets[imageIndex];
    auto vertexDataSize = vertexBufferSizes[imageIndex];
    auto vertices = reinterpret_cast<Vertex *>(reinterpret_cast<char *>(vertexBufferData) + vertexDataOffset);

    auto tmpNumberOfVertices = window->view->BackingPipelineRender(vertices, 0, maximumNumberOfVertices());

    device()->intrinsic.flushMappedMemoryRanges({{vertexBufferMemory, vertexDataOffset, vertexDataSize}});

    if (tmpNumberOfVertices != numberOfVertices) {
        invalidateCommandBuffers();
    }
    numberOfVertices = tmpNumberOfVertices;

    return Pipeline::render(imageIndex, inputSemaphore);
}

void BackingPipeline::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
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

std::vector<vk::ShaderModule> BackingPipeline::createShaderModules() const
{
    return {
        loadShader(Application::shared->resourceDir / "BackingPipeline.vert.spv"),
        loadShader(Application::shared->resourceDir / "BackingPipeline.frag.spv")
    };
}

std::vector<vk::PipelineShaderStageCreateInfo> BackingPipeline::createShaderStages(const std::vector<vk::ShaderModule> &shaders) const
{
    return {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, shaders[0], "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, shaders[1], "main"}
    };
}

std::vector<vk::PushConstantRange> BackingPipeline::createPushConstantRanges() const
{
    return PushConstants::pushConstantRanges();
}


vk::VertexInputBindingDescription BackingPipeline::createVertexInputBindingDescription() const
{
    return Vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> BackingPipeline::createVertexInputAttributeDescriptions() const
{
    return Vertex::inputAttributeDescriptions();
}

}
