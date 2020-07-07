// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <limits>
#include "ttauri/foundation/required.hpp"

namespace tt::PipelineImage {

struct Page {
    static constexpr int width = 64;
    static constexpr int height = 64;
    static constexpr int border = 1;
    static constexpr int widthIncludingBorder = width + 2 * border;
    static constexpr int heightIncludingBorder = height + 2 * border;

    ssize_t nr;

    Page(ssize_t nr) : nr(nr) {}

    /*! Create a transparent page.
     */
    Page() : nr(std::numeric_limits<ssize_t>::max()) {}

    bool isFullyTransparent() const noexcept { return nr == std::numeric_limits<ssize_t>::max(); }
};

}
