//
//  Pipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Pipeline.hpp"
#include "Device.hpp"
#include "Window.hpp"
#include "TTauri/Logging.hpp"
#include <boost/assert.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri {
namespace GUI {

Pipeline::Pipeline(Window *window) :
    window(window)
{
}

Pipeline::~Pipeline()
{
}

Device *Pipeline::device() const
{
    BOOST_ASSERT(window);
    return window->device;
}

vk::Semaphore Pipeline::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    validateCommandBuffer(imageIndex);

    vk::Semaphore waitSemaphores[] = { inputSemaphore };

    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex] };

    vk::SubmitInfo submitInfo[] = { vk::SubmitInfo(
        1, waitSemaphores, waitStages,
        1, &commandBuffers[imageIndex],
        1, signalSemaphores
    ) };

    device()->graphicQueue->intrinsic.submit(1, submitInfo, vk::Fence());

    return renderFinishedSemaphores[imageIndex];
}

/*! Build the swapchain, frame buffers and pipeline.
 */
void Pipeline::buildPipeline(vk::RenderPass _renderPass, vk::Extent2D extent, size_t maximumNumberOfTriangles)
{
    LOG_INFO("buildPipeline (%i, %i)") % extent.width % extent.height;

    this->maximumNumberOfTriangles = maximumNumberOfTriangles;
    maximumNumberOfVertices = maximumNumberOfTriangles * 3;

    renderPass = _renderPass;

    shaderModules = createShaderModules();

    shaderStages = createShaderStages(shaderModules);

    pipelineLayout = createPipelineLayout();

    vertexInputBindingDescription = createVertexInputBindingDescription();
    vertexInputAttributeDescriptions = createVertexInputAttributeDescriptions();
    pipelineVertexInputStateCreateInfo = createPipelineVertexInputStateCreateInfo(vertexInputBindingDescription, vertexInputAttributeDescriptions);

    pipelineInputAssemblyStateCreateInfo = createPipelineInputAssemblyStateCreateInfo();

    viewports = createViewports(extent);

    scissors = createScissors(extent);

    pipelineViewportStateCreateInfo = createPipelineViewportStateCreateInfo(viewports, scissors);

    pipelineRasterizationStateCreateInfo = createPipelineRasterizationStateCreateInfo();

    pipelineMultisampleStateCreateInfo = createPipelineMultisampleStateCreateInfo();

    pipelineColorBlendAttachmentStates = createPipelineColorBlendAttachmentStates();

    pipelineColorBlendStateCreateInfo = createPipelineColorBlendStateCreateInfo(pipelineColorBlendAttachmentStates);

    graphicsPipelineCreateInfo = {
        vk::PipelineCreateFlags(),
        boost::numeric_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
        &pipelineVertexInputStateCreateInfo,
        &pipelineInputAssemblyStateCreateInfo,
        nullptr, // tesselationStateCreateInfo
        &pipelineViewportStateCreateInfo,
        &pipelineRasterizationStateCreateInfo,
        &pipelineMultisampleStateCreateInfo,
        nullptr, // pipelineDepthStencilCrateInfo
        &pipelineColorBlendStateCreateInfo,
        nullptr, // pipelineDynamicsStateCreateInfo
        pipelineLayout,
        renderPass,
        0, // subpass
        vk::Pipeline(), // basePipelineHandle
        -1 // basePipelineIndex
    };

    intrinsic = device()->intrinsic.createGraphicsPipeline(vk::PipelineCache(), graphicsPipelineCreateInfo);

    vertexBuffer = createVertexBuffer(vertexInputBindingDescription.stride, maximumNumberOfVertices);
    vertexBufferMemory = device()->allocateDeviceMemoryAndBind(vertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible);

    // Create a command buffer for each swapchain framebuffer, this way we can keep the same command in the command
    // buffer as long as no widgets are being added or removed (same number of triangles being rendered).
    auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(
        device()->graphicQueue->commandPool,
        vk::CommandBufferLevel::ePrimary,
        boost::numeric_cast<uint32_t>(window->swapchainFramebuffers.size())
    );
    commandBuffers = device()->intrinsic.allocateCommandBuffers(commandBufferAllocateInfo);

    commandBuffersValid.resize(commandBuffers.size());
    invalidateCommandBuffers();

    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    renderFinishedSemaphores.resize(commandBuffers.size());
    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
        renderFinishedSemaphores[i] = device()->intrinsic.createSemaphore(semaphoreCreateInfo, nullptr);
    }
}

