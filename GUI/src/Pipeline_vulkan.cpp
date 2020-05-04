// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Pipeline_vulkan.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/Foundation/trace.hpp"
#include <array>
#include <vector>

namespace TTauri::GUI {

using namespace std;

Pipeline_vulkan::Pipeline_vulkan(Window const &window) :
    Pipeline_base(window) {}

Pipeline_vulkan::~Pipeline_vulkan()
{
}

void Pipeline_vulkan::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    if (descriptorSetVersion < getDescriptorSetVersion()) {
        let writeDescriptorSets = createWriteDescriptorSet();
        device().updateDescriptorSets(writeDescriptorSets, {});

        descriptorSetVersion = getDescriptorSetVersion();
    }

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, intrinsic);

    if (hasDescriptorSets) {
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, {descriptorSet}, {});
    }
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
        [](auto x) -> vk::DescriptorPoolSize {
            return {
                x.descriptorType,
                numeric_cast<uint32_t>(x.descriptorCount)
            };
        }
    );
  
    descriptorPool = device().createDescriptorPool({
        vk::DescriptorPoolCreateFlags(),
        1, // maxSets
        numeric_cast<uint32_t>(descriptorPoolSizes.size()), descriptorPoolSizes.data()
    });

    std::vector<vk::DescriptorSetLayout> const descriptorSetLayouts(1, descriptorSetLayout);
    
    let descriptorSets = device().allocateDescriptorSets({
        descriptorPool,
        numeric_cast<uint32_t>(descriptorSetLayouts.size()), descriptorSetLayouts.data()
    });

    descriptorSet = descriptorSets.at(0);
    descriptorSetVersion = 0;
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
    renderFinishedSemaphore = device().createSemaphore(semaphoreCreateInfo);
}

void Pipeline_vulkan::teardownSemaphores()
{
    device().destroy(renderFinishedSemaphore);
}

void Pipeline_vulkan::buildPipeline(vk::RenderPass renderPass, uint32_t renderSubpass, vk::Extent2D extent)
{
    LOG_INFO("buildPipeline previous size ({}, {})", this->extent.width, this->extent.height);
    this->extent = extent;

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
            // Reverse-z, with float buffer this will give a linear depth buffer.
            1.0f, 0.0f
        }
    };

    let scissor = vk::Rect2D{vk::Offset2D{ 0, 0 }, extent};

    let scissors = std::array{ scissor };

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

    // Reverse-z depth configuration
    const vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {
        vk::PipelineDepthStencilStateCreateFlags(),
        VK_TRUE, // depthTestEnable;
        VK_TRUE, // depthWriteEnable;
        vk::CompareOp::eGreaterOrEqual, // depthCompareOp
        VK_FALSE, // depthBoundsTestEnable
        VK_FALSE, // stencilTestEnable,
        vk::StencilOpState(), // front
        vk::StencilOpState(), // back
        1.0f, // minDepthBounds
        0.0f, // maxDepthBounds
    };
   
    /* Pre-multiplied alpha blending.
     */
    const std::vector<vk::PipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates = { {
        VK_TRUE, // blendEnable
        vk::BlendFactor::eOne, // srcColorBlendFactor
        vk::BlendFactor::eOneMinusSrcAlpha, // dstColorBlendFactor
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
        &pipelineDepthStencilStateCreateInfo,
        &pipelineColorBlendStateCreateInfo,
        nullptr, // pipelineDynamicsStateCreateInfo
        pipelineLayout,
        renderPass,
        renderSubpass, // subpass
        vk::Pipeline(), // basePipelineHandle
        -1 // basePipelineIndex
    };

    intrinsic = device().createGraphicsPipeline(vk::PipelineCache(), graphicsPipelineCreateInfo);
    LOG_INFO("/buildPipeline new size ({}, {})", extent.width, extent.height);
}

void Pipeline_vulkan::teardownPipeline()
{
    device().destroy(intrinsic);
    device().destroy(pipelineLayout);
}


void Pipeline_vulkan::buildForNewDevice(Device *device)
{
    ttauri_assert(device != nullptr);
    _device = device;
}

void Pipeline_vulkan::buildForNewSurface()
{
}

void Pipeline_vulkan::buildForNewSwapchain(vk::RenderPass renderPass, uint32_t renderSubpass, vk::Extent2D extent)
{
    if (!buffersInitialized) {
        buildVertexBuffers();
        buildDescriptorSets();
        buildSemaphores();
        buffersInitialized = true;
    }
    buildPipeline(renderPass, renderSubpass, extent);
}

void Pipeline_vulkan::teardownForSwapchainLost()
{
    teardownPipeline();
}

void Pipeline_vulkan::teardownForSurfaceLost()
{
}

void Pipeline_vulkan::teardownForDeviceLost()
{
    teardownSemaphores();
    teardownDescriptorSets();
    teardownVertexBuffers();
    buffersInitialized = false;
    _device = nullptr;
}

void Pipeline_vulkan::teardownForWindowLost()
{
}

}
