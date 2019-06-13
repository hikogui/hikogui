// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace TTauri::GUI::PipelineImage {

struct PushConstants {
    glm::vec2 windowExtent = { 0.0, 0.0 };
    glm::vec2 viewportScale = { 0.0, 0.0 };
    glm::vec2 atlasExtent = { 0.0, 0.0 };
    glm::vec2 atlasScale = { 0.0, 0.0 };

    static std::vector<vk::PushConstantRange> pushConstantRanges()
    {
        return {
            { vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants) }
        };
    }
};

}
