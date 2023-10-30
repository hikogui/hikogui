// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "cast.hpp"
#include <ratio>
#include <concepts>
#include <compare>

hi_export_module(hikogui.utility.units);

hi_export namespace hi { inline namespace v1 {

template<typename Tag, typename T, typename Ratio = std::ratio<1>>
class unit {
public:
    using value_type = T;
    using ratio = Ratio;

    constexpr unit(unit const&) noexcept = default;
    constexpr unit(unit&&) noexcept = default;
    constexpr unit& operator=(unit const&) noexcept = default;
    constexpr unit& operator=(unit&&) noexcept = default;

    constexpr explicit unit(value_type value) noexcept : _value(value) {}

    template<typename OtherT, typename OtherRatio>
    constexpr explicit unit(unit<Tag, OtherT, OtherRatio> const& other) noexcept
        requires(not std::is_same_v<unit<Tag, OtherT, OtherRatio>, unit>)
    {
        using conversion = std::ratio_divide<Ratio, OtherRatio>;

        auto tmp = wide_cast<std::common_type_t<T, OtherT>>(other.count());
        tmp *= conversion::den;
        tmp /= conversion::num;
        _value = narrow_cast<T>(tmp);
    }

    template<typename OtherT, typename OtherRatio>
    constexpr unit& operator=(unit<Tag, OtherT, OtherRatio> const& other) noexcept
        requires(not std::is_same_v<unit<Tag, OtherT, OtherRatio>, unit>)
    {
        using conversion = std::ratio_divide<Ratio, OtherRatio>;

        auto tmp = wide_cast<std::common_type_t<T, OtherT>>(other.count());
        tmp *= conversion::den;
        tmp /= conversion::num;
        _value = narrow_cast<T>(tmp);
        return *this;
    }

    [[nodiscard]] constexpr value_type count() const noexcept
    {
        return _value;
    }

    [[nodiscard]] constexpr unit operator*(value_type const& rhs) const noexcept
    {
        return unit{count() * rhs};
    }
    
    [[nodiscard]] constexpr unit operator/(value_type const& rhs) const noexcept
    {
        return unit{count() / rhs};
    }

    [[nodiscard]] constexpr unit& operator+=(unit const& rhs) noexcept
    {
        _value += rhs.count();
        return *this;
    }

    [[nodiscard]] constexpr unit& operator-=(unit const& rhs) noexcept
    {
        _value -= rhs.count();
        return *this;
    }

    [[nodiscard]] constexpr unit& operator*=(value_type const& rhs) noexcept
    {
        _value *= rhs;
        return *this;
    }

    [[nodiscard]] constexpr unit& operator/=(value_type const& rhs) noexcept
    {
        _value /= rhs;
        return *this;
    }

private:
    value_type _value;
};

}} // namespace hi::v1

template<typename Tag, typename T1, typename Ratio1, typename T2, typename Ratio2>
struct std::common_type<hi::unit<Tag, T1, Ratio1>, hi::unit<Tag, T2, Ratio2>> {
    // clang-format off
    using type = hi::unit<
        Tag,
        std::common_type_t<T1, T2>,
        std::conditional_t<std::ratio_less_v<Ratio1, Ratio2>, Ratio1, Ratio2>>;
    // clang-format on
};

hi_export namespace hi { inline namespace v1 {

template<typename Tag, typename T1, typename Ratio1, typename T2, typename Ratio2>
[[nodiscard]] constexpr bool operator==(unit<Tag, T1, Ratio1> const& lhs, unit<Tag, T2, Ratio2> const& rhs) noexcept
{
    using common_type = std::common_type_t<unit<Tag, T1, Ratio1>, unit<Tag, T2, Ratio2>>;

    hilet lhs_ = common_type{lhs};
    hilet rhs_ = common_type{rhs};

    return lhs_.count() == rhs_.count();
}

template<typename Tag, typename T1, typename Ratio1, typename T2, typename Ratio2>
[[nodiscard]] constexpr auto operator<=>(unit<Tag, T1, Ratio1> const& lhs, unit<Tag, T2, Ratio2> const& rhs) noexcept
{
    using common_type = std::common_type_t<unit<Tag, T1, Ratio1>, unit<Tag, T2, Ratio2>>;

    hilet lhs_ = common_type{lhs};
    hilet rhs_ = common_type{rhs};

    return lhs_.count() <=> rhs_.count();
}

template<typename Tag, typename T1, typename Ratio1, typename T2, typename Ratio2>
[[nodiscard]] constexpr auto operator+(unit<Tag, T1, Ratio1> const& lhs, unit<Tag, T2, Ratio2> const& rhs) noexcept
{
    using common_type = std::common_type_t<unit<Tag, T1, Ratio1>, unit<Tag, T2, Ratio2>>;

    hilet lhs_ = common_type{lhs};
    hilet rhs_ = common_type{rhs};

    return common_type{lhs_.count() + rhs_.count()};
}

template<typename Tag, typename T1, typename Ratio1, typename T2, typename Ratio2>
[[nodiscard]] constexpr auto operator-(unit<Tag, T1, Ratio1> const& lhs, unit<Tag, T2, Ratio2> const& rhs) noexcept
{
    using common_type = std::common_type_t<unit<Tag, T1, Ratio1>, unit<Tag, T2, Ratio2>>;

    hilet lhs_ = common_type{lhs};
    hilet rhs_ = common_type{rhs};

    return common_type{lhs_.count() - rhs_.count()};
}

template<typename Tag, typename T1, typename Ratio1, typename T2, typename Ratio2>
[[nodiscard]] constexpr auto operator/(unit<Tag, T1, Ratio1> const& lhs, unit<Tag, T2, Ratio2> const& rhs) noexcept
{
    using common_type = std::common_type_t<unit<Tag, T1, Ratio1>, unit<Tag, T2, Ratio2>>;

    hilet lhs_ = common_type{lhs};
    hilet rhs_ = common_type{rhs};

    return lhs_.count() / rhs_.count();
}


struct si_length_tag {};
struct px_length_tag {};
struct em_length_tag {};

using kilometers = unit<si_length_tag, double, std::kilo>;
using meters = unit<si_length_tag, double>;
using centimeters = unit<si_length_tag, double, std::centi>;
using decimeters = unit<si_length_tag, double, std::deci>;
using millimeters = unit<si_length_tag, double, std::milli>;

/** Points: 1/72 inch.
 */
using points = unit<si_length_tag, double, std::ratio<254, 720'000>::type>;

/** Inch: 254 mm.
 */
using inches = unit<si_length_tag, double, std::ratio<254, 10'000>::type>;
using feet = unit<si_length_tag, double, std::ratio<3'048, 10'000>::type>;
using yards = unit<si_length_tag, double, std::ratio<9'144, 10'000>::type>;
using miles = unit<si_length_tag, double, std::ratio<16'093'440, 10'000>::type>;

/** Device Independent Pixels: 1/96 inch.
 */
using dips = unit<si_length_tag, double, std::ratio<254, 960'000>::type>;

/** A physical pixel on a display.
 */
using pixels = unit<px_length_tag, double>;

/** Em-quad: A font's line-height.
 */
using em_quads = unit<em_length_tag, double>;

}} // namespace hi::v1
