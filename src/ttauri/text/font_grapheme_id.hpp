// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "font_id.hpp"
#include "grapheme.hpp"
#include "../hash.hpp"

namespace tt {

/** Combined font_id + grapheme for use as a key in a std::unordered_map.
 */
struct font_grapheme_id {
    font_id font_id;
    grapheme g;

    [[nodiscard]] size_t hash() const noexcept {
        return hash_mix(font_id, g);
    }

    [[nodiscard]] friend bool operator==(font_grapheme_id const &lhs, font_grapheme_id const &rhs) noexcept {
        return (lhs.font_id == rhs.font_id) && (lhs.g == rhs.g);
    }
};

}

namespace std {

template<>
struct hash<tt::font_grapheme_id> {
    [[nodiscard]] size_t operator() (tt::font_grapheme_id const &rhs) const noexcept {
        return rhs.hash();
    }
};

}