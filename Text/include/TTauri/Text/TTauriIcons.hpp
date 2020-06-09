// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/FontID.hpp"
#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/Text/FontBook.hpp"

namespace TTauri {

inline FontID TTauriIcons_font_id = FontID{};

enum class TTauriIcon : char32_t {
    MinimizeWindow = 0xf301,
    MaximizeWindowMS = 0xf302,
    RestoreWindowMS = 0xf303,
    CloseWindow = 0xf304,
    RestoreWindowMacOS = 0xf305,
    MaximizeWindowMacOS = 0xf306,
};

inline FontGlyphIDs to_FontGlyphIDs(TTauriIcon rhs) noexcept {
    ttauri_assume(TTauriIcons_font_id);
    ttauri_assume(fontBook);

    return fontBook->find_glyph(TTauriIcons_font_id, Grapheme{static_cast<char32_t>(rhs)});
}



}
