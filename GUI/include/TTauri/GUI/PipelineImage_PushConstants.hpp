// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/R32G32SFloat.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI::PipelineImage {

struct PushConstants {
    R32G32SFloat windowExtent = { 0.0, 0.0 };
    R32G32SFloat viewportScale = { 0.0, 0.0 };
    R32G32SFloat atlasExtent = { 0.0, 0.0 };
    R32G32SFloat atlasScale = { 0.0, 0.0 };

    static std::vector<vk::PushConstantRange> pushConstantRanges()
    {
        return {
            { vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants) }
        };
    }
};

}
