//
//  Pipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Pipeline.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcomma"
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#pragma clang diagnostic pop

#include "Window.hpp"
#include "Device.hpp"
#include "TTauri/Toolkit/Logging.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

vk::ShaderModule Pipeline::loadShader(boost::filesystem::path path)
{
    LOG_INFO("Loading shader %s") % path.filename().generic_string();

    auto mapped_file = boost::interprocess::file_mapping(path.c_str(), boost::interprocess::read_only);
    auto region = boost::interprocess::mapped_region(mapped_file, boost::interprocess::read_only);

    auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo(
        vk::ShaderModuleCreateFlags(),
        region.get_size(),
        reinterpret_cast<uint32_t *>(region.get_address())
    );

    return device->intrinsic.createShaderModule(shaderModuleCreateInfo);
}

Pipeline::Pipeline(Window *window, boost::filesystem::path vertexShaderPath, boost::filesystem::path fragmentShaderPath) :
    window(window), device(window->device), vertexShaderPath(vertexShaderPath), fragmentShaderPath(fragmentShaderPath)
{
    auto vertexShader = loadShader(vertexShaderPath);
    auto fragmentShader = loadShader(fragmentShaderPath);

    auto vertexShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo(
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eVertex,
        vertexShader,
        "main"
    );

    auto fragmentShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo(
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eFragment,
        fragmentShader,
        "main"
    );

    auto shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>{
        vertexShaderStageCreateInfo,
        fragmentShaderStageCreateInfo
    };

    auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo();
    auto inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo(
        vk::PipelineInputAssemblyStateCreateFlags(),
        vk::PrimitiveTopology::eTriangleList,
        VK_FALSE
    );

    auto viewPort = vk::Viewport(
        0.0f,
        0.0f,
        boost::numeric_cast<float>(window->swapchainCreateInfo.imageExtent.width),
        boost::numeric_cast<float>(window->swapchainCreateInfo.imageExtent.height),
        0.0f,
        1.0f
    );

    auto scissor = vk::Rect2D{
        {0, 0},
        window->swapchainCreateInfo.imageExtent
    };

    auto pipelineViewportStateCreateInfo = vk::PipelineViewportStateCreateInfo(
        vk::PipelineViewportStateCreateFlags(),
        1,
        &viewPort,
        1,
        &scissor
    );

    auto pipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo(
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE, // depthClampEnable
        VK_FALSE, // rasterizerDiscardEnable
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        VK_FALSE // depthBiasEnable
    );

    auto pipelineMultisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo(
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1,
        VK_FALSE, // sampleShadingEnable
        1.0f, // minSampleShading
        nullptr, // sampleMask
        VK_FALSE, // alphaToCoverageEnable
        VK_FALSE // alphaToOneEnable
    );

    auto pipelineColorBlendAttachmentState = vk::PipelineColorBlendAttachmentState(
        VK_FALSE, // blendEnable
        vk::BlendFactor::eOne, // srcColorBlendFactor
        vk::BlendFactor::eZero, // dstColorBlendFactor
        vk::BlendOp::eAdd, // colorBlendOp
        vk::BlendFactor::eOne, // srcAlphaBlendFactor
        vk::BlendFactor::eZero, // dstAlphaBlendFactor
        vk::BlendOp::eAdd, // aphaBlendOp
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );

    auto pipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo(
        vk::PipelineColorBlendStateCreateFlags(),
        VK_FALSE, // logicOpenable
        vk::LogicOp::eCopy,
        1, &pipelineColorBlendAttachmentState
    );

    auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo();
    auto pipelineLayout = device->intrinsic.createPipelineLayout(pipelineLayoutCreateInfo);

    auto colorAttachement = vk::AttachmentDescription(
        vk::AttachmentDescriptionFlags(),
        window->swapchainCreateInfo.imageFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
        vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
        vk::ImageLayout::eUndefined, // initialLayout
        vk::ImageLayout::ePresentSrcKHR // finalLayout
    );

    auto attachmentReference = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);

    auto subpass = vk::SubpassDescription(
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        0, nullptr, // inputAttachments
        1, &attachmentReference // colorAttachments
    );

    auto renderPassCreateInfo = vk::RenderPassCreateInfo(
        vk::RenderPassCreateFlags(),
        1, &colorAttachement,
        1, &subpass
    );

    auto renderPass = device->intrinsic.createRenderPass(renderPassCreateInfo);

    auto graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo(
        vk::PipelineCreateFlags(),
        boost::numeric_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
        &vertexInputStateCreateInfo,
        &inputAssemblyStateCreateInfo,
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
    );

    auto graphicsPipeline = device->intrinsic.createGraphicsPipeline(vk::PipelineCache(), graphicsPipelineCreateInfo);

    device->intrinsic.destroy(graphicsPipeline);
    device->intrinsic.destroy(renderPass);
    device->intrinsic.destroy(pipelineLayout);
    device->intrinsic.destroy(vertexShader);
    device->intrinsic.destroy(fragmentShader);
}

Pipeline::~Pipeline()
{
}

}}}