/*! Teardown the swapchain, frame buffers and pipeline.
 */
void Pipeline::teardownPipeline()
{
    for (size_t i = 0; i <  renderFinishedSemaphores.size(); i++) {
        device()->intrinsic.destroy(renderFinishedSemaphores[i]);
    }

    device()->intrinsic.freeCommandBuffers(device()->graphicQueue->commandPool, commandBuffers);
    commandBuffers.clear();

    device()->intrinsic.destroy(vertexBuffer);
    device()->intrinsic.free(vertexBufferMemory);
    device()->intrinsic.destroy(intrinsic);
    device()->intrinsic.destroy(pipelineLayout);
    for (auto shaderModule: shaderModules) {
        device()->intrinsic.destroy(shaderModule);
    }
}

void Pipeline::invalidateCommandBuffers()
{
    for (size_t imageIndex = 0; imageIndex < commandBuffersValid.size(); imageIndex++) {
        commandBuffersValid[imageIndex] = false;
    }
}

void Pipeline::validateCommandBuffer(uint32_t imageIndex)
{
    if (commandBuffersValid[imageIndex]) {
        return;
    }

    LOG_INFO("validateCommandBuffer %i (%i, %i)") % imageIndex % scissors[0].extent.width % scissors[0].extent.height;

    commandBuffers[imageIndex].reset(vk::CommandBufferResetFlagBits::eReleaseResources);

    auto commandBufferBeginInfo = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    commandBuffers[imageIndex].begin(commandBufferBeginInfo);

    std::array<float,4> blackColor = {0.0f, 0.0f, 0.0f, 1.0f};
    auto clearColor = vk::ClearValue(vk::ClearColorValue(blackColor));
    auto renderPassBeginInfo = vk::RenderPassBeginInfo(
        renderPass,
        window->swapchainFramebuffers[imageIndex],
        scissors[0],
        1,
        &clearColor
    );
    commandBuffers[imageIndex].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffers[imageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, intrinsic);

    drawInCommandBuffer(commandBuffers[imageIndex]);
    
    commandBuffers[imageIndex].endRenderPass();

    commandBuffers[imageIndex].end();

    commandBuffersValid[imageIndex] = true;
}


vk::ShaderModule Pipeline::loadShader(boost::filesystem::path path) const
{
    LOG_INFO("Loading shader %s") % path.filename().generic_string();

	auto tmp_path = path.generic_string();
	boost::interprocess::file_mapping mapped_file(tmp_path.c_str(), boost::interprocess::read_only);
    auto region = boost::interprocess::mapped_region(mapped_file, boost::interprocess::read_only);

    // Check uint32_t alignment of pointer.
    BOOST_ASSERT((reinterpret_cast<std::uintptr_t>(region.get_address()) & 3) == 0);

    auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo(
        vk::ShaderModuleCreateFlags(),
        region.get_size(),
        reinterpret_cast<uint32_t *>(region.get_address())
    );

    return device()->intrinsic.createShaderModule(shaderModuleCreateInfo);
}

vk::PipelineLayout Pipeline::createPipelineLayout() const
{
    auto pushConstantRanges = createPushConstantRanges();

    auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(),
        0, nullptr,
        boost::numeric_cast<uint32_t>(pushConstantRanges.size()), pushConstantRanges.data()
    );
    return device()->intrinsic.createPipelineLayout(pipelineLayoutCreateInfo);
}

vk::PipelineVertexInputStateCreateInfo Pipeline::createPipelineVertexInputStateCreateInfo(const vk::VertexInputBindingDescription &vertexBindingDescriptions, const std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) const
{
    return {
        vk::PipelineVertexInputStateCreateFlags(),
        1,
        &vertexBindingDescriptions,
        boost::numeric_cast<uint32_t>(vertexAttributeDescriptions.size()), vertexAttributeDescriptions.data()
    };
}

