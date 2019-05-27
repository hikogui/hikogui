// Copyright 2019 Pokitec
// All rights reserved.

#include "Pipeline_vulkan.hpp"
#include "Device_vulkan.hpp"
#include "Window_vulkan.hpp"
#include "TTauri/all.hpp"
#include <boost/assert.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri::GUI {

using namespace std;

Pipeline_vulkan::Pipeline_vulkan(const std::shared_ptr<Window> window) :
    Pipeline(std::move(window)) {}

Pipeline_vulkan::~Pipeline_vulkan()
{
}

vk::Semaphore Pipeline_vulkan::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    auto const vulkanDevice = device<Device_vulkan>();
    auto &imageObject = frameBufferObjects.at(imageIndex);

    if (imageObject.descriptorSetVersion < getDescriptorSetVersion()) {
        auto const writeDescriptorSets = createWriteDescriptorSet(imageIndex);
        vulkanDevice->updateDescriptorSets(writeDescriptorSets, {});

        imageObject.descriptorSetVersion = getDescriptorSetVersion();
    }

    validateCommandBuffer(imageIndex);

    vector<vk::Semaphore> const waitSemaphores = { inputSemaphore };
    vector<vk::PipelineStageFlags> const waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    BOOST_ASSERT(waitSemaphores.size() == waitStages.size());

    vector<vk::Semaphore> const signalSemaphores = { imageObject.renderFinishedSemaphore };
    vector<vk::CommandBuffer> const commandBuffersToSubmit = { imageObject.commandBuffer };

    vector<vk::SubmitInfo> const submitInfo = { {
            boost::numeric_cast<uint32_t>(waitSemaphores.size()), waitSemaphores.data(), waitStages.data(),
            boost::numeric_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(),
            boost::numeric_cast<uint32_t>(signalSemaphores.size()), signalSemaphores.data()
    } };

    vulkanDevice->graphicsQueue.submit(submitInfo, vk::Fence());

    return imageObject.renderFinishedSemaphore;
}

void Pipeline_vulkan::buildCommandBuffers()
{
    auto const vulkanDevice = device<Device_vulkan>();

    auto const commandBuffers = vulkanDevice->allocateCommandBuffers({
        vulkanDevice->graphicsCommandPool, 
        vk::CommandBufferLevel::ePrimary, 
        boost::numeric_cast<uint32_t>(frameBufferObjects.size())
    });

    for (size_t imageIndex = 0; imageIndex < frameBufferObjects.size(); imageIndex++) {
        auto &frameBufferObject = frameBufferObjects.at(imageIndex);
        frameBufferObject.commandBuffer = commandBuffers.at(imageIndex);
        frameBufferObject.commandBufferValid = false;
    }

    invalidateCommandBuffers(false);
}

void Pipeline_vulkan::teardownCommandBuffers()
{
    auto vulkanDevice = device<Device_vulkan>();

    auto const commandBuffers = transform<vector<vk::CommandBuffer>>(frameBufferObjects, [](auto x) { return x.commandBuffer; });

    vulkanDevice->freeCommandBuffers(vulkanDevice->graphicsCommandPool, commandBuffers);
}

