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
using namespace std;
using namespace gsl;

BackingPipeline_vulkan::BackingPipeline_vulkan(const std::shared_ptr<Window> &window) :
    Pipeline_vulkan(window)
{
}

vk::Semaphore BackingPipeline_vulkan::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    auto const tmpNumberOfVertices = window.lock()->view->backingPipelineRender(vertexBufferData.at(imageIndex), 0);

    if (vertexBufferNeedsFlushing) {
        auto const vertexDataOffsetAndSize = vertexBufferOffsetAndSizes.at(imageIndex);
        device<Device_vulkan>()->intrinsic.flushMappedMemoryRanges({ { vertexBufferMemory, vertexDataOffsetAndSize.first, tmpNumberOfVertices * sizeof (Vertex) } });
    }

    if (tmpNumberOfVertices != numberOfVertices) {
        invalidateCommandBuffers();
    }
    numberOfVertices = tmpNumberOfVertices;

    return Pipeline_vulkan::render(imageIndex, inputSemaphore);
}

void BackingPipeline_vulkan::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    pushConstants.windowExtent = { scissors.at(0).extent.width , scissors.at(0).extent.height };
    pushConstants.viewportScale = { 2.0 / scissors.at(0).extent.width, 2.0 / scissors.at(0).extent.height };
    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, 
        sizeof(PushConstants), 
        &pushConstants
    );

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
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, shaders.at(0), "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, shaders.at(1), "main"}
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

void BackingPipeline_vulkan::buildVertexBuffers(size_t nrFrameBuffers)
{
    Pipeline_vulkan::buildVertexBuffers(nrFrameBuffers);

    auto vulkanDevice = device<Device_vulkan>();

    auto const lastOffsetAndSize = vertexBufferOffsetAndSizes.back();
    auto const mappingSize = lastOffsetAndSize.first + lastOffsetAndSize.second;
    auto const mappingPointer = vulkanDevice->intrinsic.mapMemory(vertexBufferMemory, 0, mappingSize, vk::MemoryMapFlags());

    vertexBufferData = TTauri::transform<vector<span<Vertex>>>(vertexBufferOffsetAndSizes, [&mappingPointer](const pair<size_t, size_t> &offsetAndSize) -> span<Vertex> {
        [[gsl::suppress(type.1)]] {
            auto const offset = offsetAndSize.first;
            auto const size = offsetAndSize.second;

            return {
                reinterpret_cast<Vertex *>(reinterpret_cast<uintptr_t>(mappingPointer) + offset),
                boost::numeric_cast<uint32_t>(size / sizeof(Vertex))
            };
        }
    });
}

void BackingPipeline_vulkan::teardownVertexBuffers()
{
    auto vulkanDevice = device<Device_vulkan>();

    vulkanDevice->intrinsic.unmapMemory(vertexBufferMemory);

    vertexBufferData.clear();

    Pipeline_vulkan::teardownVertexBuffers();
}

}}
