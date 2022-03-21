// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cctype>
#include "../assert.hpp"
#include "../strings.hpp"

namespace tt::inline v1 {

/** ISO-3166 country code.
 */
class iso_3166 {
public:
    constexpr iso_3166(iso_3166 const &) noexcept = default;
    constexpr iso_3166(iso_3166 &&) noexcept = default;
    constexpr iso_3166 &operator=(iso_3166 const &) noexcept = default;
    constexpr iso_3166 &operator=(iso_3166 &&) noexcept = default;

    constexpr iso_3166() noexcept : _v(999) {}

    iso_3166(std::string_view str);

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _v == 999;
    }
    
    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr uint16_t number() const noexcept
    {
        return _v;
    }

    [[nodiscard]] std::string_view code2() const noexcept;
    [[nodiscard]] std::string_view code3() const noexcept;

    [[nodiscard]] constexpr friend bool operator==(iso_3166 const &lhs, iso_3166 const &rhs) noexcept = default;
    [[nodiscard]] constexpr friend auto operator<=>(iso_3166 const &lhs, iso_3166 const &rhs) noexcept = default;

private:
    uint16_t _v;
};

} // namespace tt::inline v1

template<>
struct std::hash<tt::iso_3166> {
    [[nodiscard]] size_t operator()(tt::iso_3166 const& rhs) const noexcept
    {
        return std::hash<uint16_t>{}(rhs.number());
    }
};
