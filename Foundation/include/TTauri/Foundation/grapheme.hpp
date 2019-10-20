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

struct gstring {
    std::vector<grapheme> graphemes;

    using const_iterator = std::vector<grapheme>::const_iterator;
    using value_type = grapheme;

    ssize_t size() const noexcept {
        return to_signed(graphemes.size());
    }

    grapheme const &at(ssize_t i) const {
        return graphemes.at(i);
    }

    grapheme &at(ssize_t i) {
        return graphemes.at(i);
    }

    auto begin() const noexcept {
        return graphemes.begin();
    }

    auto end() const noexcept {
        return graphemes.end();
    }

    gstring &operator+=(gstring const &rhs) noexcept {
        for (let &rhs_grapheme: rhs.graphemes) {
            graphemes.push_back(rhs_grapheme);
        }
        return *this;
    }

    gstring &operator+=(grapheme const &grapheme) noexcept {
        graphemes.push_back(grapheme);
        return *this;
    }
};

template<>
inline gstring translateString(std::u32string_view const inputString, TranslateStringOptions options) noexcept
{
    let normalizedString = Foundation_globals->unicodeData->toNFC(inputString, true, true);

    auto outputString = gstring{};
    auto breakState = GraphemeBreakState{};
    auto cluster = std::u32string{};

    for (let codePoint : normalizedString) {
        if (Foundation_globals->unicodeData->checkGraphemeBreak(codePoint, breakState)) {
            if (cluster.size() > 0) {
                outputString += grapheme(cluster);
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    outputString += grapheme(cluster);
    return outputString;
}

template<>
inline std::u32string translateString(const gstring& inputString, TranslateStringOptions options) noexcept
{
    std::u32string outputString;

    for (let c : inputString) {
        outputString += c.NFC();
    }
    return outputString;
}


}
