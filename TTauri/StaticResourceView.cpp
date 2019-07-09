// Copyright 2019 Pokitec
// All rights reserved.

#include "StaticResourceView.hpp"
#include "TTauri/utils.hpp"

#define BINARY_ASSETS_PipelineImage_frag_spv_IMPL
#include "PipelineImage.frag.spv.hpp"
#define BINARY_ASSETS_PipelineImage_vert_spv_IMPL
#include "PipelineImage.vert.spv.hpp"

namespace TTauri {

int StaticResourceView::add(uint8_t const *data, size_t size, URL const &location)
{
    auto byteData = reinterpret_cast<std::byte const *>(data);

    StaticResourceView::objects.try_emplace(location, byteData, size);
    return 1;
}

}