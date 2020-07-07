// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/R32G32SFloat.hpp"
#include <vulkan/vulkan.hpp>

namespace tt::PipelineSDF {

struct SpecializationConstants {
    float SDF8maxDistance;
    float atlasImageWidth;

    [[nodiscard]] vk::SpecializationInfo specializationInfo(std::vector<vk::SpecializationMapEntry> &entries) const noexcept {
        return {
            numeric_cast<uint32_t>(ssize(entries)), entries.data(),
            sizeof (SpecializationConstants),
            this
        };
    }

    [[nodiscard]] static std::vector<vk::SpecializationMapEntry> specializationConstantMapEntries() noexcept {
        return {
            {0, offsetof(SpecializationConstants, SDF8maxDistance), sizeof(SDF8maxDistance)},
            {1, offsetof(SpecializationConstants, atlasImageWidth), sizeof(atlasImageWidth)},
        };
    }
};

}
