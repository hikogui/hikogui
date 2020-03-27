// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/Grapheme.hpp"
#include "TTauri/Foundation/strings.hpp"
#include <vector>

namespace TTauri::Text {

struct gstring {
    std::vector<Grapheme> graphemes;

    using const_iterator = std::vector<Grapheme>::const_iterator;
    using value_type = Grapheme;

    ssize_t size() const noexcept {
        return to_signed(graphemes.size());
    }

    Grapheme const &at(ssize_t i) const {
        return graphemes.at(i);
    }

    Grapheme &at(ssize_t i) {
        return graphemes.at(i);
    }

    decltype(auto) begin() noexcept { return graphemes.begin(); }
    decltype(auto) begin() const noexcept { return graphemes.begin(); }
    decltype(auto) cbegin() const noexcept { return graphemes.cbegin(); }
    decltype(auto) end() noexcept { return graphemes.end(); }
    decltype(auto) end() const noexcept { return graphemes.end(); }
    decltype(auto) cend() const noexcept { return graphemes.cend(); }

    decltype(auto) front() noexcept { return graphemes.front(); }
    decltype(auto) front() const noexcept { return graphemes.front(); }
    decltype(auto) back() noexcept { return graphemes.back(); }
    decltype(auto) back() const noexcept { return graphemes.back(); }

    gstring &operator+=(gstring const &rhs) noexcept {
        for (let &rhs_grapheme: rhs.graphemes) {
            graphemes.push_back(rhs_grapheme);
        }
        return *this;
    }

    gstring &operator+=(Grapheme const &grapheme) noexcept {
        graphemes.push_back(grapheme);
        return *this;
    }
};

}

namespace TTauri {

template<>
TTauri::Text::gstring translateString(std::u32string_view const inputString, TranslateStringOptions options) noexcept;

template<>
std::u32string translateString(const TTauri::Text::gstring& inputString, TranslateStringOptions options) noexcept;

}
