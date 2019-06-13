// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <limits>

namespace TTauri::GUI::PipelineImage {

struct Page {
    static constexpr size_t width = 64;
    static constexpr size_t height = 64;
    static constexpr size_t border = 1;
    static constexpr size_t widthIncludingBorder = width + 2 * border;
    static constexpr size_t heightIncludingBorder = height + 2 * border;

    size_t nr;

    bool isFullyTransparent() const { return nr == std::numeric_limits<size_t>::max(); }
};

}
