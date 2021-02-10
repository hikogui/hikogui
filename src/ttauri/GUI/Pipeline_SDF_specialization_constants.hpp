// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../R32G32SFloat.hpp"
#include <vulkan/vulkan.hpp>

namespace tt::pipeline_SDF {

struct specialization_constants {
    float SDF8maxDistance;
    float atlasImageWidth;

    [[nodiscard]] vk::SpecializationInfo specializationInfo(std::vector<vk::SpecializationMapEntry> &entries) const noexcept {
        return {
            narrow_cast<uint32_t>(std::ssize(entries)), entries.data(),
            sizeof (specialization_constants),
            this
        };
    }

    [[nodiscard]] static std::vector<vk::SpecializationMapEntry> specializationConstantMapEntries() noexcept {
        return {
            {0, offsetof(specialization_constants, SDF8maxDistance), sizeof(SDF8maxDistance)},
            {1, offsetof(specialization_constants, atlasImageWidth), sizeof(atlasImageWidth)},
        };
    }
};

}
