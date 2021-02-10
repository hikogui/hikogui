// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../R32G32SFloat.hpp"
#include <vulkan/vulkan.hpp>

namespace tt::pipeline_SDF {

struct push_constants {
    R32G32SFloat windowExtent = f32x4{ 0.0, 0.0 };
    R32G32SFloat viewportScale = f32x4{ 0.0, 0.0 };
    int32_t subpixel_orientation = 0;

    static std::vector<vk::PushConstantRange> pushConstantRanges()
    {
        return {
            { vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(push_constants) }
        };
    }
};

}
