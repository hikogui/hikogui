// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/StaticResources.hpp"

namespace TTauri {

#include "GUI/shaders/PipelineImage.vert.spv.inl"
#include "GUI/shaders/PipelineImage.frag.spv.inl"
#include "GUI/shaders/PipelineFlat.vert.spv.inl"
#include "GUI/shaders/PipelineFlat.frag.spv.inl"

StaticResources::StaticResources() noexcept
{
    intrinsic.try_emplace(PipelineImage_vert_spv_filename, PipelineImage_vert_spv_bytes);
    intrinsic.try_emplace(PipelineImage_frag_spv_filename, PipelineImage_frag_spv_bytes);
    intrinsic.try_emplace(PipelineFlat_vert_spv_filename, PipelineFlat_vert_spv_bytes);
    intrinsic.try_emplace(PipelineFlat_frag_spv_filename, PipelineFlat_frag_spv_bytes);
}

gsl::span<std::byte const> const StaticResources::get(std::string const &key) const
{
    let i = intrinsic.find(key);
    if (i == intrinsic.end()) {
        TTAURI_THROW(key_error("Could not find static resource")
            .set<"key"_tag>(key)
        );
    }
    return i->second;
}

}
