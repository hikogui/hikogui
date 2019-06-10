// Copyright 2019 Pokitec
// All rights reserved.

#include "Pipeline_vulkan.hpp"
#include "Device.hpp"
#include "Window.hpp"
#include "TTauri/all.hpp"
#include <boost/assert.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri::GUI {

using namespace std;

Pipeline_vulkan::Pipeline_vulkan(const std::shared_ptr<Window> window) :
    Pipeline_base(std::move(window)) {}

Pipeline_vulkan::~Pipeline_vulkan()
{
}

vk::Semaphore Pipeline_vulkan::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    let vulkanDevice = device();
    auto &imageObject = frameBufferObjects.at(imageIndex);

    if (imageObject.descriptorSetVersion < getDescriptorSetVersion()) {
        let writeDescriptorSets = createWriteDescriptorSet(imageIndex);
        vulkanDevice->updateDescriptorSets(writeDescriptorSets, {});

        imageObject.descriptorSetVersion = getDescriptorSetVersion();
    }

    validateCommandBuffer(imageIndex);

    std::array<vk::Semaphore, 1> const waitSemaphores = { inputSemaphore };
    std::array<vk::PipelineStageFlags, 1> const waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    BOOST_ASSERT(waitSemaphores.size() == waitStages.size());

    std::array<vk::Semaphore, 1> const signalSemaphores = { imageObject.renderFinishedSemaphore };
    std::array<vk::CommandBuffer, 1> const commandBuffersToSubmit = { imageObject.commandBuffer };

    std::array<vk::SubmitInfo, 1> const submitInfo = {
        vk::SubmitInfo{
            boost::numeric_cast<uint32_t>(waitSemaphores.size()), waitSemaphores.data(), waitStages.data(),
            boost::numeric_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(),
            boost::numeric_cast<uint32_t>(signalSemaphores.size()), signalSemaphores.data()
        }
    };

    vulkanDevice->graphicsQueue.submit(submitInfo, vk::Fence());

    return imageObject.renderFinishedSemaphore;
}

void Pipeline_vulkan::buildCommandBuffers()
{
    let vulkanDevice = device();

    let commandBuffers = vulkanDevice->allocateCommandBuffers({
        vulkanDevice->graphicsCommandPool, 
        vk::CommandBufferLevel::ePrimary, 
        boost::numeric_cast<uint32_t>(frameBufferObjects.size())
    });

    for (size_t imageIndex = 0; imageIndex < frameBufferObjects.size(); imageIndex++) {
        auto &frameBufferObject = frameBufferObjects.at(imageIndex);
        frameBufferObject.commandBuffer = commandBuffers.at(imageIndex);
        frameBufferObject.commandBufferValid = false;
    }

    invalidateCommandBuffers();
}

void Pipeline_vulkan::teardownCommandBuffers()
{
    auto vulkanDevice = device();

    let commandBuffers = transform<vector<vk::CommandBuffer>>(frameBufferObjects, [](auto x) { return x.commandBuffer; });

    vulkanDevice->freeCommandBuffers(vulkanDevice->graphicsCommandPool, commandBuffers);
}

void Pipeline_vulkan::buildDescriptorSets()
{
    let vulkanDevice = device();

    let descriptorSetLayoutBindings = createDescriptorSetLayoutBindings();

    const vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        vk::DescriptorSetLayoutCreateFlags(),
        boost::numeric_cast<uint32_t>(descriptorSetLayoutBindings.size()), descriptorSetLayoutBindings.data()
    };

    descriptorSetLayout = vulkanDevice->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

    let descriptorPoolSizes = transform<std::vector<vk::DescriptorPoolSize>>(
        descriptorSetLayoutBindings,
        [this](auto x) -> vk::DescriptorPoolSize {
            return {
                x.descriptorType,
                boost::numeric_cast<uint32_t>(x.descriptorCount * this->frameBufferObjects.size())
            };
        }
    );
  
    descriptorPool = vulkanDevice->createDescriptorPool({
        vk::DescriptorPoolCreateFlags(),
        boost::numeric_cast<uint32_t>(frameBufferObjects.size()), // maxSets
        boost::numeric_cast<uint32_t>(descriptorPoolSizes.size()), descriptorPoolSizes.data()
    });

    std::vector<vk::DescriptorSetLayout> const descriptorSetLayouts(frameBufferObjects.size(), descriptorSetLayout);
    
    let descriptorSets = vulkanDevice->allocateDescriptorSets({
        descriptorPool,
        boost::numeric_cast<uint32_t>(descriptorSetLayouts.size()), descriptorSetLayouts.data()
    });

    for (size_t imageIndex = 0; imageIndex < frameBufferObjects.size(); imageIndex++) {
        auto &frameBufferObject = frameBufferObjects.at(imageIndex);
        frameBufferObject.descriptorSet = descriptorSets.at(imageIndex);
        frameBufferObject.descriptorSetVersion = 0;
    }
}

