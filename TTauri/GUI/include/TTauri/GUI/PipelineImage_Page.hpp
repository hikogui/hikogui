// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <limits>

namespace TTauri::GUI::PipelineImage {

struct Page {
    static constexpr int width = 64;
    static constexpr int height = 64;
    static constexpr int border = 1;
    static constexpr int widthIncludingBorder = width + 2 * border;
    static constexpr int heightIncludingBorder = height + 2 * border;

    int nr;

    bool isFullyTransparent() const noexcept { return nr == std::numeric_limits<int>::max(); }
};

}
