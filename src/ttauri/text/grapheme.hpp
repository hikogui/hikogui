// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_db_non_starter.hpp"
#include <cstdint>
#include <string>
#include <cstddef>

namespace tt::inline v1 {

/** A grapheme, what a user thinks a character is.
 * This will exclude ligatures, because a user would see those as separate characters.
 */
struct grapheme {
    using value_type = uint64_t;

    /**
     * [63:43] Starter code-point 0.
     * [42:33] Non-starter code 1
     * [32:23] Non-starter code 2
     * [22:13] Non-starter code 3
     * [12: 3] Non-starter code 4
     * [ 2: 0] Length 0-5, 6 == overlong, 7 == eof.
     */
    value_type value;

    constexpr grapheme() noexcept = default;
    constexpr grapheme(grapheme const &) noexcept = default;
    constexpr grapheme(grapheme &&) noexcept = default;
    constexpr grapheme &operator=(grapheme const &) noexcept = default;
    constexpr grapheme &operator=(grapheme &&) noexcept = default;

    constexpr explicit grapheme(char32_t code_point) noexcept : value((static_cast<value_type>(code_point) << 43) | 1) {}

    constexpr grapheme &operator=(char32_t code_point) noexcept
    {
        value = (static_cast<value_type>(code_point) << 43) | 1;
        return *this;
    }

    explicit grapheme(std::u32string_view code_points) noexcept;
    grapheme &operator=(std::u32string_view code_points) noexcept;
    [[nodiscard]] static grapheme from_NFC(std::u32string_view code_points) noexcept;

    constexpr operator bool() const noexcept
    {
        return value != 0;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        ttlet size_ = value & 0x7;
        return size_ == 7 ? 0 : size_ <= 5 ? size_ : 5;
    }

    [[nodiscard]] constexpr char32_t operator[](size_t i) const noexcept
    {
        tt_axiom(i < size());

        if (i == 0) {
            return static_cast<char32_t>(value >> 43);

        } else {
            ttlet shift = (4 - i) * 10 + 3;
            return detail::unicode_db_non_starter_table[(value >> shift) & 0x3ff];
        }
    }

    template<size_t I>
    [[nodiscard]] friend constexpr char32_t get(grapheme const &rhs) noexcept
    {
        if constexpr (I == 0) {
            return static_cast<char32_t>(rhs.value >> 43);

        } else {
            constexpr auto shift = (4 - I) * 10 + 3;
            return detail::unicode_db_non_starter_table[(rhs.value >> shift) & 0x3ff];
        }
    }

    [[nodiscard]] constexpr char32_t front() const noexcept
    {
        return get<0>(*this);
    }

    [[nodiscard]] std::u32string NFC() const noexcept
    {
        std::u32string r;
        r.reserve(size());
        for (std::size_t i = 0; i != size(); ++i) {
            r += (*this)[i];
        }
        return r;
    }

    [[nodiscard]] std::u32string NFD() const noexcept;

    [[nodiscard]] std::u32string NFKC() const noexcept;

    [[nodiscard]] std::u32string NFKD() const noexcept;

    /** Paragraph separator.
     */
    static grapheme PS() noexcept
    {
        return grapheme(U'\u2029');
    }

    /** Line separator.
     */
    static grapheme LS() noexcept
    {
        return grapheme(U'\u2028');
    }

    [[nodiscard]] friend constexpr bool operator==(grapheme const &a, grapheme const &b) noexcept = default;
    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const &a, grapheme const &b) noexcept = default;

    [[nodiscard]] friend bool operator==(grapheme const &lhs, char32_t const &rhs) noexcept
    {
        return (lhs.size() == 1) && (get<0>(lhs) == rhs);
    }

    [[nodiscard]] friend bool operator==(grapheme const &lhs, char const &rhs) noexcept
    {
        return lhs == static_cast<char32_t>(rhs);
    }

    [[nodiscard]] friend std::string to_string(grapheme const &rhs) noexcept
    {
        return to_string(rhs.NFC());
    }

    [[nodiscard]] friend std::u32string to_u32string(grapheme const &rhs) noexcept
    {
        return rhs.NFC();
    }
};

} // namespace tt::inline v1

template<>
struct std::hash<tt::grapheme> {
    [[nodiscard]] std::size_t operator()(tt::grapheme const &rhs) const noexcept
    {
        return std::hash<tt::grapheme::value_type>{}(rhs.value);
    }
};
