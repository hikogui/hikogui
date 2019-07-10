// Copyright 2019 Pokitec
// All rights reserved.

#include "StaticResources.hpp"

namespace TTauri {

#include "GUI/PipelineImage.vert.spv.inl"
#include "GUI/PipelineImage.frag.spv.inl"

StaticResources::StaticResources()
{
    intrinsic.try_emplace(PipelineImage_vert_spv_filename, PipelineImage_vert_spv_bytes);
    intrinsic.try_emplace(PipelineImage_frag_spv_filename, PipelineImage_frag_spv_bytes);
}

gsl::span<std::byte const> const &StaticResources::get(std::string const &filename) const
{
    let i = intrinsic.find(filename);
    if (i == intrinsic.end()) {
        BOOST_THROW_EXCEPTION(FileError("Could not find static resource")
            << boost::errinfo_file_name(filename)
        );
    }
    return i->second;
}

}