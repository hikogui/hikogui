// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_db_non_starter.hpp"
#include "../required.hpp"
#include "../strings.hpp"
#include <cstdint>
#include <string>
#include <cstddef>

namespace tt::inline v1 {

/** A grapheme, what a user thinks a character is.
 *
 * This class will hold:
 * - empty/eof (no code points encoded at all, the value 0)
 * - U+0000 (value 1)
 * - 1 starter code-point, followed by 0-4 combining characters.
 *
 * A grapheme should not include typographical ligatures such as 'fi' as
 * the font should handle creating ligatures.
 *
 * If a grapheme is initialized with more than 4 combining characters
 * the extra combining characters are dropped and can be detected by
 * calling `overlong()`.
 *
 * This class is trivial and constant-destructible so that it can be used
 * as a character class in `std::basic_string` and used as a non-type template parameter.
 *
 */
struct grapheme {
    using value_type = uint64_t;

    /**
     * [63:43] Starter code-point 0.
     * [42:33] Non-starter code 1
     * [32:23] Non-starter code 2
     * [22:13] Non-starter code 3
     * [12: 3] Non-starter code 4
     * [ 2: 0] Length 0-5, 6 == overlong, 7 == reserved.
     */
    value_type value;

    constexpr grapheme() noexcept = default;
    constexpr grapheme(grapheme const &) noexcept = default;
    constexpr grapheme(grapheme &&) noexcept = default;
    constexpr grapheme &operator=(grapheme const &) noexcept = default;
    constexpr grapheme &operator=(grapheme &&) noexcept = default;

    /** Encode a single code-point.
     */
    constexpr explicit grapheme(char32_t code_point) noexcept : value((static_cast<value_type>(code_point) << 43) | 1) {}

    /** Encode a single code-point.
     */
    constexpr grapheme &operator=(char32_t code_point) noexcept
    {
        value = (static_cast<value_type>(code_point) << 43) | 1;
        return *this;
    }

    /** Encode a single code-point.
     */
    constexpr explicit grapheme(char code_point) noexcept : value((static_cast<value_type>(code_point) << 43) | 1) {}

    /** Encode a single code-point.
     */
    constexpr grapheme &operator=(char code_point) noexcept
    {
        value = (static_cast<value_type>(code_point) << 43) | 1;
        return *this;
    }

    /** Encode a grapheme from a list of code-points.
    * 
    * @param code_points The non-normalized list of code-points.
    */
    explicit grapheme(std::u32string_view code_points) noexcept;

    /** Encode a grapheme from a list of code-points.
     *
     * @param code_points The non-normalized list of code-points.
     */
    grapheme &operator=(std::u32string_view code_points) noexcept;

    /** Encode a grapheme from a list of NFKC-normalized code-points.
     *
     * @param code_points The NFKC-normalized list of code-points.
     */
    [[nodiscard]] static grapheme from_composed(std::u32string_view code_points) noexcept;

    /** Create empty grapheme / end-of-file.
     */
    [[nodiscard]] static constexpr grapheme eof() noexcept
    {
        grapheme r;
        r.value = 0;
        return r;
    }

    /** Clear the grapheme.
     */
    constexpr void clear() noexcept
    {
        value = 0;
    }

    /** Check if the grapheme is empty.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return value == 0;
    }

    /** Check if the grapheme holds any code-points.
     */
    constexpr operator bool() const noexcept
    {
        return not empty();
    }

    /** Check if the grapheme is valid.
    * 
    * A grapheme is invalid in case:
    * - The grapheme is empty.
    * - The first code-point is part of general category 'C'.
    * - The first code-point is a combining character; canonical combining class != 0.
    */
    [[nodiscard]] bool valid() const noexcept;

    /** Return the number of code-points encoded in the grapheme.
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        ttlet size_ = value & 0x7;
        return size_ <= 5 ? size_ : 5;
    }

    /** Check if the grapheme was initialized with more than 4 combining characters.
     */
    [[nodiscard]] constexpr bool overlong() const noexcept
    {
        return (value & 0x7) == 6;
    }

    /** Get the code-point at the given index.
     *
     * @note It is undefined-behavior to index beyond the number of encoded code-points.
     * @param i Index of code-point in the grapheme.
     * @return code-point at the given index.
     */
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

    /** Get the code-point at the given index.
     *
     * @note It is undefined-behavior to index beyond the number of encoded code-points.
     * @tparam I Index of code-point in the grapheme.
     * @param rhs The grapheme to query.
     * @return code-point at the given index.
     */
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

    /** Get a list of code-point normalized to NFC.
     */
    [[nodiscard]] constexpr std::u32string composed() const noexcept
    {
        auto r = std::u32string{};
        r.reserve(size());
        for (std::size_t i = 0; i != size(); ++i) {
            r += (*this)[i];
        }
        return r;
    }

    /** Get a list of code-point normalized to NFD.
     */
    [[nodiscard]] std::u32string decomposed() const noexcept;

    /** Compare equivalence of two graphemes.
     */
    [[nodiscard]] friend constexpr bool operator==(grapheme const &a, grapheme const &b) noexcept = default;

    /** Compare two graphemes lexicographically.
     */
    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const &a, grapheme const &b) noexcept = default;

    [[nodiscard]] friend constexpr bool operator==(grapheme const &lhs, char32_t const &rhs) noexcept
    {
        return lhs == grapheme{rhs};
    }

    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const &lhs, char32_t const &rhs) noexcept
    {
        return lhs <=> grapheme{rhs};
    }

    [[nodiscard]] friend constexpr bool operator==(grapheme const &lhs, char const &rhs) noexcept
    {
        return lhs == grapheme{rhs};
    }

    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const &lhs, char const &rhs) noexcept
    {
        return lhs <=> grapheme{rhs};
    }

    [[nodiscard]] friend std::string to_string(grapheme const &rhs) noexcept
    {
        return tt::to_string(rhs.composed());
    }

    [[nodiscard]] friend std::u32string to_u32string(grapheme const &rhs) noexcept
    {
        return rhs.composed();
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
