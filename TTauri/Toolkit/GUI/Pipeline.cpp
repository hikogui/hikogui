//
//  Pipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Pipeline.hpp"

#include <boost/assert.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include "Device.hpp"
#include "Window.hpp"
#include "TTauri/Toolkit/Logging.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

// Device may change when the window is moved, therefor make this indirect.
#define device (window->device)

Pipeline::Pipeline(Window *window) :
    window(window)
{
}

Pipeline::~Pipeline()
{
}

/*! Build the swapchain, frame buffers and pipeline.
 */
void Pipeline::buildPipeline(vk::RenderPass _renderPass, vk::Extent2D extent)
{
    LOG_INFO("buildPipeline (%i, %i)") % extent.width % extent.height;

    renderPass = _renderPass;

    shaderModules = createShaderModules();

    shaderStages = createShaderStages(shaderModules);

    pipelineLayout = createPipelineLayout();

    pipelineVertexInputStateCreateInfo = createPipelineVertexInputStateCreateInfo();

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

    intrinsic = device->intrinsic.createGraphicsPipeline(vk::PipelineCache(), graphicsPipelineCreateInfo);

    // Create a command buffer for each swapchain framebuffer, this way we can keep the same command in the command
    // buffer as long as no widgets are being added or removed (same number of triangles being rendered).
    auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(
        device->graphicQueue->commandPool,
        vk::CommandBufferLevel::ePrimary,
        boost::numeric_cast<uint32_t>(window->swapchainFramebuffers.size())
    );
    commandBuffers = device->intrinsic.allocateCommandBuffers(commandBufferAllocateInfo);

    commandBuffersValid.resize(commandBuffers.size());
    invalidateCommandBuffers();

    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    renderFinishedSemaphores.resize(commandBuffers.size());
    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
        renderFinishedSemaphores[i] = device->intrinsic.createSemaphore(semaphoreCreateInfo, nullptr);
    }
}

/*! Teardown the swapchain, frame buffers and pipeline.
 */
void Pipeline::teardownPipeline(void)
{
    for (size_t i = 0; i <  renderFinishedSemaphores.size(); i++) {
        device->intrinsic.destroy(renderFinishedSemaphores[i]);
    }

    device->intrinsic.freeCommandBuffers(device->graphicQueue->commandPool, commandBuffers);
    commandBuffers.clear();

    device->intrinsic.destroy(intrinsic);
    device->intrinsic.destroy(pipelineLayout);
    for (auto shaderModule: shaderModules) {
        device->intrinsic.destroy(shaderModule);
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

    commandBuffers[imageIndex].draw(3, 1, 0, 0);

    commandBuffers[imageIndex].endRenderPass();

    commandBuffers[imageIndex].end();

    commandBuffersValid[imageIndex] = true;
}

vk::Semaphore Pipeline::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    LOG_INFO("Render %i/%i") % imageIndex % renderFinishedSemaphores.size();
    validateCommandBuffer(imageIndex);

    vk::Semaphore waitSemaphores[] = {inputSemaphore};

    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[imageIndex]};
    
    vk::SubmitInfo submitInfo[] = {vk::SubmitInfo(
        1, waitSemaphores, waitStages,
        1, &commandBuffers[imageIndex],
        1, signalSemaphores
    )};

    device->graphicQueue->intrinsic.submit(1, submitInfo, vk::Fence());

    return renderFinishedSemaphores[imageIndex];
}

vk::ShaderModule Pipeline::loadShader(boost::filesystem::path path) const
{
    LOG_INFO("Loading shader %s") % path.filename().generic_string();

    auto mapped_file = boost::interprocess::file_mapping(path.c_str(), boost::interprocess::read_only);
    auto region = boost::interprocess::mapped_region(mapped_file, boost::interprocess::read_only);

    // Check uint32_t alignment of pointer.
    BOOST_ASSERT((reinterpret_cast<std::uintptr_t>(region.get_address()) & 3) == 0);

    auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo(
        vk::ShaderModuleCreateFlags(),
        region.get_size(),
        reinterpret_cast<uint32_t *>(region.get_address())
    );

    return device->intrinsic.createShaderModule(shaderModuleCreateInfo);
}

vk::PipelineLayout Pipeline::createPipelineLayout(void) const
{
    auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo();
    return device->intrinsic.createPipelineLayout(pipelineLayoutCreateInfo);
}

vk::PipelineVertexInputStateCreateInfo Pipeline::createPipelineVertexInputStateCreateInfo(void) const
{
    return {};
}

vk::PipelineInputAssemblyStateCreateInfo Pipeline::createPipelineInputAssemblyStateCreateInfo(void) const
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

vk::PipelineRasterizationStateCreateInfo Pipeline::createPipelineRasterizationStateCreateInfo(void) const
{
    return {
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE, // depthClampEnable
        VK_FALSE, // rasterizerDiscardEnable
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        VK_FALSE // depthBiasEnable
    };
}

vk::PipelineMultisampleStateCreateInfo Pipeline::createPipelineMultisampleStateCreateInfo(void) const
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

std::vector<vk::PipelineColorBlendAttachmentState> Pipeline::createPipelineColorBlendAttachmentStates(void) const
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

}}}
