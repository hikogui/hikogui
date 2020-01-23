// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/tagged_id.hpp"
#include "TTauri/Foundation/Grapheme.hpp"
#include <algorithm>
#include <utility>

namespace TTauri {

/** FontID is 15 bits.
 */
using FontID = tagged_id<uint16_t, "font_id"_tag, 0x7ffe>;
//static_cast(FontID::mask <= 0x7fff, "Font ID must be 15 bits or less.");
using GlyphID = tagged_id<uint16_t, "glyph_id"_tag>;

using FontFamilyID = tagged_id<uint16_t, "fontfamily_id"_tag>;


/** 
 */
struct FontIDGrapheme {
    FontID font_id;
    Grapheme g;

    [[nodiscard]] size_t hash() const noexcept {
        return std::hash<FontID>{}(font_id) ^ std::hash<Grapheme>{}(g);
    }

    [[nodiscard]] friend bool operator==(FontIDGrapheme const &lhs, FontIDGrapheme const &rhs) noexcept {
        return (lhs.font_id == rhs.font_id) && (lhs.g == rhs.g);
    }
};



// "Compatibility mappings are guaranteed to be no longer than 18 characters, although most consist of just a few characters."
// https://unicode.org/reports/tr44/ (TR44 5.7.3)
class FontGlyphsIDs_long {
    int8_t nr_glyphs = 0;
    std::array<GlyphID,18> glyph_ids;

    FontGlyphsIDs_long() noexcept = delete;
    FontGlyphsIDs_long(FontGlyphsIDs_long const &rhs) noexcept = default;
    FontGlyphsIDs_long(FontGlyphsIDs_long &&rhs) noexcept = default;
    FontGlyphsIDs_long &operator=(FontGlyphsIDs_long const &rhs) noexcept = default;
    FontGlyphsIDs_long &operator=(FontGlyphsIDs_long &&rhs) noexcept = default;

    force_inline FontGlyphsIDs_long(GlyphID g1, GlyphID g2, GlyphID g3) noexcept {
        (*this) += g1;
        (*this) += g2;
        (*this) += g3;
    }

    force_inline FontGlyphsIDs_long operator+=(GlyphID rhs) noexcept {
        ttauri_assume(nr_glyphs < ssize(glyph_ids));
        glyph_ids[nr_glyphs++] = rhs;
        return *this;
    }

    friend class FontGlyphIDs;
};

class FontGlyphIDs {
    constexpr static uint64_t empty = 0xffff'ffff'ffff'ffff;

    /*
     * 0 to 3 glyph_ids, with or without a font_id.
     * 63:48 glyph_id[2]
     * 47:32 glyph_id[1]
     * 31:16 glyph_id[0]
     *    15 '1'
     * 14: 0 font_id
     *
     * More than 3 glyphs
     * 63:16 FontGlyphsIDs_long *
     *    15 '0'
     * 14: 0 font_id
     */
    uint64_t value;

public:
    force_inline FontGlyphIDs() : value(empty) {}

    FontGlyphIDs(FontGlyphIDs const &rhs) : value(rhs.value) {
        if (rhs.has_pointer()) {
            value = new_pointer(*(rhs.get_pointer()));
        }
    }

    force_inline FontGlyphIDs(FontGlyphIDs &&rhs) : value(rhs.value) {
        rhs.value = empty;
    }

    FontGlyphIDs &operator=(FontGlyphIDs const &rhs) {
        delete_pointer();
        value = new_pointer(*(rhs.get_pointer()));
        return *this;
    }

    force_inline FontGlyphIDs &operator=(FontGlyphIDs &&rhs) {
        using std::swap;
        swap(value, rhs.value);
        return *this;
    }

    ~FontGlyphIDs() {
        delete_pointer();
    }

    void clear() noexcept {
        delete_pointer();
        value = empty;
    }

    force_inline operator bool () const noexcept {
        return size() > 0;
    }

    [[nodiscard]] force_inline FontID font_id() const noexcept {
        return FontID{value & FontID::mask};
    }

    force_inline void set_font_id(FontID font_id) noexcept {
        value = (value & ~static_cast<uint64_t>(FontID::mask)) | static_cast<uint64_t>(font_id);
    }

    FontGlyphIDs &operator+=(GlyphID rhs) noexcept {
        switch (size()) {
        case 0: value = (value & 0xffff'ffff'0000'ffff) | (static_cast<uint64_t>(rhs) << 16); break;
        case 1: value = (value & 0xffff'0000'ffff'ffff) | (static_cast<uint64_t>(rhs) << 32); break;
        case 2: value = (value & 0x0000'ffff'ffff'ffff) | (static_cast<uint64_t>(rhs) << 48); break;
        case 3:
            value = (value & FontID::mask) | new_pointer((*this)[0], (*this)[1], (*this)[2]);
            [[fallthrough]];
        default:
            *(get_pointer()) += rhs;
        }
        return *this;
    }

    [[nodiscard]] force_inline GlyphID front() const noexcept {
        if (size() == 0) {
            return GlyphID{};
        } else {
            return (*this)[0];
        }
    }

    [[nodiscard]] force_inline GlyphID operator[](size_t index) const noexcept {
        if (has_pointer()) {
            ttauri_assume(index < 18);
            return get_pointer()->glyph_ids[index];
        } else {
            switch (index) {
            case 0: return GlyphID{(value >> 16) & GlyphID::mask};
            case 1: return GlyphID{(value >> 32) & GlyphID::mask};
            case 2: return GlyphID{(value >> 48) & GlyphID::mask};
            default: no_default;
            }
        }
    }

    [[nodiscard]] force_inline size_t size() const noexcept {
        if (has_pointer()) {
            return get_pointer()->nr_glyphs;
        } else if (!(*this)[0]) {
            return 0;
        } else if (!(*this)[1]) {
            return 1;
        } else if (!(*this)[2]) {
            return 2;
        } else {
            return 3;
        }
    }

private:
    [[nodiscard]] force_inline bool has_pointer() const noexcept {
        return (value & 0x8000) == 0;
    }

    [[nodiscard]] force_inline FontGlyphsIDs_long const *get_pointer() const noexcept {
        ttauri_assume(has_pointer());
        return std::launder(reinterpret_cast<FontGlyphsIDs_long const *>(static_cast<ptrdiff_t>(value) >> 16));
    }

    [[nodiscard]] force_inline FontGlyphsIDs_long *get_pointer() noexcept {
        ttauri_assume(has_pointer());
        return std::launder(reinterpret_cast<FontGlyphsIDs_long *>(static_cast<ptrdiff_t>(value) >> 16));
    }

    void delete_pointer() noexcept {
        if (has_pointer()) {
            delete get_pointer();
            value = empty;
        }
    }

    template<typename... Args>
    [[nodiscard]] static uint64_t new_pointer(Args &&... args) noexcept {
        auto *ptr = new FontGlyphsIDs_long(std::forward<Args>(args)...);
        return static_cast<uint64_t>(reinterpret_cast<ptrdiff_t>(ptr) << 16);
    }

};

};

namespace std {

template<>
struct hash<TTauri::FontIDGrapheme> {
    [[nodiscard]] size_t operator() (TTauri::FontIDGrapheme const &rhs) const noexcept {
        return rhs.hash();
    }
};

}