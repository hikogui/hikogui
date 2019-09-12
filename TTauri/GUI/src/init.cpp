// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/init.hpp"
#include "TTauri/Foundation/globals.hpp"

#include "PipelineImage.vert.spv.inl"
#include "PipelineImage.frag.spv.inl"
#include "PipelineFlat.vert.spv.inl"
#include "PipelineFlat.frag.spv.inl"

namespace TTauri::GUI {

void GUI_init()
{
    staticResources.try_emplace(PipelineImage_vert_spv_filename, PipelineImage_vert_spv_bytes);
    staticResources.try_emplace(PipelineImage_frag_spv_filename, PipelineImage_frag_spv_bytes);
    staticResources.try_emplace(PipelineFlat_vert_spv_filename, PipelineFlat_vert_spv_bytes);
    staticResources.try_emplace(PipelineFlat_frag_spv_filename, PipelineFlat_frag_spv_bytes);
}

}
