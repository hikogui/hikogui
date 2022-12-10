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

    hi::alignment alignment = hi::alignment{};

    constexpr box_constraints() noexcept = default;
    constexpr box_constraints(box_constraints const&) noexcept = default;
    constexpr box_constraints(box_constraints&&) noexcept = default;
    constexpr box_constraints& operator=(box_constraints const&) noexcept = default;
    constexpr box_constraints& operator=(box_constraints&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(box_constraints const&, box_constraints const&) noexcept = default;

    constexpr box_constraints(
        int minimum_width,
        int minimum_height,
        int preferred_width,
        int preferred_height,
        int maximum_width,
        int maximum_height) noexcept :
        minimum_width(minimum_width),
        minimum_height(minimum_height),
        preferred_width(preferred_width),
        preferred_height(preferred_height),
        maximum_width(maximum_width),
        maximum_height(maximum_height)
    {
    }

    [[deprecated]] constexpr box_constraints(
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

    [[deprecated]] [[nodiscard]] constexpr extent2 minimum() const noexcept
    {
        return extent2{narrow_cast<float>(minimum_width), narrow_cast<float>(minimum_height)};
    }

    [[deprecated]] [[nodiscard]] constexpr extent2 preferred() const noexcept
    {
        return extent2{narrow_cast<float>(preferred_width), narrow_cast<float>(preferred_height)};
    }

    [[deprecated]] [[nodiscard]] constexpr extent2 maximum() const noexcept
    {
        return extent2{narrow_cast<float>(maximum_width), narrow_cast<float>(maximum_height)};
    }

    constexpr box_constraints& set_margins(int rhs) noexcept
    {
        margin_left = rhs;
        margin_bottom = rhs;
        margin_right = rhs;
        margin_top = rhs;
        return *this;
    }

    constexpr box_constraints& set_padding(int rhs) noexcept
    {
        padding_left = rhs;
        padding_bottom = rhs;
        padding_right = rhs;
        padding_top = rhs;
        return *this;
    }

    [[nodiscard]] constexpr box_constraints internalize_margins() const noexcept
    {
        auto r = *this;
        r.padding_left += r.margin_left;
        r.padding_right += r.margin_right;
        r.padding_top += r.margin_top;
        r.padding_bottom += r.margin_bottom;

        r.minimum_width += r.margin_left + r.margin_right;
        r.preferred_width += r.margin_left + r.margin_right;
        r.maximum_width += r.margin_left + r.margin_right;

        r.minimum_height += r.margin_bottom + r.margin_top;
        r.preferred_height += r.margin_bottom + r.margin_top;
        r.maximum_height += r.margin_bottom + r.margin_top;

        r.margin_left = 0;
        r.margin_right = 0;
        r.margin_bottom = 0;
        r.margin_top = 0;
        hi_axiom(r.holds_invariant());
        return r;
    }

    [[nodiscard]] constexpr box_constraints constrain(int min_width, int min_height, int max_width, int max_height) const noexcept
    {
        hi_assert(min_width <= max_width);
        hi_assert(min_height <= max_height);

        auto r = *this;

        inplace_max(r.minimum_width, min_width);
        inplace_max(r.minimum_height, min_height);
        inplace_min(r.maximum_width, max_width);
        inplace_min(r.maximum_height, max_height);

        inplace_max(r.preferred_width, r.minimum_width);
        inplace_max(r.preferred_height, r.minimum_height);
        inplace_max(r.maximum_width, r.preferred_width);
        inplace_max(r.maximum_height, r.preferred_height);
        hi_axiom(r.holds_invariant());
        return r;
    }

    [[deprecated]] [[nodiscard]] constexpr hi::margins margins() const noexcept
    {
        return hi::margins{
            narrow_cast<float>(margin_left),
            narrow_cast<float>(margin_bottom),
            narrow_cast<float>(margin_right),
            narrow_cast<float>(margin_top)};
    }

    [[deprecated]] constexpr box_constraints& set_margins(hi::margins const& rhs) noexcept
    {
        margin_left = narrow_cast<int>(rhs.left());
        margin_bottom = narrow_cast<int>(rhs.bottom());
        margin_right = narrow_cast<int>(rhs.right());
        margin_top = narrow_cast<int>(rhs.top());
        return *this;
    }

    [[deprecated]] [[nodiscard]] constexpr hi::margins padding() const noexcept
    {
        return hi::margins{
            narrow_cast<float>(padding_left),
            narrow_cast<float>(padding_bottom),
            narrow_cast<float>(padding_right),
            narrow_cast<float>(padding_top)};
    }

    [[deprecated]] constexpr box_constraints& operator+=(extent2 const& rhs) noexcept
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

    [[deprecated]] [[nodiscard]] constexpr box_constraints operator+(extent2 const& rhs) noexcept
    {
        auto r = *this;
        r += rhs;

        hi_axiom(r.holds_invariant());
        return r;
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        if (alignment == horizontal_alignment::flush or alignment == horizontal_alignment::justified) {
            return false;
        }
        if (minimum_width > preferred_width or preferred_width > maximum_width) {
            return false;
        }
        if (minimum_height > preferred_height or preferred_height > maximum_height) {
            return false;
        }
        return true;
    }

    [[deprecated]] [[nodiscard]] friend constexpr box_constraints max(box_constraints const& lhs, extent2 const& rhs) noexcept
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

    /** Get the maximum integer that should be used in constraints.
     *
     * @return 16777216; The largest integer that can be represented perfectly.
     */
    [[nodiscard]] constexpr static int max_int() noexcept
    {
        return 16777216;
    }
};

}} // namespace hi::v1
