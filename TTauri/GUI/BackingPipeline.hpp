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
namespace GUI {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlasses and sharing for all views.
 */
class BackingPipeline : public Pipeline {
public:
    BackingPipeline(Window *window);
    virtual ~BackingPipeline();
    
protected:
    virtual std::vector<vk::ShaderModule> createShaderModules(void) const;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const;

};

}}
