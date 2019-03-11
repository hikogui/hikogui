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
#include "TTauri/Application.hpp"
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri {
namespace GUI {

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
    auto vertices = static_cast<Vertex *>(mapVertexBuffer());
    auto tmpNumberOfVertices = window->view->BackingPipelineRender(vertices, 0, maximumNumberOfVertices);
    unmapVertexBuffer();
  
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

    std::vector<vk::Buffer> vertexBuffers = { vertexBuffer };
    std::vector<vk::DeviceSize> offsets;
    for (size_t i = 0; i < vertexBuffers.size(); i++) {
        offsets.push_back(0);
    }
    commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

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
        loadShader(app->getPathToResource("BackingPipeline.vert.spv")),
        loadShader(app->getPathToResource("BackingPipeline.frag.spv"))
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


}}