void Pipeline_vulkan::buildDescriptorSets()
{
    auto const vulkanDevice = device<Device_vulkan>();

    auto const descriptorSetLayoutBindings = createDescriptorSetLayoutBindings();

    const vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        vk::DescriptorSetLayoutCreateFlags(),
        boost::numeric_cast<uint32_t>(descriptorSetLayoutBindings.size()), descriptorSetLayoutBindings.data()
    };

    descriptorSetLayout = vulkanDevice->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

    auto const descriptorPoolSizes = transform<std::vector<vk::DescriptorPoolSize>>(
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
    
    auto const descriptorSets = vulkanDevice->allocateDescriptorSets({
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
    auto const vulkanDevice = device<Device_vulkan>();

    auto const descriptorSets = transform<vector<vk::DescriptorSet>>(frameBufferObjects, [](auto x) { return x.descriptorSet; });

    vulkanDevice->destroy(descriptorPool);
    vulkanDevice->destroy(descriptorSetLayout);
}

void Pipeline_vulkan::buildSemaphores()
{
    auto const vulkanDevice = device<Device_vulkan>();

    auto const semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    for (auto &frameBufferObject: frameBufferObjects) {
        frameBufferObject.renderFinishedSemaphore = vulkanDevice->createSemaphore(semaphoreCreateInfo);
    }
}

void Pipeline_vulkan::teardownSemaphores()
{
    auto const vulkanDevice = device<Device_vulkan>();

    for (auto const &frameBufferObject: frameBufferObjects) {
        vulkanDevice->destroy(frameBufferObject.renderFinishedSemaphore);
    }
}

void Pipeline_vulkan::buildPipeline(vk::RenderPass _renderPass, vk::Extent2D _extent)
{
    auto const vulkanDevice = device<Device_vulkan>();

    LOG_INFO("buildPipeline (%i, %i)") % extent.width % extent.height;

    renderPass = move(_renderPass);
    extent = move(_extent);
    scissor = {{ 0, 0 }, extent};

    const auto pushConstantRanges = createPushConstantRanges();
    const auto vertexInputBindingDescription = createVertexInputBindingDescription();
    const auto vertexInputAttributeDescriptions = createVertexInputAttributeDescriptions();
    const auto shaderStages = createShaderStages();

    const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {descriptorSetLayout};

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

    const std::vector<vk::Viewport> viewports = { {
            0.0f, 0.0f,
            boost::numeric_cast<float>(extent.width), boost::numeric_cast<float>(extent.height),
            0.0f, 1.0f
    } };

    const std::vector<vk::Rect2D> scissors = { scissor };

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
        VK_FALSE, // blendEnable
        vk::BlendFactor::eOne, // srcColorBlendFactor
        vk::BlendFactor::eZero, // dstColorBlendFactor
        vk::BlendOp::eAdd, // colorBlendOp
        vk::BlendFactor::eOne, // srcAlphaBlendFactor
        vk::BlendFactor::eZero, // dstAlphaBlendFactor
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
    LOG_INFO("/buildPipeline (%i, %i)") % extent.width % extent.height;
}

void Pipeline_vulkan::teardownPipeline()
{
    auto vulkanDevice = device<Device_vulkan>();
    vulkanDevice->destroy(intrinsic);
    vulkanDevice->destroy(pipelineLayout);
}

void Pipeline_vulkan::buildForDeviceChange(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers)
{
    frameBufferObjects.resize(nrFrameBuffers);
    buildVertexBuffers(nrFrameBuffers);
    buildCommandBuffers();
    buildDescriptorSets();
    buildSemaphores();
    buildPipeline(renderPass, extent);
}

void Pipeline_vulkan::teardownForDeviceChange()
{
    invalidateCommandBuffers(true);
    teardownPipeline();
    teardownSemaphores();
    teardownDescriptorSets();
    teardownCommandBuffers();
    teardownVertexBuffers();
    frameBufferObjects.resize(0);
}

void Pipeline_vulkan::buildForSwapchainChange(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers)
{
    if (nrFrameBuffers != frameBufferObjects.size()) {
        teardownSemaphores();
        teardownDescriptorSets();
        teardownCommandBuffers();
        teardownVertexBuffers();

        frameBufferObjects.resize(nrFrameBuffers);
        buildVertexBuffers(nrFrameBuffers);
        buildCommandBuffers();
        buildDescriptorSets();
        buildSemaphores();
    }
    buildPipeline(renderPass, extent);
}

void Pipeline_vulkan::teardownForSwapchainChange()
{
    invalidateCommandBuffers(true);
    teardownPipeline();
}

void Pipeline_vulkan::invalidateCommandBuffers(bool reset)
{
    for (auto &frameBufferObject: frameBufferObjects) {
        frameBufferObject.commandBufferValid = false;
        if (reset) {
            auto const commandBuffer = frameBufferObject.commandBuffer;
            commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        }
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

    commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    std::array<float, 4> const blackColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    vector<vk::ClearValue> const clearColors = {{blackColor}};

    auto vulkanWindow = lock_dynamic_cast<Window_vulkan>(window);

    commandBuffer.beginRenderPass({
            renderPass, 
            vulkanWindow->swapchainFramebuffers.at(imageIndex), 
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
