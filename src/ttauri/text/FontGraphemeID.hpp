// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/hash.hpp"
#include "ttauri/text/FontID.hpp"
#include "ttauri/text/Grapheme.hpp"

namespace tt {

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
struct hash<tt::FontGraphemeID> {
    [[nodiscard]] size_t operator() (tt::FontGraphemeID const &rhs) const noexcept {
        return rhs.hash();
    }
};

}