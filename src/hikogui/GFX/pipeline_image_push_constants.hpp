// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../image/module.hpp"
#include <vulkan/vulkan.hpp>

namespace hi::inline v1::pipeline_image {

struct push_constants {
    sfloat_rg32 windowExtent = extent2{0.0, 0.0};
    sfloat_rg32 viewportScale = scale2{0.0, 0.0};
    sfloat_rg32 atlasExtent = extent2{0.0, 0.0};
    sfloat_rg32 atlasScale = scale2{0.0, 0.0};

    static std::vector<vk::PushConstantRange> pushConstantRanges()
    {
        return {{vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(push_constants)}};
    }
};

} // namespace hi::inline v1::pipeline_image
