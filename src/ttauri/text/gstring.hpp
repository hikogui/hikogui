// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "grapheme.hpp"
#include "../strings.hpp"
#include <vector>

namespace tt::inline v1 {

struct gstring {
    std::vector<grapheme> graphemes;

    using const_iterator = std::vector<grapheme>::const_iterator;
    using value_type = grapheme;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return graphemes.empty();
    }

    size_t size() const noexcept
    {
        return graphemes.size();
    }

    grapheme const &at(size_t i) const
    {
        return graphemes.at(i);
    }

    grapheme &at(size_t i)
    {
        return graphemes.at(i);
    }

    auto begin() noexcept
    {
        return graphemes.begin();
    }
    auto begin() const noexcept
    {
        return graphemes.begin();
    }
    auto cbegin() const noexcept
    {
        return graphemes.cbegin();
    }
    auto end() noexcept
    {
        return graphemes.end();
    }
    auto end() const noexcept
    {
        return graphemes.end();
    }
    auto cend() const noexcept
    {
        return graphemes.cend();
    }

    decltype(auto) front() noexcept
    {
        return graphemes.front();
    }
    decltype(auto) front() const noexcept
    {
        return graphemes.front();
    }
    decltype(auto) back() noexcept
    {
        return graphemes.back();
    }
    decltype(auto) back() const noexcept
    {
        return graphemes.back();
    }

    [[nodiscard]] friend auto begin(gstring &rhs) noexcept
    {
        return rhs.begin();
    }

    [[nodiscard]] friend auto begin(gstring const &rhs) noexcept
    {
        return rhs.begin();
    }

    [[nodiscard]] friend auto cbegin(gstring const &rhs) noexcept
    {
        return rhs.cbegin();
    }

    [[nodiscard]] friend auto end(gstring &rhs) noexcept
    {
        return rhs.end();
    }

    [[nodiscard]] friend auto end(gstring const &rhs) noexcept
    {
        return rhs.end();
    }

    [[nodiscard]] friend auto cend(gstring const &rhs) noexcept
    {
        return rhs.cend();
    }

    [[nodiscard]] friend size_t size(gstring const &rhs) noexcept
    {
        return rhs.size();
    }

    gstring &operator+=(gstring const &rhs) noexcept
    {
        for (ttlet &rhs_grapheme : rhs.graphemes) {
            graphemes.push_back(rhs_grapheme);
        }
        return *this;
    }

    gstring &operator+=(grapheme const &grapheme) noexcept
    {
        graphemes.push_back(grapheme);
        return *this;
    }

    [[nodiscard]] friend std::u32string to_u32string(gstring const &rhs) noexcept
    {
        std::u32string r;
        r.reserve(rhs.size());
        for (ttlet &c : rhs) {
            r += c.NFC();
        }
        return r;
    }

    [[nodiscard]] friend std::string to_string(gstring const &rhs) noexcept
    {
        return tt::to_string(to_u32string(rhs));
    }

    friend std::ostream &operator<<(std::ostream &lhs, gstring const &rhs)
    {
        return lhs << to_string(rhs);
    }
};

[[nodiscard]] gstring to_gstring(std::u32string_view rhs) noexcept;

[[nodiscard]] inline gstring to_gstring(std::string_view rhs) noexcept
{
    return to_gstring(to_u32string(rhs));
}

} // namespace tt::inline v1
