
#pragma once

#include "ft2build.h"
#include FT_FREETYPE_H

#include <memory>

namespace TTauri::Draw {

class FreeType {
public:
    FT_Library intrinsic;

    FreeType();

    virtual ~FreeType();

    FreeType(const FreeType &) = delete;
    FreeType &operator=(const FreeType &) = delete;
    FreeType(FreeType &&) = delete;
    FreeType &operator=(FreeType &&) = delete;

    static std::shared_ptr<FreeType> singleton;
};

}