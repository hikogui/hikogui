// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/geometry.hpp"

namespace TTauri::GUI::PipelineImage {

struct Page {
    static const size_t width = 64;
    static const size_t height = 64;
    static const size_t border = 1;
    static const size_t widthIncludingBorder = width + 2 * border;
    static const size_t heightIncludingBorder = height + 2 * border;

    size_t nr;

    bool isFullyTransparent() const { return nr == std::numeric_limits<size_t>::max(); }
};

}
