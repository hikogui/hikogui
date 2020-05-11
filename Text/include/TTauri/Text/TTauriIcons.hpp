// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/FontID.hpp"
#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/Text/FontBook.hpp"

namespace TTauri::Text {

inline FontID TTauriIcons_font_id = FontID{};

enum class TTauriIcon : char32_t {
    MinimizeWindow = 0xf301,
    MaximizeWindow = 0xf302,
    RestoreWindow = 0xf303,
    CloseWindow = 0xf304,
};

inline FontGlyphIDs to_FontGlyphIDs(TTauriIcon rhs) noexcept {
    ttauri_assume(TTauriIcons_font_id);
    ttauri_assume(fontBook);

    return fontBook->find_glyph(TTauriIcons_font_id, Grapheme{static_cast<char32_t>(rhs)});
}



}
