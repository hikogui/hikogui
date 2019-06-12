// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "strings.hpp"

namespace TTauri {

/*! A grapheme, what a user thinks a character is.
 * This will exclude ligatures, because a user would see those as seperate characters.
 */
struct grapheme {
    //! codePoints representing the grapheme, normalized to NFC.
    std::u32string codePoints;

    grapheme() : codePoints({}) {}

    grapheme(std::u32string codePoints) :
        codePoints(translateString<std::u32string>(normalizeNFC(translateString<std::string>(codePoints)))) {}

    ~grapheme() {
    }

    grapheme(const grapheme& other) {
        codePoints = other.codePoints;
    }

    grapheme& operator=(const grapheme& other) {
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

    std::u32string NFC() const {
        return codePoints;
    }

    std::u32string NFD() const {
        return translateString<std::u32string>(normalizeNFD(translateString<std::string>(codePoints)));
    }

    std::u32string NFKC() const {
        return translateString<std::u32string>(normalizeNFKC(translateString<std::string>(codePoints)));
    }

    std::u32string NFKD() const {
        return translateString<std::u32string>(normalizeNFKD(translateString<std::string>(codePoints)));
    }

    std::u32string NFKCCasefold() const {
        return translateString<std::u32string>(normalizeNFKCCasefold(translateString<std::string>(codePoints)));
    }

};

inline bool operator<(grapheme const& a, grapheme const& b) {
    return a.NFKCCasefold() < b.NFKCCasefold();
}

inline bool operator==(grapheme const& a, grapheme const& b) {
    return a.NFKCCasefold() == b.NFKCCasefold();
}

struct gstring {
    std::vector<grapheme> graphemes;

    using const_iterator = std::vector<grapheme>::const_iterator;
    using value_type = grapheme;

    size_t size() const {
        return graphemes.size();
    }

    grapheme const &at(size_t i) const {
        return graphemes.at(i);
    }

    grapheme &at(size_t i) {
        return graphemes.at(i);
    }

    typename const_iterator begin() const {
        return graphemes.begin();
    }

    typename const_iterator end() const {
        return graphemes.end();
    }

    gstring &operator+=(gstring const &str) {
        for (let grapheme: graphemes) {
            graphemes.push_back(std::move(grapheme));
        }
        return *this;
    }

    gstring &operator+=(grapheme const grapheme) {
        graphemes.push_back(std::move(grapheme));
        return *this;
    }
};

template<>
inline gstring translateString(const std::u32string& inputString, TranslateStringOptions options)
{
    gstring outputString;
    std::u32string cluster;
    utf8proc_int32_t breakState = 0;
    utf8proc_int32_t previousCodePoint = -1;

    for (let currentCodePoint : inputString) {
        let sl = splitLigature(currentCodePoint);
        if (sl.size() > 0) {
            outputString += grapheme(cluster);
            cluster.clear();
            for (let c : sl) {
                outputString += grapheme({ c });
            }
            breakState = 0;
            continue;
        }

        if (previousCodePoint >= 0) {
            if (utf8proc_grapheme_break_stateful(previousCodePoint, currentCodePoint, &breakState)) {
                outputString += grapheme(cluster);
                cluster.clear();
            }
        }

        cluster += currentCodePoint;
        previousCodePoint = currentCodePoint;
    }
    outputString += grapheme(cluster);
    return outputString;
}

template<>
inline std::u32string translateString(const gstring& inputString, TranslateStringOptions options)
{
    std::u32string outputString;

    for (let c : inputString) {
        outputString += c.NFC();
    }
    return outputString;
}


}
