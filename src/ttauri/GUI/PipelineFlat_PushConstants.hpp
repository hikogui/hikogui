// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../R32G32SFloat.hpp"
#include <vulkan/vulkan.hpp>

namespace tt::PipelineFlat {

struct PushConstants {
    R32G32SFloat windowExtent = f32x4{ 0.0, 0.0 };
    R32G32SFloat viewportScale = f32x4{ 0.0, 0.0 };

    static std::vector<vk::PushConstantRange> pushConstantRanges()
    {
        return {
            { vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants) }
        };
    }
};

}
