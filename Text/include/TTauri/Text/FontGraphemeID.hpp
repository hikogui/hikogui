// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/hash.hpp"
#include "TTauri/Text/FontID.hpp"
#include "TTauri/Text/Grapheme.hpp"

namespace TTauri {

/** Combined FontID + Grapheme for use as a key in a std::unordered_map.
 */
struct FontGraphemeID {
    FontID font_id;
    Grapheme g;

    [[nodiscard]] size_t hash() const noexcept {
        return hash_mix(font_id, g);
    }

    [[nodiscard]] friend bool operator==(FontGraphemeID const &lhs, FontGraphemeID const &rhs) noexcept {
        return (lhs.font_id == rhs.font_id) && (lhs.g == rhs.g);
    }
};

}

namespace std {

template<>
struct hash<TTauri::FontGraphemeID> {
    [[nodiscard]] size_t operator() (TTauri::FontGraphemeID const &rhs) const noexcept {
        return rhs.hash();
    }
};

}