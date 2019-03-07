//
//  BackingPipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "BackingPipeline.hpp"
#include "TTauri/Toolkit/Application.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

using namespace TTauri::Toolkit;

BackingPipeline::BackingPipeline(Window *window) :
    Pipeline(window)
{
}

BackingPipeline::~BackingPipeline()
{
}

std::vector<vk::ShaderModule> BackingPipeline::createShaderModules(void) const
{
    return {
        loadShader(app->getPathToResource("BackingPipeline.vert.spv")),
        loadShader(app->getPathToResource("BackingPipeline.frag.spv"))
    };
}

std::vector<vk::PipelineShaderStageCreateInfo> BackingPipeline::createShaderStages(const std::vector<vk::ShaderModule> &shaders) const
{
    return {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, shaders[0], "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, shaders[1], "main"}
    };
}


}}}
