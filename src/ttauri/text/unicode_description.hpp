// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "unicode_general_category.hpp"
#include "unicode_bidi_class.hpp"
#include "unicode_bidi_bracket_type.hpp"
#include "unicode_grapheme_cluster_break.hpp"

namespace tt {

class unicode_description {
public:
    [[nodiscard]] constexpr unicode_description(
        char32_t code_point,
        unicode_general_category general_category,
        unicode_grapheme_cluster_break grapheme_cluster_break,
        unicode_bidi_class bidi_class,
        unicode_bidi_bracket_type bidi_bracket_type,
        char32_t bidi_mirrored_glyph,
        bool decomposition_canonical,
        uint8_t decomposition_combining_class,
        uint8_t decomposition_length,
        uint32_t decomposition_index
    ) noexcept :
        general_info((static_cast<uint32_t>(code_point) << 10) | (static_cast<uint32_t>(general_category) << 5) | (static_cast<uint32_t>(grapheme_cluster_break) << 1)),
        bidi_class(static_cast<uint32_t>(bidi_class)),
        bidi_bracket_type(static_cast<uint32_t>(bidi_bracket_type)),
        bidi_mirrored_glyph(static_cast<uint32_t>(bidi_mirrored_glyph)),
        decomposition_canonical(static_cast<uint32_t>(decomposition_canonical)),
        decomposition_combining_class(static_cast<uint32_t>(decomposition_combining_class)),
        decomposition_index(static_cast<uint32_t>(decomposition_index)),
        decomposition_length(static_cast<uint32_t>(decomposition_length))
    {
        tt_axiom(code_point <= 0x10ffff);
        tt_axiom(static_cast<uint32_t>(general_category) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(grapheme_cluster_break) <= 0x0f);
        tt_axiom(static_cast<uint32_t>(bidi_class) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(bidi_bracket_type) <= 0x03);
        tt_axiom(static_cast<uint32_t>(bidi_mirrored_glyph) <= 0x10ffff);
        tt_axiom(static_cast<uint32_t>(decomposition_combining_class) <= 0xff);
        tt_axiom(static_cast<uint32_t>(decomposition_length) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(decomposition_index) <= 0x1f'ffff);
    }

private:
    // 1st dword
    // code_point must be in msb for fast binary search, so no bit-fields here.
    // [31:10] code-point
    // [9:5] general category
    // [4:1] grapheme cluster break
    // [0:0] reserved
    uint32_t general_info;

    // 2nd dword
    uint32_t bidi_class:5;
    uint32_t bidi_bracket_type:2;
    uint32_t bidi_mirrored_glyph : 21;
    uint32_t bidi_reserved : 4 = 0;

    // 3rd dword
    uint32_t decomposition_canonical:1;
    uint32_t decomposition_combining_class : 8;
    uint32_t decomposition_index : 21;
    uint32_t decomposition_reserved1 : 2 = 0;

    // 4th dword
    uint32_t decomposition_length : 5;
    uint32_t decomposition_reserved2 : 27 = 0;
};

static_assert(sizeof(unicode_description) == 16);

}
