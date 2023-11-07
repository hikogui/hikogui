// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <compare>
#include <cctype>
#include <string_view>
#include <string>
#include <format>

hi_export_module(hikogui.i18n.iso_3166 : intf);

hi_export namespace hi::inline v1 {

/** ISO-3166 country code.
 */
hi_export class iso_3166 {
public:
    constexpr iso_3166(iso_3166 const&) noexcept = default;
    constexpr iso_3166(iso_3166&&) noexcept = default;
    constexpr iso_3166& operator=(iso_3166 const&) noexcept = default;
    constexpr iso_3166& operator=(iso_3166&&) noexcept = default;

    constexpr iso_3166() noexcept : _v(0) {}

    constexpr iso_3166(uint16_t number) : _v(number)
    {
        hi_check(number <= 999, "ISO-3166 number must be between 0 and 999, got {}", number);
    }

    constexpr iso_3166(std::string_view str);

    constexpr iso_3166(intrinsic_t, uint16_t v) noexcept : _v(v)
    {
        hi_axiom(_v < 1000);
    }

    [[nodiscard]] constexpr uint16_t const& intrinsic() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr uint16_t& intrinsic() noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _v == 0;
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr uint16_t number() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr std::string code2() const noexcept;
    [[nodiscard]] constexpr std::string code3() const noexcept;

    [[nodiscard]] constexpr friend std::string to_string(iso_3166 const &rhs) noexcept
    {
        return rhs.code2();
    }

    [[nodiscard]] constexpr friend bool operator==(iso_3166 const& lhs, iso_3166 const& rhs) noexcept = default;
    [[nodiscard]] constexpr friend auto operator<=>(iso_3166 const& lhs, iso_3166 const& rhs) noexcept = default;

    /** Check if rhs matches with lhs.
     *
     * @param lhs The country or wild-card.
     * @param rhs The country.
     * @return True when lhs is a wild-card or when lhs and rhs are equal.
     */
    [[nodiscard]] constexpr friend bool matches(iso_3166 const& lhs, iso_3166 const& rhs) noexcept
    {
        return lhs.empty() or lhs == rhs;
    }

private:
    uint16_t _v;
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::iso_3166> {
    [[nodiscard]] size_t operator()(hi::iso_3166 const& rhs) const noexcept
    {
        return std::hash<uint16_t>{}(rhs.number());
    }
};
