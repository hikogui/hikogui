// Copyright 2019 Pokitec
// All rights reserved.

#include "StaticResources.hpp"

namespace TTauri {

#include "PipelineImage.vert.spv.inl"
#include "PipelineImage.frag.spv.inl"

StaticResources::StaticResources()
{
    intrinsic.try_emplace(PipelineImage_vert_spv_url, PipelineImage_vert_spv_bytes);
    intrinsic.try_emplace(PipelineImage_frag_spv_url, PipelineImage_frag_spv_bytes);
}

gsl::span<std::byte const> const &StaticResources::get(URL const &location) const
{
    let i = intrinsic.find(location);
    if (i == intrinsic.end()) {
        BOOST_THROW_EXCEPTION(FileError("Could not find static resource")
            << errinfo_url(location)
        );
    }
    return i->second;
}

}