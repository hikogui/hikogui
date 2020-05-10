// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/GlyphID.hpp"
#include "TTauri/Text/FontID.hpp"
#include "TTauri/Foundation/hash.hpp"
#include "TTauri/Foundation/tagged_id.hpp"
#include "TTauri/Foundation/aarect.hpp"
#include <tuple>

namespace TTauri {
struct Path;
}

namespace TTauri::Text {

// "Compatibility mappings are guaranteed to be no longer than 18 characters, although most consist of just a few characters."
// https://unicode.org/reports/tr44/ (TR44 5.7.3)
class FontGlyphIDs_long {
    int8_t nr_glyphs = 0;
    std::array<GlyphID,18> glyph_ids;

    FontGlyphIDs_long() noexcept = delete;
    FontGlyphIDs_long(FontGlyphIDs_long const &rhs) noexcept = default;
    FontGlyphIDs_long(FontGlyphIDs_long &&rhs) noexcept = default;
    FontGlyphIDs_long &operator=(FontGlyphIDs_long const &rhs) noexcept = default;
    FontGlyphIDs_long &operator=(FontGlyphIDs_long &&rhs) noexcept = default;

    force_inline FontGlyphIDs_long(GlyphID g1, GlyphID g2, GlyphID g3) noexcept {
        (*this) += g1;
        (*this) += g2;
        (*this) += g3;
    }

    force_inline FontGlyphIDs_long operator+=(GlyphID rhs) noexcept {
        ttauri_assume(nr_glyphs >= 0);
        ttauri_assume(nr_glyphs < ssize(glyph_ids));
        glyph_ids[nr_glyphs++] = rhs;
        return *this;
    }

    [[nodiscard]] size_t hash() const noexcept {
        ttauri_assume(nr_glyphs > 3);
        ttauri_assume(nr_glyphs < ssize(glyph_ids));

        uint64_t r = 0;
        for (int8_t i = 0; i != nr_glyphs; ++i) {
            r = hash_mix_two(r, std::hash<GlyphID>{}(glyph_ids[i]));
        }

        return r;
    }

    [[nodiscard]] friend bool operator==(FontGlyphIDs_long const &lhs, FontGlyphIDs_long const &rhs) noexcept {
        ttauri_assume(lhs.nr_glyphs > 3);
        ttauri_assume(rhs.nr_glyphs > 3);
        ttauri_assume(lhs.nr_glyphs < ssize(lhs.glyph_ids));
        ttauri_assume(rhs.nr_glyphs < ssize(rhs.glyph_ids));

        if (lhs.nr_glyphs == rhs.nr_glyphs) {
            for (int8_t i = 0; i != lhs.nr_glyphs; ++i) {
                if (lhs.glyph_ids[i] != rhs.glyph_ids[i]) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
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
        if (this != &rhs) {
            delete_pointer();
            value = rhs.value;
            if (rhs.has_pointer()) {
                value = new_pointer(*(rhs.get_pointer()));
            }
        }
        return *this;
    }

    force_inline FontGlyphIDs &operator=(FontGlyphIDs &&rhs) {
        if (this != &rhs) {
            using std::swap;
            swap(value, rhs.value);
        }
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

    [[nodiscard]] size_t hash() const noexcept {
        if (has_pointer()) {
            return get_pointer()->hash();
        } else {
            return std::hash<uint64_t>{}(value);
        }
    }

    [[nodiscard]] std::pair<Path,aarect> getPathAndBoundingBox() const noexcept;
    [[nodiscard]] aarect getBoundingBox() const noexcept;

private:
    [[nodiscard]] force_inline bool has_pointer() const noexcept {
        return (value & 0x8000) == 0;
    }

    [[nodiscard]] force_inline FontGlyphIDs_long const *get_pointer() const noexcept {
        ttauri_assume(has_pointer());
        return std::launder(reinterpret_cast<FontGlyphIDs_long const *>(static_cast<ptrdiff_t>(value) >> 16));
    }

    [[nodiscard]] force_inline FontGlyphIDs_long *get_pointer() noexcept {
        ttauri_assume(has_pointer());
        return std::launder(reinterpret_cast<FontGlyphIDs_long *>(static_cast<ptrdiff_t>(value) >> 16));
    }

    void delete_pointer() noexcept {
        if (has_pointer()) {
            delete get_pointer();
            value = empty;
        }
    }

    template<typename... Args>
    [[nodiscard]] static uint64_t new_pointer(Args &&... args) noexcept {
        auto *ptr = new FontGlyphIDs_long(std::forward<Args>(args)...);
        return static_cast<uint64_t>(reinterpret_cast<ptrdiff_t>(ptr) << 16);
    }

public:
    [[nodiscard]] friend bool operator==(FontGlyphIDs const &lhs, FontGlyphIDs const &rhs) noexcept {
        if (lhs.has_pointer() == rhs.has_pointer()) {
            if (lhs.has_pointer()) {
                return *(lhs.get_pointer()) == *(rhs.get_pointer());
            } else {
                return lhs.value == rhs.value;
            }
        } else {
            return false;
        }

        if (lhs.has_pointer() || rhs.has_pointer()) {
            if (lhs.size() != rhs.size()) {
                return false;
            } else {
                // At this point, both are pointers
            }
        } else {
            return lhs.value == rhs.value;
        }
    }
};

}

namespace std {

template<>
struct hash<TTauri::Text::FontGlyphIDs> {
    [[nodiscard]] size_t operator()(TTauri::Text::FontGlyphIDs const &rhs) const noexcept {
        return rhs.hash();
    }
};

}