// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "Grapheme.hpp"
#include "../strings.hpp"
#include <vector>

namespace tt {

struct gstring {
    std::vector<Grapheme> graphemes;

    using const_iterator = std::vector<Grapheme>::const_iterator;
    using value_type = Grapheme;

    ssize_t size() const noexcept {
        return ssize(graphemes);
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
        for (ttlet &rhs_grapheme: rhs.graphemes) {
            graphemes.push_back(rhs_grapheme);
        }
        return *this;
    }

    gstring &operator+=(Grapheme const &grapheme) noexcept {
        graphemes.push_back(grapheme);
        return *this;
    }

    [[nodiscard]] friend std::u32string to_u32string(gstring const &rhs) noexcept {
        std::u32string r;
        r.reserve(ssize(rhs));
        for (ttlet &c : rhs) {
            r += c.NFC();
        }
        return r;
    }

    [[nodiscard]] friend std::string to_string(gstring const &rhs) noexcept {
        return tt::to_string(to_u32string(rhs));
    }


    friend std::ostream &operator<<(std::ostream &lhs, gstring const &rhs) {
        return lhs << to_string(rhs);
    }
};

[[nodiscard]] gstring to_gstring(std::u32string_view rhs) noexcept;

[[nodiscard]] inline gstring to_gstring(std::string_view rhs) noexcept {
    return to_gstring(tt::to_u32string(rhs));
}



}
