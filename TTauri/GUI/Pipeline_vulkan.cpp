// Copyright 2019 Pokitec
// All rights reserved.

#include "Pipeline_vulkan.hpp"
#include "Device.hpp"
#include "Window.hpp"

namespace TTauri::GUI {

using namespace std;

Pipeline_vulkan::Pipeline_vulkan(Window const &window) :
    Pipeline_base(window) {}

Pipeline_vulkan::~Pipeline_vulkan()
{
}

vk::Semaphore Pipeline_vulkan::render(uint32_t frameBufferIndex, vk::Semaphore inputSemaphore)
{
    auto &imageObject = frameBufferObjects.at(frameBufferIndex);

    if (imageObject.descriptorSetVersion < getDescriptorSetVersion()) {
        let writeDescriptorSets = createWriteDescriptorSet(frameBufferIndex);
        device().updateDescriptorSets(writeDescriptorSets, {});

        imageObject.descriptorSetVersion = getDescriptorSetVersion();
    }

    validateCommandBuffer(frameBufferIndex);

    std::array<vk::Semaphore, 1> const waitSemaphores = { inputSemaphore };
    std::array<vk::PipelineStageFlags, 1> const waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    BOOST_ASSERT(waitSemaphores.size() == waitStages.size());

    std::array<vk::Semaphore, 1> const signalSemaphores = { imageObject.renderFinishedSemaphore };
    std::array<vk::CommandBuffer, 1> const commandBuffersToSubmit = { imageObject.commandBuffer };

    std::array<vk::SubmitInfo, 1> const submitInfo = {
        vk::SubmitInfo{
            numeric_cast<uint32_t>(waitSemaphores.size()), waitSemaphores.data(), waitStages.data(),
            numeric_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(),
            numeric_cast<uint32_t>(signalSemaphores.size()), signalSemaphores.data()
        }
    };

    device().graphicsQueue.submit(submitInfo, vk::Fence());

    return imageObject.renderFinishedSemaphore;
}

void Pipeline_vulkan::buildCommandBuffers()
{
    let commandBuffers = device().allocateCommandBuffers({
        device().graphicsCommandPool, 
        vk::CommandBufferLevel::ePrimary, 
        numeric_cast<uint32_t>(frameBufferObjects.size())
    });

    for (int i = 0; i < to_int(frameBufferObjects.size()); i++) {
        auto &frameBufferObject = frameBufferObjects.at(i);
        frameBufferObject.commandBuffer = commandBuffers.at(i);
        frameBufferObject.commandBufferValid = false;
    }

    invalidateCommandBuffers();
}

void Pipeline_vulkan::teardownCommandBuffers()
{
    let commandBuffers = transform<vector<vk::CommandBuffer>>(frameBufferObjects, [](auto x) { return x.commandBuffer; });

    device().freeCommandBuffers(device().graphicsCommandPool, commandBuffers);
}

void Pipeline_vulkan::buildDescriptorSets()
{
    let descriptorSetLayoutBindings = createDescriptorSetLayoutBindings();

    hasDescriptorSets = descriptorSetLayoutBindings.size() > 0;
    if (!hasDescriptorSets) {
        return;
    }

    const vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        vk::DescriptorSetLayoutCreateFlags(),
        numeric_cast<uint32_t>(descriptorSetLayoutBindings.size()), descriptorSetLayoutBindings.data()
    };

    descriptorSetLayout = device().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

    let descriptorPoolSizes = transform<std::vector<vk::DescriptorPoolSize>>(
        descriptorSetLayoutBindings,
        [this](auto x) -> vk::DescriptorPoolSize {
            return {
                x.descriptorType,
                numeric_cast<uint32_t>(x.descriptorCount * this->frameBufferObjects.size())
            };
        }
    );
  
    descriptorPool = device().createDescriptorPool({
        vk::DescriptorPoolCreateFlags(),
        numeric_cast<uint32_t>(frameBufferObjects.size()), // maxSets
        numeric_cast<uint32_t>(descriptorPoolSizes.size()), descriptorPoolSizes.data()
    });

    std::vector<vk::DescriptorSetLayout> const descriptorSetLayouts(frameBufferObjects.size(), descriptorSetLayout);
    
    let descriptorSets = device().allocateDescriptorSets({
        descriptorPool,
        numeric_cast<uint32_t>(descriptorSetLayouts.size()), descriptorSetLayouts.data()
    });

    for (int i = 0; i < frameBufferObjects.size(); i++) {
        auto &frameBufferObject = frameBufferObjects.at(i);
        frameBufferObject.descriptorSet = descriptorSets.at(i);
        frameBufferObject.descriptorSetVersion = 0;
    }
}

void Pipeline_vulkan::teardownDescriptorSets()
{
    if (!hasDescriptorSets) {
        return;
    }

    device().destroy(descriptorPool);
    device().destroy(descriptorSetLayout);
}

void Pipeline_vulkan::buildSemaphores()
{
    let semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    for (auto &frameBufferObject: frameBufferObjects) {
        frameBufferObject.renderFinishedSemaphore = device().createSemaphore(semaphoreCreateInfo);
    }
}

