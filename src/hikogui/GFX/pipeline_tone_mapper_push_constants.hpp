// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../rapid/sfloat_rg32.hpp"
#include <vulkan/vulkan.hpp>

namespace tt::inline v1::pipeline_tone_mapper {

struct push_constants {
    float saturation = 1.0;

    static std::vector<vk::PushConstantRange> pushConstantRanges()
    {
        return {{vk::ShaderStageFlagBits::eFragment, 0, sizeof(push_constants)}};
    }
};

} // namespace tt::inline v1::pipeline_tone_mapper
