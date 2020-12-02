// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "FontID.hpp"
#include "FontGlyphIDs.hpp"
#include "FontBook.hpp"
#include "../application.hpp"

namespace tt {

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
    tt_assume(TTauriIcons_font_id);
    tt_assume(application::global->fonts);

    return application::global->fonts->find_glyph(TTauriIcons_font_id, Grapheme{static_cast<char32_t>(rhs)});
}



}
