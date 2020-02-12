// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <array>

namespace TTauri::GUI::PipelineMSDF {

struct FragmentSpecializationConstants {
    float rangeMultiplier = 0.0f;

    static std::array<vk::SpecializationMapEntry,1> specializationEntries() noexcept {
        return {
            vk::SpecializationMapEntry{0, offsetof(FragmentSpecializationConstants, rangeMultiplier), sizeof(rangeMultiplier)}
        };
    }

    vk::SpecializationInfo specializationInfo(std::array<vk::SpecializationMapEntry,1> const &entries) const noexcept {
        return { numeric_cast<uint32_t>(entries.size()), entries.data(), sizeof(FragmentSpecializationConstants), this };
    }
};

}
