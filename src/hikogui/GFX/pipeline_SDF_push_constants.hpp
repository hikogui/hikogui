// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../rapid/sfloat_rg32.hpp"
#include <vulkan/vulkan.hpp>

namespace hi::inline v1::pipeline_SDF {

struct push_constants {
    sfloat_rg32 window_extent = extent2{0.0, 0.0};
    sfloat_rg32 viewport_scale = scale2{0.0, 0.0};
    sfloat_rg32 red_subpixel_offset = vector2{0.0, 0.0};
    sfloat_rg32 blue_subpixel_offset = vector2{0.0, 0.0};
    VkBool32 has_subpixels = false;

    static std::vector<vk::PushConstantRange> pushConstantRanges()
    {
        return {{vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(push_constants)}};
    }
};

} // namespace hi::inline v1::pipeline_SDF
