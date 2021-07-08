// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cctype>
#include "../assert.hpp"
#include "../strings.hpp"

namespace tt {

/** ISO-3166 country code.
 * ISO-3166-1 alpha-2 (upper-case)
 * nul, nul means empty.
 */
class iso_3166 {
public:
    constexpr iso_3166(iso_3166 const &) noexcept = default;
    constexpr iso_3166(iso_3166 &&) noexcept = default;
    constexpr iso_3166 &operator=(iso_3166 const &) noexcept = default;
    constexpr iso_3166 &operator=(iso_3166 &&) noexcept = default;

    constexpr iso_3166() noexcept : v0(0), v1(0) {}

    constexpr iso_3166(std::string_view str) noexcept
    {
        if (std::size(str) == 0) {
            _v0 = 0;
            _v1 = 0;
        } else if (std::size(str) == 2) {
            _v0 = to_upper(str[0]);
            _v1 = to_upper(str[1]);
        } else {
            tt_no_default();
        }
    }

    constexpr explicit operator bool () const noexcept
    {
        return _v0 == 0 and _v1 == 0;
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
        return r;
    }

    [[nodiscard]] constexpr friend operator==(iso_3166 const &lhs, iso_3166 const &rhs) noexcept = default;
    [[nodiscard]] constexpr friend operator<=>(iso_3166 const &lhs, iso_3166 const &rhs) noexcept = default;

private:
    char _v0;
    char _v1;
};

}

