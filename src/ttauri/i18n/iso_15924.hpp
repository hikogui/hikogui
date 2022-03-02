// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../assert.hpp"
#include "../strings.hpp"
#include "../unicode/unicode_script.hpp"
#include <string_view>
#include <cctype>

namespace tt::inline v1 {

/** ISO-15924 script code.
 * A 4 letter title case script code:
 */
class iso_15924 {
public:
    constexpr iso_15924() noexcept : _v(999) {}
    constexpr iso_15924(iso_15924 const &) noexcept = default;
    constexpr iso_15924(iso_15924 &&) noexcept = default;
    constexpr iso_15924 &operator=(iso_15924 const &) noexcept = default;
    constexpr iso_15924 &operator=(iso_15924 &&) noexcept = default;

    constexpr iso_15924(uint16_t number) noexcept : _v(number) {}
    iso_15924(unicode_script const &script) noexcept;


    [[nodiscard]] char const *code() const noexcept;
    [[nodiscard]] char const *open_type() const noexcept;

    [[nodiscard]] constexpr friend bool operator==(iso_15924 const &lhs, iso_15924 const &rhs) noexcept = default;
    [[nodiscard]] constexpr friend auto operator<=>(iso_15924 const &lhs, iso_15924 const &rhs) noexcept = default;

private:
    uint16_t _v;
};

} // namespace tt::inline v1
