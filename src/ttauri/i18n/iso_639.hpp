// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cctype>
#include "../assert.hpp"
#include "../strings.hpp"

namespace tt::inline v1 {

/** ISO-639 language code.
 * A 2 or 3 lower case language code selected from the following iso standards in this order:
 *  1. ISO 639-1 (2002)
 *  2. ISO 639-2 (1998)
 *  3. ISO 639-3 (2007)
 *  4. ISO 639-5 (2008),
 */
class iso_639 {
public:
    constexpr iso_639(iso_639 const &) noexcept = default;
    constexpr iso_639(iso_639 &&) noexcept = default;
    constexpr iso_639 &operator=(iso_639 const &) noexcept = default;
    constexpr iso_639 &operator=(iso_639 &&) noexcept = default;

    constexpr iso_639() noexcept : _v(0) {}

    constexpr iso_639(std::string_view str) noexcept : _v(0)
    {
        tt_axiom(str.size() == 2 or str.size() == 3);
        set<0>(*this, str[0]);
        set<1>(*this, str[1]);
        if (str.size() == 3) {
            set<2>(*this, str[2]);
        }
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        auto tmp = _v & 0x7fff;
        // clang-format off
        return
            tmp == 0 ? 0 :
            tmp <= 0x1f ? 1 :
            tmp <= 0x3ff ? 2 :
            3;
        // clang-format on
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return size() == 0;
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }


    constexpr explicit operator std::string() const noexcept
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

    [[nodiscard]] constexpr friend bool operator==(iso_639 const &lhs, iso_639 const &rhs) noexcept = default;

    template<std::size_t I>
    constexpr friend iso_639 &set(iso_639 &rhs, char c) noexcept
    {
        tt_axiom((c >= 'a' and c <= 'z') or (c >= '1' and c <= '5'));

        // clang-format off
        uint16_t x =
            (c >= 'a' and c <= 'z') ? c - 'a' + 1 :
            (c >= '1' and c <= '5') ? c - '1' + 27 :
            0;
        // clang-format on

        tt_axiom(x <= 0x1f);
        constexpr auto shift = I * 5;
        rhs._v &= ~(0x1f << shift);
        rhs._v |= x << shift;
        return rhs;
    }

    template<std::size_t I>
    [[nodiscard]] constexpr friend char get(iso_639 const &rhs) noexcept
    {
        constexpr auto shift = I * 5;
        auto x = (rhs._v >> shift) & 0x1f;
        if (x == 0) {
            return 0;
        } else if (x <= 26) {
            return 'a' + static_cast<char>(x - 1);
        } else {
            return '1' + static_cast<char>(x - 27);
        }
    }

    /** Get the default script if this language.
     */
    [[nodiscard]] iso_15924 default_script() const noexcept;

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

} // namespace tt::inline v1
