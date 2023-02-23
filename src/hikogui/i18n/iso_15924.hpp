// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../unicode/unicode_script.hpp"
#include "../unicode/unicode_bidi_class.hpp"
#include <string_view>
#include <cstdint>
#include <format>

namespace hi::inline v1 {

/** ISO-15924 script code.
 * A 4 letter title case script code:
 */
class iso_15924 {
public:
    constexpr iso_15924() noexcept : _v(0) {}
    constexpr iso_15924(iso_15924 const&) noexcept = default;
    constexpr iso_15924(iso_15924&&) noexcept = default;
    constexpr iso_15924& operator=(iso_15924 const&) noexcept = default;
    constexpr iso_15924& operator=(iso_15924&&) noexcept = default;

    constexpr iso_15924(uint16_t number) : _v(number)
    {
        hi_check(number <= 999, "ISO-15924 number must be between 0 and 999, got {}", number);
    }

    iso_15924(unicode_script const& script) noexcept;
    explicit operator unicode_script() const noexcept;

    iso_15924(std::string_view code4);

    constexpr iso_15924(intrinsic_t, uint16_t v) noexcept : _v(v)
    {
        hi_axiom(_v <= 999);
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

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Get the iso-15924 numeric value.
     */
    [[nodiscard]] constexpr uint16_t number() const noexcept
    {
        return _v;
    }

    /** Get the iso-15924 4-letter code.
     */
    [[nodiscard]] std::string_view code4() const noexcept;

    /** Get the 4-letter code used by open-type.
     */
    [[nodiscard]] std::string_view code4_open_type() const noexcept;

    [[nodiscard]] unicode_bidi_class writing_direction() const noexcept;

    [[nodiscard]] constexpr friend bool operator==(iso_15924 const& lhs, iso_15924 const& rhs) noexcept = default;

    /** Check if rhs matches with lhs.
     *
     * @param lhs The script or wild-card.
     * @param rhs The script.
     * @return True when lhs is a wild-card or when lhs and rhs are equal.
     */
    [[nodiscard]] constexpr friend bool matches(iso_15924 const& lhs, iso_15924 const& rhs) noexcept
    {
        return lhs.empty() or lhs == rhs;
    }

private:
    uint16_t _v;
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::iso_15924> {
    [[nodiscard]] size_t operator()(hi::iso_15924 const& rhs) const noexcept
    {
        return std::hash<uint16_t>{}(rhs.number());
    }
};
