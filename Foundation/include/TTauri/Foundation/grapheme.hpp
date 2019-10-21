// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri {

/*! A grapheme, what a user thinks a character is.
 * This will exclude ligatures, because a user would see those as seperate characters.
 */
struct grapheme {
    //! codePoints representing the grapheme, normalized to NFC.
    std::u32string codePoints;

    grapheme() noexcept : codePoints({}) {}

    grapheme(std::u32string codePoints) noexcept :
        codePoints(Foundation_globals->unicodeData->toNFC(codePoints)) {}

    ~grapheme() {
    }

    grapheme(const grapheme& other) noexcept {
        codePoints = other.codePoints;
    }

    grapheme& operator=(const grapheme& other) noexcept {
        codePoints = other.codePoints;
        return *this;
    }

    grapheme(grapheme&& other) noexcept {
        codePoints = std::move(other.codePoints);
    }

    grapheme& operator=(grapheme&& other) noexcept {
        codePoints = std::move(other.codePoints);
        return *this;
    }

    std::u32string NFC() const noexcept {
        return codePoints;
    }

    std::u32string NFD() const noexcept {
        return Foundation_globals->unicodeData->toNFC(codePoints);
    }

    std::u32string NFKC() const noexcept {
        return Foundation_globals->unicodeData->toNFKC(codePoints);
    }

    std::u32string NFKD() const noexcept {
        return Foundation_globals->unicodeData->toNFKD(codePoints);
    }
};

inline bool operator<(grapheme const& a, grapheme const& b) noexcept {
    return a.NFKC() < b.NFKC();
}

inline bool operator==(grapheme const& a, grapheme const& b) noexcept {
    return a.NFKC() == b.NFKC();
}




}