vk::PipelineInputAssemblyStateCreateInfo Pipeline::createPipelineInputAssemblyStateCreateInfo() const
{
    return {
        vk::PipelineInputAssemblyStateCreateFlags(),
        vk::PrimitiveTopology::eTriangleList,
        VK_FALSE
    };
}

std::vector<vk::Viewport> Pipeline::createViewports(vk::Extent2D extent) const
{
    return {{
        0.0f,
        0.0f,
        boost::numeric_cast<float>(extent.width),
        boost::numeric_cast<float>(extent.height),
        0.0f,
        1.0f
    }};
}

std::vector<vk::Rect2D> Pipeline::createScissors(vk::Extent2D extent) const
{
    return {{
        {0, 0},
        extent
    }};
}

vk::PipelineViewportStateCreateInfo Pipeline::createPipelineViewportStateCreateInfo(const std::vector<vk::Viewport> &viewports, std::vector<vk::Rect2D> &scissors) const
{
    return {
        vk::PipelineViewportStateCreateFlags(),
        boost::numeric_cast<uint32_t>(viewports.size()), viewports.data(),
        boost::numeric_cast<uint32_t>(scissors.size()), scissors.data()
    };
}

vk::PipelineRasterizationStateCreateInfo Pipeline::createPipelineRasterizationStateCreateInfo() const
{
    return {
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE, // depthClampEnable
        VK_FALSE, // rasterizerDiscardEnable
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        VK_FALSE, // depthBiasEnable
        0.0, // depthBiasConstantFactor
        0.0, // depthBiasClamp
        0.0, // depthBiasSlopeFactor
        1.0 // lineWidth
    };
}

vk::PipelineMultisampleStateCreateInfo Pipeline::createPipelineMultisampleStateCreateInfo() const
{
    return {
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1,
        VK_FALSE, // sampleShadingEnable
        1.0f, // minSampleShading
        nullptr, // sampleMask
        VK_FALSE, // alphaToCoverageEnable
        VK_FALSE // alphaToOneEnable
    };
}

std::vector<vk::PipelineColorBlendAttachmentState> Pipeline::createPipelineColorBlendAttachmentStates() const
{
    return {{
        VK_FALSE, // blendEnable
        vk::BlendFactor::eOne, // srcColorBlendFactor
        vk::BlendFactor::eZero, // dstColorBlendFactor
        vk::BlendOp::eAdd, // colorBlendOp
        vk::BlendFactor::eOne, // srcAlphaBlendFactor
        vk::BlendFactor::eZero, // dstAlphaBlendFactor
        vk::BlendOp::eAdd, // aphaBlendOp
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    }};
}

vk::PipelineColorBlendStateCreateInfo Pipeline::createPipelineColorBlendStateCreateInfo(const std::vector<vk::PipelineColorBlendAttachmentState> &attachements) const
{
    return {
        vk::PipelineColorBlendStateCreateFlags(),
        VK_FALSE, // logicOpenable
        vk::LogicOp::eCopy,
        boost::numeric_cast<uint32_t>(attachements.size()), attachements.data()
    };
}

vk::Buffer Pipeline::createVertexBuffer(size_t vertexSize, size_t numberOfVertices) const
{
    vk::BufferCreateInfo vertexBufferCreateInfo = {
        vk::BufferCreateFlags(),
        vertexInputBindingDescription.stride * maximumNumberOfVertices,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive
    };
    return device()->intrinsic.createBuffer(vertexBufferCreateInfo, nullptr);
}

void *Pipeline::mapVertexBuffer() const
{
    auto memoryRequirements = device()->intrinsic.getBufferMemoryRequirements(vertexBuffer);
    return device()->intrinsic.mapMemory(vertexBufferMemory, 0, memoryRequirements.size, vk::MemoryMapFlags());
}

void Pipeline::unmapVertexBuffer() const
{
    auto memoryRequirements = device()->intrinsic.getBufferMemoryRequirements(vertexBuffer);

    std::vector<vk::MappedMemoryRange> mappedMemoryRanges = {
        {vertexBufferMemory, 0, memoryRequirements.size}
    };
    device()->intrinsic.flushMappedMemoryRanges(mappedMemoryRanges);
    device()->intrinsic.unmapMemory(vertexBufferMemory);
}

}}
