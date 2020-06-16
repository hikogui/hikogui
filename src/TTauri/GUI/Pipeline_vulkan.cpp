// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Pipeline_vulkan.hpp"
#include "TTauri/GUI/GUIDevice.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/Foundation/trace.hpp"
#include <array>
#include <vector>

namespace tt {

using namespace std;

Pipeline_vulkan::Pipeline_vulkan(Window const &window) :
    Pipeline_base(window) {}

Pipeline_vulkan::~Pipeline_vulkan()
{
}

void Pipeline_vulkan::drawInCommandBuffer(vk::CommandBuffer commandBuffer)
{
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, intrinsic);

    if (descriptorSet) {
        if (descriptorSetVersion < getDescriptorSetVersion()) {
            descriptorSetVersion = getDescriptorSetVersion();

            device().updateDescriptorSets(createWriteDescriptorSet(), {});
        }

        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            pipelineLayout, 0, {descriptorSet}, {}
        );
    }
}


void Pipeline_vulkan::buildDescriptorSets()
{
    ttlet descriptorSetLayoutBindings = createDescriptorSetLayoutBindings();

    if (ssize(descriptorSetLayoutBindings) == 0) {
        // Make sure that there is no descriptor set.
        descriptorSet = nullptr;
        return;
    }

    ttlet descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo{
        vk::DescriptorSetLayoutCreateFlags(),
        numeric_cast<uint32_t>(descriptorSetLayoutBindings.size()), descriptorSetLayoutBindings.data()
    };

    descriptorSetLayout = device().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

    ttlet descriptorPoolSizes = transform<std::vector<vk::DescriptorPoolSize>>(
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

    ttlet descriptorSetLayouts = std::array{
        descriptorSetLayout
    };
    
    ttlet descriptorSets = device().allocateDescriptorSets({
        descriptorPool,
        numeric_cast<uint32_t>(descriptorSetLayouts.size()), descriptorSetLayouts.data()
    });

    descriptorSet = descriptorSets.at(0);
    descriptorSetVersion = 0;
}

void Pipeline_vulkan::teardownDescriptorSets()
{
    if (!descriptorSet) {
        return;
    }

    device().destroy(descriptorPool);
    device().destroy(descriptorSetLayout);
    descriptorSet = nullptr;
}

vk::PipelineDepthStencilStateCreateInfo Pipeline_vulkan::getPipelineDepthStencilStateCreateInfo() const
{
    // Reverse-z depth configuration
    return {
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
}

/* Pre-multiplied alpha blending.
*/
std::vector<vk::PipelineColorBlendAttachmentState> Pipeline_vulkan::getPipelineColorBlendAttachmentStates() const
{
    return { {
        VK_TRUE, // blendEnable
        vk::BlendFactor::eOne, // srcColorBlendFactor
        vk::BlendFactor::eOneMinusSrcAlpha, // dstColorBlendFactor
        vk::BlendOp::eAdd, // colorBlendOp
        vk::BlendFactor::eOne, // srcAlphaBlendFactor
        vk::BlendFactor::eZero, // dstAlphaBlendFactor
        vk::BlendOp::eAdd, // aphaBlendOp
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    } };
}

void Pipeline_vulkan::buildPipeline(vk::RenderPass renderPass, uint32_t renderSubpass, vk::Extent2D _extent)
{
    LOG_INFO("buildPipeline previous size ({}, {})", extent.width, extent.height);
    extent = _extent;

    const auto pushConstantRanges = createPushConstantRanges();
    const auto vertexInputBindingDescription = createVertexInputBindingDescription();
    const auto vertexInputAttributeDescriptions = createVertexInputAttributeDescriptions();
    const auto shaderStages = createShaderStages();

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    if (descriptorSet) {
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

    ttlet scissor = vk::Rect2D{vk::Offset2D{ 0, 0 }, extent};

    ttlet scissors = std::array{ scissor };

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

    ttlet pipelineDepthStencilStateCreateInfo = getPipelineDepthStencilStateCreateInfo();
    
   
    /* Pre-multiplied alpha blending.
     */
    ttlet pipelineColorBlendAttachmentStates = getPipelineColorBlendAttachmentStates();
    
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


void Pipeline_vulkan::buildForNewDevice(GUIDevice *device)
{
    tt_assert(device != nullptr);
    _device = device;
}

void Pipeline_vulkan::buildForNewSurface()
{
}

void Pipeline_vulkan::buildForNewSwapchain(vk::RenderPass renderPass, uint32_t renderSubpass, vk::Extent2D _extent)
{
    if (!buffersInitialized) {
        buildVertexBuffers();
        buffersInitialized = true;
    }
    // Input attachments described by the descriptor set will change when a
    // new swap chain is created.
    buildDescriptorSets();
    buildPipeline(renderPass, renderSubpass, _extent);
}

void Pipeline_vulkan::teardownForSwapchainLost()
{
    teardownPipeline();
    teardownDescriptorSets();
}

void Pipeline_vulkan::teardownForSurfaceLost()
{
}

void Pipeline_vulkan::teardownForDeviceLost()
{
    teardownVertexBuffers();
    buffersInitialized = false;
    _device = nullptr;
}

void Pipeline_vulkan::teardownForWindowLost()
{
}

}
