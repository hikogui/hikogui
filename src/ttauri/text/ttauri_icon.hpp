// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_id.hpp"
#include "font_glyph_ids.hpp"
#include "font_book.hpp"
#include "../application.hpp"

namespace tt {

inline font_id ttauri_icons_font_id = font_id{};

enum class ttauri_icon : char32_t {
    MinimizeWindow = 0xf301,
    MaximizeWindowMS = 0xf302,
    RestoreWindowMS = 0xf303,
    CloseWindow = 0xf304,
    RestoreWindowMacOS = 0xf305,
    MaximizeWindowMacOS = 0xf306,
};

inline font_glyph_ids to_font_glyph_ids(ttauri_icon rhs) noexcept {
    tt_axiom(ttauri_icons_font_id);
    tt_axiom(font_book::global);

    return font_book::global->find_glyph(ttauri_icons_font_id, grapheme{static_cast<char32_t>(rhs)});
}



}
