// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pixel_map.inl"
#include "endian.hpp"
#include <algorithm>

namespace hi::inline v1 {

void mergeMaximum(pixel_map<uint8_t> &dst, pixel_map<uint8_t> const &src) noexcept
{
    hi_assert(src.width() >= dst.width());
    hi_assert(src.height() >= dst.height());

    for (auto rowNr = 0; rowNr < dst.height(); rowNr++) {
        auto dstRow = dst[rowNr];
        hilet srcRow = src[rowNr];
        for (auto columnNr = 0; columnNr < dstRow.width(); columnNr++) {
            auto &dstPixel = dstRow[columnNr];
            hilet srcPixel = srcRow[columnNr];
            dstPixel = std::max(dstPixel, srcPixel);
        }
    }
}

} // namespace hi::inline v1