void Pipeline_vulkan::teardownDescriptorSets()
{
    let vulkanDevice = device();

    let descriptorSets = transform<vector<vk::DescriptorSet>>(frameBufferObjects, [](auto x) { return x.descriptorSet; });

    vulkanDevice->destroy(descriptorPool);
    vulkanDevice->destroy(descriptorSetLayout);
}

void Pipeline_vulkan::buildSemaphores()
{
    let vulkanDevice = device();

    let semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    for (auto &frameBufferObject: frameBufferObjects) {
        frameBufferObject.renderFinishedSemaphore = vulkanDevice->createSemaphore(semaphoreCreateInfo);
    }
}

void Pipeline_vulkan::teardownSemaphores()
{
    let vulkanDevice = device();

    for (let &frameBufferObject: frameBufferObjects) {
        vulkanDevice->destroy(frameBufferObject.renderFinishedSemaphore);
    }
}

void Pipeline_vulkan::buildPipeline(vk::RenderPass _renderPass, vk::Extent2D _extent)
{
    let vulkanDevice = device();

    LOG_INFO("buildPipeline previous size (%i, %i)") % extent.width % extent.height;

    renderPass = move(_renderPass);
    extent = move(_extent);
    scissor = {{ 0, 0 }, extent};

    const auto pushConstantRanges = createPushConstantRanges();
    const auto vertexInputBindingDescription = createVertexInputBindingDescription();
    const auto vertexInputAttributeDescriptions = createVertexInputAttributeDescriptions();
    const auto shaderStages = createShaderStages();

    const std::array<vk::DescriptorSetLayout, 1> descriptorSetLayouts = {descriptorSetLayout};

    pipelineLayout = vulkanDevice->createPipelineLayout({
        vk::PipelineLayoutCreateFlags(),
        boost::numeric_cast<uint32_t>(descriptorSetLayouts.size()), descriptorSetLayouts.data(),
        boost::numeric_cast<uint32_t>(pushConstantRanges.size()), pushConstantRanges.data()
    });

    const vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {
        vk::PipelineVertexInputStateCreateFlags(),
        1, &vertexInputBindingDescription,
        boost::numeric_cast<uint32_t>(vertexInputAttributeDescriptions.size()), vertexInputAttributeDescriptions.data()
    };

    const vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {
        vk::PipelineInputAssemblyStateCreateFlags(),
        vk::PrimitiveTopology::eTriangleList,
        VK_FALSE
    };

    const std::array<vk::Viewport, 1> viewports = {
        vk::Viewport{
            0.0f, 0.0f,
            boost::numeric_cast<float>(extent.width), boost::numeric_cast<float>(extent.height),
            0.0f, 1.0f
        }
    };

    const std::array<vk::Rect2D, 1> scissors = { scissor };

    const vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {
        vk::PipelineViewportStateCreateFlags(),
        boost::numeric_cast<uint32_t>(viewports.size()), viewports.data(),
        boost::numeric_cast<uint32_t>(scissors.size()), scissors.data()
    };

    const vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo =  {
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE, // depthClampEnable
        VK_FALSE, // rasterizerDiscardEnable
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eCounterClockwise,
        VK_FALSE, // depthBiasEnable
        0.0, // depthBiasConstantFactor
        0.0, // depthBiasClamp
        0.0, // depthBiasSlopeFactor
        1.0 // lineWidth
    };

    const vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1,
        VK_FALSE, // sampleShadingEnable
        1.0f, // minSampleShading
        nullptr, // sampleMask
        VK_FALSE, // alphaToCoverageEnable
        VK_FALSE // alphaToOneEnable
    };

    const std::vector<vk::PipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates = { {
        VK_TRUE, // blendEnable
        vk::BlendFactor::eOne, // srcColorBlendFactor
        vk::BlendFactor::eOneMinusSrcAlpha, // dstColorBlendFactor
        vk::BlendOp::eAdd, // colorBlendOp
        vk::BlendFactor::eOne, // srcAlphaBlendFactor
        vk::BlendFactor::eOneMinusSrcAlpha, // dstAlphaBlendFactor
        vk::BlendOp::eAdd, // aphaBlendOp
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    } };

    const vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo =  {
        vk::PipelineColorBlendStateCreateFlags(),
        VK_FALSE, // logicOpenable
        vk::LogicOp::eCopy,
        boost::numeric_cast<uint32_t>(pipelineColorBlendAttachmentStates.size()), pipelineColorBlendAttachmentStates.data()
    };

    const vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
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

    intrinsic = vulkanDevice->createGraphicsPipeline(vk::PipelineCache(), graphicsPipelineCreateInfo);
    LOG_INFO("/buildPipeline new size (%i, %i)") % extent.width % extent.height;
}

