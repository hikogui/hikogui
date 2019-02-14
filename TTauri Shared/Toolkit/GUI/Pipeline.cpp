//
//  Pipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Pipeline.hpp"

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "Device.hpp"
#include "TTauri/Toolkit/Logging.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

vk::ShaderModule Pipeline::loadShader(boost::filesystem::path path) const
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

Pipeline::Pipeline(Device *device) :
    device(device)
{

}

Pipeline::~Pipeline()
{
    device->intrinsic.destroy(intrinsic);
    device->intrinsic.destroy(pipelineLayout);
    for (auto shaderModule: shaderModules) {
        device->intrinsic.destroy(shaderModule);
    }
}

void Pipeline::initialize(vk::RenderPass renderPass, vk::Extent2D extent, vk::Format format)
{
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
}

}}}