void Pipeline_vulkan::teardownSemaphores()
{
    for (let &frameBufferObject: frameBufferObjects) {
        device().destroy(frameBufferObject.renderFinishedSemaphore);
    }
}

void Pipeline_vulkan::buildPipeline(vk::RenderPass _renderPass, vk::Extent2D _extent)
{
    LOG_INFO("buildPipeline previous size ({0}, {1})", extent.width, extent.height);

    renderPass = move(_renderPass);
    extent = move(_extent);
    scissor = {{ 0, 0 }, extent};

    const auto pushConstantRanges = createPushConstantRanges();
    const auto vertexInputBindingDescription = createVertexInputBindingDescription();
    const auto vertexInputAttributeDescriptions = createVertexInputAttributeDescriptions();
    const auto shaderStages = createShaderStages();

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    if (hasDescriptorSets) {
        descriptorSetLayouts.push_back(descriptorSetLayout);
    }

    pipelineLayout = device().createPipelineLayout({
        vk::PipelineLayoutCreateFlags(),
        numeric_cast<uint32_t>(descriptorSetLayouts.size()), descriptorSetLayouts.data(),
        numeric_cast<uint32_t>(pushConstantRanges.size()), pushConstantRanges.data()
    });

    const vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {
        vk::PipelineVertexInputStateCreateFlags(),
        1, &vertexInputBindingDescription,
        numeric_cast<uint32_t>(vertexInputAttributeDescriptions.size()), vertexInputAttributeDescriptions.data()
    };

    const vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {
        vk::PipelineInputAssemblyStateCreateFlags(),
        vk::PrimitiveTopology::eTriangleList,
        VK_FALSE
    };

    const std::array<vk::Viewport, 1> viewports = {
        vk::Viewport{
            0.0f, 0.0f,
            numeric_cast<float>(extent.width), numeric_cast<float>(extent.height),
            0.0f, 1.0f
        }
    };

    const std::array<vk::Rect2D, 1> scissors = { scissor };

    const vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {
        vk::PipelineViewportStateCreateFlags(),
        numeric_cast<uint32_t>(viewports.size()), viewports.data(),
        numeric_cast<uint32_t>(scissors.size()), scissors.data()
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
        vk::BlendFactor::eSrcAlpha, // srcColorBlendFactor
        vk::BlendFactor::eOneMinusSrcAlpha, // dstColorBlendFactor
        vk::BlendOp::eAdd, // colorBlendOp
        vk::BlendFactor::eOne, // srcAlphaBlendFactor
        vk::BlendFactor::eOne, // dstAlphaBlendFactor
        vk::BlendOp::eAdd, // aphaBlendOp
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    } };

    const vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo =  {
        vk::PipelineColorBlendStateCreateFlags(),
        VK_FALSE, // logicOpenable
        vk::LogicOp::eCopy,
        numeric_cast<uint32_t>(pipelineColorBlendAttachmentStates.size()), pipelineColorBlendAttachmentStates.data()
    };

    const vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
        vk::PipelineCreateFlags(),
        numeric_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
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

    intrinsic = device().createGraphicsPipeline(vk::PipelineCache(), graphicsPipelineCreateInfo);
    LOG_INFO("/buildPipeline new size ({0}, {1})", extent.width, extent.height);
}

void Pipeline_vulkan::teardownPipeline()
{
    device().destroy(intrinsic);
    device().destroy(pipelineLayout);
}


void Pipeline_vulkan::buildForNewDevice()
{
}

void Pipeline_vulkan::buildForNewSurface()
{
}

void Pipeline_vulkan::buildForNewSwapchain(vk::RenderPass renderPass, vk::Extent2D extent, int nrFrameBuffers)
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

void Pipeline_vulkan::validateCommandBuffer(uint32_t frameBufferIndex)
{
    auto &frameBufferObject = frameBufferObjects.at(frameBufferIndex);

    if (frameBufferObject.commandBufferValid) {
        return;
    }

    LOG_INFO("validateCommandBuffer {0} ({1}, {2})", frameBufferIndex, extent.width, extent.height);

    auto commandBuffer = frameBufferObject.commandBuffer;

    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    let backgroundColor = wsRGBA(window.widget->backgroundColor).to_Linear_sRGBA_vec4();
    std::array<float, 4> _backgroundColor = { backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a };
    std::array<vk::ClearValue, 1> const clearColors = { { _backgroundColor } };

    commandBuffer.beginRenderPass({
            renderPass, 
            window.swapchainFramebuffers.at(frameBufferIndex),
            scissor, 
            numeric_cast<uint32_t>(clearColors.size()),
            clearColors.data()
        }, vk::SubpassContents::eInline
    );

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, intrinsic);

    if (hasDescriptorSets) {
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, {frameBufferObject.descriptorSet}, {});
    }

    drawInCommandBuffer(commandBuffer, frameBufferIndex);

    commandBuffer.endRenderPass();

    commandBuffer.end();

    frameBufferObject.commandBufferValid = true;
}

}