void Pipeline_vulkan::teardownPipeline()
{
    auto vulkanDevice = device();
    vulkanDevice->destroy(intrinsic);
    vulkanDevice->destroy(pipelineLayout);
}


void Pipeline_vulkan::buildForNewDevice()
{
}

void Pipeline_vulkan::buildForNewSurface()
{
}

void Pipeline_vulkan::buildForNewSwapchain(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers)
{
    if (nrFrameBuffers != frameBufferObjects.size()) {
        if (frameBufferObjects.size() > 0) {
            teardownSemaphores();
            teardownDescriptorSets();
            teardownCommandBuffers();
            teardownVertexBuffers();
        }

        frameBufferObjects.resize(nrFrameBuffers);
        buildVertexBuffers(nrFrameBuffers);
        buildCommandBuffers();
        buildDescriptorSets();
        buildSemaphores();
    }
    buildPipeline(renderPass, extent);
}

void Pipeline_vulkan::teardownForSwapchainLost()
{
    invalidateCommandBuffers();
    teardownPipeline();
}

void Pipeline_vulkan::teardownForSurfaceLost()
{
}

void Pipeline_vulkan::teardownForDeviceLost()
{
    teardownSemaphores();
    teardownDescriptorSets();
    teardownCommandBuffers();
    teardownVertexBuffers();
    frameBufferObjects.resize(0);
}

void Pipeline_vulkan::teardownForWindowLost()
{
}

void Pipeline_vulkan::invalidateCommandBuffers()
{
    for (auto &frameBufferObject: frameBufferObjects) {
        frameBufferObject.commandBufferValid = false;
        let commandBuffer = frameBufferObject.commandBuffer;
        commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    }
}

void Pipeline_vulkan::validateCommandBuffer(uint32_t imageIndex)
{
    auto &frameBufferObject = frameBufferObjects.at(imageIndex);

    if (frameBufferObject.commandBufferValid) {
        return;
    }

    LOG_INFO("validateCommandBuffer %i (%i, %i)") % imageIndex % extent.width % extent.height;

    auto commandBuffer = frameBufferObject.commandBuffer;

    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    auto _window = window.lock();

    let backgroundColor = color_cast<Color_sRGB>(_window->widget->backgroundColor).value;
    std::array<float, 4> _backgroundColor = { backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a };
    std::array<vk::ClearValue, 1> const clearColors = { { _backgroundColor } };

    commandBuffer.beginRenderPass({
            renderPass, 
            _window->swapchainFramebuffers.at(imageIndex),
            scissor, 
            boost::numeric_cast<uint32_t>(clearColors.size()),
            clearColors.data()
        }, vk::SubpassContents::eInline
    );

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, intrinsic);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, {frameBufferObject.descriptorSet}, {});

    drawInCommandBuffer(commandBuffer, imageIndex);

    commandBuffer.endRenderPass();

    commandBuffer.end();

    frameBufferObject.commandBufferValid = true;
}

}
