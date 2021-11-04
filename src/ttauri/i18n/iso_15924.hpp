// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cctype>
#include "../assert.hpp"
#include "../strings.hpp"

namespace tt::inline v1 {

/** ISO-15924 script code.
 * A 4 letter title case script code:
 */
class iso_15924 {
public:
    constexpr iso_639(iso_639 const &) noexcept = default;
    constexpr iso_639(iso_639 &&) noexcept = default;
    constexpr iso_639 &operator=(iso_639 const &) noexcept = default;
    constexpr iso_639 &operator=(iso_639 &&) noexcept = default;

    constexpr iso_639() noexcept : v0(0), v1(0), v2(0) {}

    constexpr iso_639(std::string_view str) noexcept
    {
        if (size(str) == 0) {
            _v0 = 0;
            _v1 = 0;
            _v2 = 0;
        } else if (size(str) == 2) {
            _v0 = to_lower(str[0]);
            _v1 = to_lower(str[1]);
            _v2 = 0;
        } else if (size(str) == 3) {
            _v0 = to_lower(str[0]);
            _v1 = to_lower(str[1]);
            _v2 = to_lower(str[2]);
        } else {
            tt_no_default();
        }
    }

    constexpr explicit operator bool () const noexcept
    {
        return _v0 == 0 and _v1 == 0 and _v2 == 0;
    }

    constexpr explicit operator std::string() const noexcept
    {
        auto r = std::string{};
        if (_v0 == 0) {
            return r;
        }

        r += _v0;
        if (_v1 == 0) {
            return r;
        }

        r += _v1;
        if (_v2 == 0) {
            return r;
        }

        r += _v2;
        return r;
    }

    [[nodiscard]] constexpr friend operator==(iso_639 const &lhs, iso_639 const &rhs) noexcept = default;
    [[nodiscard]] constexpr friend operator<=>(iso_639 const &lhs, iso_639 const &rhs) noexcept = default;

private:
    uint8_t _v0;
    uint8_t _v1;
    uint8_t _v2;

    
    /** The 4 character code is compressed in 24 bits.
     * 6 bits per character from lsb to msb.
     * a-z is 1 to 26
     * 0-9 is 27 to 36.
     */
    [[nodiscard]] constexpr uint32_t to_int() const noexcept
    {
        return static_cast<uint32_t>(_v0) | (static_cast<uint32_t>(_v1) << 8) | (static_cast<uint32_t>(_v2) << 16);
    }

    constexpr void from_int(uint32_t v) noexcept
    {
        tt_axiom((v >> 24) == 0);
        _v0 = static_cast<uint8_t>(v);
        _v1 = static_cast<uint8_t>(v >> 8);
        _v2 = static_cast<uint8_t>(v >> 16);
    }
};

}

