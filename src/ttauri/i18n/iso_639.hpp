// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cctype>
#include "../assert.hpp"
#include "../strings.hpp"

namespace tt {

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
    char _v0;
    char _v1;
    char _v2;
};

}

