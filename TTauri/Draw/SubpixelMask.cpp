// Copyright 2019 Pokitec
// All rights reserved.

#include "SubpixelMask.hpp"
#include "TTauri/Color.hpp"
#include <algorithm>

namespace TTauri::Draw {



void composit(PixelMap<wsRGBApm> &under, wsRGBApm over, SubpixelMask const& mask)
{
    assert(mask.height >= under.height);
    assert((mask.width * 3) >= under.width);

    for (size_t rowNr = 0; rowNr < under.height; rowNr++) {
        let maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (size_t maskColumnNr = 0, columnNr = 0; columnNr < static_cast<size_t>(underRow.width); columnNr++, maskColumnNr += 3) {
            let mask = glm::u8vec3 {
                maskRow[maskColumnNr],
                maskRow[maskColumnNr + 1],
                maskRow[maskColumnNr + 2]
            };

            auto &pixel = underRow[columnNr];
            pixel.subpixelComposit(over, mask);
        }
    }
}

}
