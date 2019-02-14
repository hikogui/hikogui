//
//  BackingPipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "Pipeline.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class BackingPipeline : public Pipeline {
public:
    BackingPipeline(Device *device);
    virtual ~BackingPipeline();

protected:
    virtual std::vector<vk::ShaderModule> createShaderModules(void) const;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const;

};

}}}
