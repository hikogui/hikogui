// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../strings.hpp"
#include <cctype>

namespace hi::inline v1 {

/** ISO-639 language code.
 *
 * A 2 or 3 lower case language code selected from the following iso standards:
 *  1. ISO 639-1 (2002)
 *  2. ISO 639-2 (1998)
 *  3. ISO 639-3 (2007)
 *  4. ISO 639-5 (2008)
 *
 * This class compresses this 2 or 3 character language code inside 16 bits,
 * so that together with the script only 32 bits are needed per attributed character.
 */
class iso_639 {
public:
    /** Set the letter at a specific position.
     *
     * @tparam I index
     * @param rhs The language code to change.
     * @param c The character to set. a-z, A-Z, 0-5 or nul.
     */
    template<std::size_t I>
    constexpr friend iso_639& set(iso_639& rhs, char c)
    {
        hi_check(
            c == 0 or (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or (c >= '1' and c <= '5'),
            "Must be letters or the digits between '1' and '5', or nul");

        // clang-format off
        uint16_t const x =
            (c >= 'a' and c <= 'z') ? c - 'a' + 1 :
            (c >= 'A' and c <= 'Z') ? c - 'A' + 1 :
            (c >= '1' and c <= '5') ? c - '1' + 27 :
            0;
        // clang-format on

        hi_assert(x <= 0x1f);
        constexpr auto shift = I * 5;
        rhs._v &= ~(0x1f << shift);
        rhs._v |= x << shift;
        return rhs;
    }

        /** Get the letter at a specific position.
     *
     * @tparam I index
     * @param rhs The language code read from.
     * @return The character at index, a-z, 0-5 or nul.
     */
    template<std::size_t I>
    [[nodiscard]] constexpr friend char get(iso_639 const& rhs) noexcept
    {
        constexpr auto shift = I * 5;
        hilet x = (rhs._v >> shift) & 0x1f;
        if (x == 0) {
            return 0;
        } else if (x <= 26) {
            return 'a' + narrow_cast<char>(x - 1);
        } else {
            return '1' + narrow_cast<char>(x - 27);
        }
    }

    constexpr iso_639(iso_639 const&) noexcept = default;
    constexpr iso_639(iso_639&&) noexcept = default;
    constexpr iso_639& operator=(iso_639 const&) noexcept = default;
    constexpr iso_639& operator=(iso_639&&) noexcept = default;

    /** Construct empty language.
     */
    constexpr iso_639() noexcept : _v(0) {}

    /** Construct a language from the 2 or 3 letter code.
     */
    constexpr iso_639(std::string_view str) : _v(0)
    {
        try {
            hi_check(str.size() == 2 or str.size() == 3, "ISO-639 incorrect length.");

            set<0>(*this, str[0]);
            set<1>(*this, str[1]);
            if (str.size() == 3) {
                set<2>(*this, str[2]);
            }
        } catch (...) {
            throw parse_error(std::format("A ISO-639 language code must be 2 or 3 letters in length, got '{}'", str));
        }
    }

    /** Get the number of character.
     *
     * @return 2 or 3 for a code, or 0 if empty.
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        hilet tmp = _v & 0x7fff;
        // clang-format off
        return
            tmp == 0 ? 0 :
            tmp <= 0x1f ? 1 :
            tmp <= 0x3ff ? 2 :
            3;
        // clang-format on
    }

    /** Check if the language is empty.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _v == 0;
    }

    /** Check if the language is used.
     */
    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Get the hash value for this language code.
     */
    [[nodiscard]] size_t hash() const noexcept
    {
        return std::hash<uint16_t>{}(_v);
    }

    /** Get the 2 or 3 letter ISO-639 code.
     */
    [[nodiscard]] constexpr std::string code() const noexcept
    {
        auto r = std::string{};
        if (size() >= 2) {
            r += get<0>(*this);
            r += get<1>(*this);
        }
        if (size() == 3) {
            r += get<2>(*this);
        }
        return r;
    }

    /** Compare two language codes.
     */
    [[nodiscard]] constexpr friend bool operator==(iso_639 const& lhs, iso_639 const& rhs) noexcept = default;

    /** Compare two language codes.
     */
    [[nodiscard]] constexpr friend auto operator<=>(iso_639 const& lhs, iso_639 const& rhs) noexcept = default;

private:
    /**
     * Encoded as follows:
     * - [15] Individual language, to determine if iso-639-2 or iso-639-3.
     * - [14:10] optional third letter
     * - [9:5] second letter
     * - [4:0] first letter
     *
     * The alphabet for the 5 bit letters are: <nul>abcdefghijklmnopqrstuvwxyz12345.
     */
    uint16_t _v;
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::iso_639> {
    [[nodiscard]] size_t operator()(hi::iso_639 const& rhs) const noexcept
    {
        return rhs.hash();
    }
};
