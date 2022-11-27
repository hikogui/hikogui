// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/extent.hpp"
#include "../geometry/margins.hpp"
#include "../geometry/alignment.hpp"
#include "../assert.hpp"
#include "../cast.hpp"
#include <cstdint>
#include <limits>
#include <concepts>

namespace hi { inline namespace v1 {

/** 2D constraints.
 * @ingroup geometry
 *
 * This type holds multiple possible sizes that an 2D object may be.
 * We need multiple sizes in case there is a non-linear relation between the width and height of an object.
 *
 */
struct box_constraints {
    int minimum_width = 0;
    int preferred_width = 0;
    int maximum_width = 0;
    int margin_left = 0;
    int margin_right = 0;
    int padding_left = 0;
    int padding_right = 0;

    int minimum_height = 0;
    int preferred_height = 0;
    int maximum_height = 0;
    int margin_bottom = 0;
    int margin_top = 0;
    int padding_bottom = 0;
    int padding_top = 0;

    hi::alignment alignment = hi::alignment::middle_flush();

    constexpr box_constraints() noexcept = default;
    constexpr box_constraints(box_constraints const&) noexcept = default;
    constexpr box_constraints(box_constraints&&) noexcept = default;
    constexpr box_constraints& operator=(box_constraints const&) noexcept = default;
    constexpr box_constraints& operator=(box_constraints&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(box_constraints const&, box_constraints const&) noexcept = default;

    constexpr box_constraints(
        extent2 minimum,
        extent2 preferred,
        extent2 maximum,
        hi::alignment alignment = hi::alignment{},
        hi::margins margins = hi::margins{},
        hi::margins padding = hi::margins{}) noexcept :
        minimum_width(narrow_cast<int>(minimum.width())),
        preferred_width(narrow_cast<int>(preferred.width())),
        maximum_width(narrow_cast<int>(maximum.width())),
        margin_left(narrow_cast<int>(margins.left())),
        margin_right(narrow_cast<int>(margins.right())),
        padding_left(narrow_cast<int>(padding.left())),
        padding_right(narrow_cast<int>(padding.right())),
        minimum_height(narrow_cast<int>(minimum.height())),
        preferred_height(narrow_cast<int>(preferred.height())),
        maximum_height(narrow_cast<int>(maximum.height())),
        margin_bottom(narrow_cast<int>(margins.bottom())),
        margin_top(narrow_cast<int>(margins.top())),
        padding_bottom(narrow_cast<int>(padding.bottom())),
        padding_top(narrow_cast<int>(padding.top())),
        alignment(alignment)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr extent2 minimum() const noexcept
    {
        return extent2{narrow_cast<float>(minimum_width), narrow_cast<float>(minimum_height)};
    }

    [[nodiscard]] constexpr extent2 preferred() const noexcept
    {
        return extent2{narrow_cast<float>(preferred_width), narrow_cast<float>(preferred_height)};
    }

    [[nodiscard]] constexpr extent2 maximum() const noexcept
    {
        return extent2{narrow_cast<float>(maximum_width), narrow_cast<float>(maximum_height)};
    }

    [[nodiscard]] constexpr hi::margins margins() const noexcept
    {
        return hi::margins{
            narrow_cast<float>(margin_left),
            narrow_cast<float>(margin_bottom),
            narrow_cast<float>(margin_right),
            narrow_cast<float>(margin_top)};
    }

    constexpr box_constraints &set_margins(int rhs) noexcept
    {
        margin_left = rhs;
        margin_bottom = rhs;
        margin_right = rhs;
        margin_top = rhs;
        return *this;
    }

    constexpr box_constraints &set_margins(hi::margins const &rhs) noexcept
    {
        margin_left = narrow_cast<int>(rhs.left());
        margin_bottom = narrow_cast<int>(rhs.bottom());
        margin_right = narrow_cast<int>(rhs.right());
        margin_top = narrow_cast<int>(rhs.top());
        return *this;
    }

    [[nodiscard]] constexpr hi::margins padding() const noexcept
    {
        return hi::margins{
            narrow_cast<float>(padding_left),
            narrow_cast<float>(padding_bottom),
            narrow_cast<float>(padding_right),
            narrow_cast<float>(padding_top)};
    }

    constexpr box_constraints& operator+=(extent2 const& rhs) noexcept
    {
        minimum_width += narrow_cast<int>(rhs.width());
        preferred_width += narrow_cast<int>(rhs.width());
        maximum_width += narrow_cast<int>(rhs.width());
        minimum_height += narrow_cast<int>(rhs.height());
        preferred_height += narrow_cast<int>(rhs.height());
        maximum_height += narrow_cast<int>(rhs.height());
        hi_axiom(holds_invariant());
        return *this;
    }

    [[nodiscard]] constexpr box_constraints operator+(extent2 const& rhs) noexcept
    {
        auto r = *this;
        r += rhs;
        return r;
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return minimum_width <= preferred_width and preferred_width <= maximum_width and minimum_height <= preferred_height and
            preferred_height <= maximum_height;
    }

    [[nodiscard]] friend constexpr box_constraints max(box_constraints const& lhs, extent2 const& rhs) noexcept
    {
        auto r = lhs;
        inplace_max(r.minimum_width, narrow_cast<int>(rhs.width()));
        inplace_max(r.preferred_width, narrow_cast<int>(rhs.width()));
        inplace_max(r.maximum_width, narrow_cast<int>(rhs.width()));
        inplace_max(r.minimum_height, narrow_cast<int>(rhs.height()));
        inplace_max(r.preferred_height, narrow_cast<int>(rhs.height()));
        inplace_max(r.maximum_height, narrow_cast<int>(rhs.height()));
        hi_axiom(r.holds_invariant());
        return r;
    }

    /** Makes a constraint that encompasses both given constraints.
     *
     * @note The alignment is selected from the left-hand-side.
     * @param lhs The left hand side box constraints.
     * @param rhs The right hand side box constraints.
     * @return A box constraints that encompasses both given constraints.
     */
    [[nodiscard]] friend constexpr box_constraints max(box_constraints const& lhs, box_constraints const& rhs) noexcept
    {
        auto r = lhs;
        inplace_max(r.minimum_width, rhs.minimum_width);
        inplace_max(r.preferred_width, rhs.preferred_width);
        inplace_max(r.maximum_width, rhs.maximum_width);
        inplace_max(r.margin_left, rhs.margin_left);
        inplace_max(r.margin_right, rhs.margin_right);
        inplace_max(r.padding_left, rhs.padding_left);
        inplace_max(r.padding_right, rhs.padding_right);

        inplace_max(r.minimum_height, rhs.minimum_height);
        inplace_max(r.preferred_height, rhs.preferred_height);
        inplace_max(r.maximum_height, rhs.maximum_height);
        inplace_max(r.margin_bottom, rhs.margin_bottom);
        inplace_max(r.margin_top, rhs.margin_top);
        inplace_max(r.padding_bottom, rhs.padding_bottom);
        inplace_max(r.padding_top, rhs.padding_top);

        hi_axiom(r.holds_invariant());
        return r;
    }

    template<std::convertible_to<box_constraints>... Args>
    [[nodiscard]] friend constexpr box_constraints
    max(box_constraints const& first, box_constraints const& second, box_constraints const& third, Args const&...args) noexcept
    {
        return max(first, max(second, third, args...));
    }
};

}} // namespace hi::v1
