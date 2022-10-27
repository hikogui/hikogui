// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility.hpp"
#include "../strings.hpp"
#include "../unfair_mutex.hpp"
#include "../cast.hpp"
#include "../stable_set.hpp"
#include <cstdint>
#include <string>
#include <string_view>
#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>
#include <bit>

namespace hi::inline v1 {
namespace detail {

inline auto long_graphemes = ::hi::stable_set<std::u32string>{};

} // namespace detail

struct composed_t {};

/** A grapheme-cluster, what a user thinks a character is.
 *
 * A grapheme should not include typographical ligatures such as 'fi' as
 * the font should handle creating ligatures.
 *
 * If a grapheme is initialized with more than 1 code-points a long_grapheme
 * is allocated. This grapheme is never deleted from memory.
 *
 * This class is trivial and constant-destructible so that it can be used
 * as a character class in `std::basic_string` and used as a non-type template parameter.
 */
struct grapheme {
    using value_type = uint32_t;

    /** A pointer to a grapheme.
     *
     * This class will hold:
     * - A single code point between U+0000 to U+10ffff, or
     * - An index + 0x110000 into the long_graphemes table, or
     * - 0x1fffff meaning empty/eof.
     *
     * Bits [31:21] are always '0'.
     */
    value_type _value;

    constexpr grapheme() noexcept = default;
    constexpr grapheme(grapheme const&) noexcept = default;
    constexpr grapheme(grapheme&&) noexcept = default;
    constexpr grapheme& operator=(grapheme const&) noexcept = default;
    constexpr grapheme& operator=(grapheme&&) noexcept = default;

    constexpr grapheme(nullptr_t) noexcept : _value(0x1f'ffff) {}

    /** Encode a single code-point.
     */
    constexpr explicit grapheme(char32_t code_point) noexcept : _value(truncate<value_type>(code_point))
    {
        hi_axiom(code_point <= 0x10ffff);
    }

    constexpr explicit grapheme(char ascii_char) noexcept : _value(truncate<value_type>(ascii_char))
    {
        hi_axiom(ascii_char >= 0 and ascii_char <= 0x7f);
    }

    /** Encode a single code-point.
     */
    constexpr grapheme& operator=(char32_t code_point) noexcept
    {
        _value = truncate<value_type>(code_point);
        return *this;
    }

    /** Encode a single code-point.
     */
    constexpr grapheme& operator=(char ascii_char) noexcept
    {
        hi_axiom(ascii_char >= 0 and ascii_char <= 0x7f);
        _value = truncate<value_type>(ascii_char);
        return *this;
    }

    /** Encode a grapheme from a list of code-points.
     *
     * @param code_points The non-normalized list of code-points.
     */
    explicit grapheme(std::u32string_view code_points) noexcept;

    /** Encode a grapheme from a list of code-points.
     *
     * @param code_points The NFC/NKFC normalized and composed code-point of this grapheme.
     */
    explicit grapheme(composed_t, std::u32string_view code_points) noexcept;

    /** Create empty grapheme / end-of-file.
     */
    [[nodiscard]] static constexpr grapheme eof() noexcept
    {
        grapheme r;
        r._value = 0x1f'ffff;
        return r;
    }

    /** Clear the grapheme.
     */
    constexpr void clear() noexcept
    {
        _value = 0x1f'ffff;
    }

    /** Check if the grapheme is empty.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _value == 0x1f'ffff;
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

    [[nodiscard]] std::u32string const& long_grapheme() const noexcept
    {
        hi_assert(_value >= 0x10'0000 and _value < 0x1f'ffff);
        return detail::long_graphemes[_value - 0x11'0000];
    }

    /** Return the number of code-points encoded in the grapheme.
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        if (_value == 0x1f'ffff) {
            return 0;

        } else if (_value <= 0x10'ffff) {
            return 1;

        } else {
            return long_grapheme().size();
        }
    }

    /** Get the code-point at the given index.
     *
     * @note It is undefined-behavior to index beyond the number of encoded code-points.
     * @param i Index of code-point in the grapheme.
     * @return code-point at the given index.
     */
    [[nodiscard]] constexpr char32_t operator[](size_t i) const noexcept
    {
        hi_assert_bounds(i, *this);

        if (_value <= 0x10'ffff) {
            return truncate<char32_t>(_value);
        } else {
            return long_grapheme()[i];
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
    [[nodiscard]] friend constexpr char32_t get(grapheme const& rhs) noexcept
    {
        hi_assert_bounds(I, rhs);

        if (rhs._value <= 0x10'ffff) {
            return rhs._value;
        } else {
            return rhs.long_grapheme()[I];
        }
    }

    /** Get a list of code-point normalized to NFC.
     */
    [[nodiscard]] constexpr std::u32string composed() const noexcept
    {
        if (_value <= 0x10'ffff) {
            return std::u32string{truncate<char32_t>(_value)};
        } else {
            return long_grapheme();
        }
    }

    /** Get a list of code-point normalized to NFD.
     */
    [[nodiscard]] std::u32string decomposed() const noexcept;

    /** Compare equivalence of two graphemes.
     */
    [[nodiscard]] friend constexpr bool operator==(grapheme const&, grapheme const&) noexcept = default;

    /** Compare two graphemes lexicographically.
     */
    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const& lhs, grapheme const& rhs) noexcept
    {
        return lhs.decomposed() <=> rhs.decomposed();
    }

    [[nodiscard]] friend constexpr bool operator==(grapheme const& lhs, char32_t const& rhs) noexcept
    {
        return lhs == grapheme{rhs};
    }

    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const& lhs, char32_t const& rhs) noexcept
    {
        return lhs <=> grapheme{rhs};
    }

    [[nodiscard]] friend constexpr bool operator==(grapheme const& lhs, char const& rhs) noexcept
    {
        return lhs == grapheme{rhs};
    }

    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const& lhs, char const& rhs) noexcept
    {
        return lhs <=> grapheme{rhs};
    }

    [[nodiscard]] friend std::string to_string(grapheme const& rhs) noexcept
    {
        return hi::to_string(rhs.composed());
    }

    [[nodiscard]] friend std::u32string to_u32string(grapheme const& rhs) noexcept
    {
        return rhs.composed();
    }
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::grapheme> {
    [[nodiscard]] std::size_t operator()(hi::grapheme const& rhs) const noexcept
    {
        return std::hash<hi::grapheme::value_type>{}(rhs._value);
    }
};
