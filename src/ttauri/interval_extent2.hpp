// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "interval.hpp"
#include "numeric_array.hpp"
#include "geometry/extent.hpp"

namespace tt {

/** A 2D vector using interval arithmetic.
 * This class can be used to work calculate 2D minimum/maximum size.
 *
 * For proper interval arithmetic the floating point rounding direction
 * should be set to +infinity.
 */
class interval_extent2 {
public:
    [[nodiscard]] interval_extent2(interval_extent2 const &other) noexcept = default;
    [[nodiscard]] interval_extent2(interval_extent2 &&other) noexcept = default;
    [[nodiscard]] interval_extent2 &operator=(interval_extent2 const &other) noexcept = default;
    [[nodiscard]] interval_extent2 &operator=(interval_extent2 &&other) noexcept = default;

    [[nodiscard]] interval_extent2(extent2 min, extent2 max) noexcept : value(-static_cast<f32x4>(min) + static_cast<f32x4>(max)._00xy())
    {
        tt_axiom(min.is_valid() && max.is_valid());
        tt_axiom(min.width() <= max.width());
        tt_axiom(min.height() <= max.height());
    }

    [[nodiscard]] interval_extent2(finterval x, finterval y) noexcept :
        interval_extent2(extent2{x.minimum(), y.minimum()}, extent2{x.maximum(), y.maximum()})
    {
    }

    [[nodiscard]] interval_extent2() noexcept :
        interval_extent2(
            extent2{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()},
            extent2{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()})
    {
    }

    [[nodiscard]] interval_extent2(extent2 other) noexcept : interval_extent2(other, other) {}

    [[nodiscard]] interval_extent2(float x, float y) noexcept : interval_extent2(extent2{x, y}) {}

    [[nodiscard]] static interval_extent2 make_minimum(extent2 other) noexcept
    {
        return {other, extent2{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
    }

    [[nodiscard]] static interval_extent2 make_minimum(float x, float y) noexcept
    {
        return make_minimum(extent2{x, y});
    }

    [[nodiscard]] static interval_extent2 make_maximum(extent2 other) noexcept
    {
        return {extent2{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()}, other};
    }

    [[nodiscard]] static interval_extent2 make_maximum(float x, float y) noexcept
    {
        return make_maximum(extent2{x, y});
    }

    [[nodiscard]] static interval_extent2 make_zero_to_maximum(extent2 other) noexcept
    {
        return {extent2{}, other};
    }

    [[nodiscard]] static interval_extent2 make_zero_to_maximum(float x, float y) noexcept
    {
        return make_zero_to_maximum(extent2{x, y});
    }

    [[nodiscard]] extent2 minimum() const noexcept
    {
        return extent2{(-value).xy00()};
    }

    [[nodiscard]] extent2 maximum() const noexcept
    {
        return extent2{value.zw00()};
    }

    [[nodiscard]] finterval width() const noexcept
    {
        return finterval(-value.x(), value.z());
    }

    [[nodiscard]] finterval height() const noexcept
    {
        return finterval(-value.y(), value.w());
    }

    [[nodiscard]] interval_extent2 &operator+=(interval_extent2 const &rhs) noexcept
    {
        value += rhs.value;
        return *this;
    }

    [[nodiscard]] interval_extent2 &operator-=(interval_extent2 const &rhs) noexcept
    {
        value -= rhs.value.yxwz();
        return *this;
    }

    [[nodiscard]] friend interval_extent2 operator+(interval_extent2 const &lhs, interval_extent2 const &rhs) noexcept
    {
        return make(lhs.value + rhs.value);
    }

    [[nodiscard]] friend interval_extent2 operator-(interval_extent2 const &lhs, interval_extent2 const &rhs) noexcept
    {
        return make(lhs.value - rhs.value.yxwz());
    }

    [[nodiscard]] friend bool operator==(interval_extent2 const &lhs, interval_extent2 const &rhs) noexcept
    {
        return lhs.value == rhs.value;
    }

    /** Check if lhs.x or lhs.y is smaller then rhs.minimum.
     */
    [[nodiscard]] friend bool operator<<(extent2 const &lhs, interval_extent2 const &rhs) noexcept
    {
        return lhs.width() < rhs.minimum().width() || lhs.height() < rhs.minimum().height();
    }

    /** Check if lhs.x or lhs.y is larger then rhs.maximum.
     */
    [[nodiscard]] friend bool operator>>(extent2 const &lhs, interval_extent2 const &rhs) noexcept
    {
        return lhs.width() > rhs.maximum().width() || lhs.height() > rhs.maximum().height();
    }
    [[nodiscard]] friend std::string to_string(interval_extent2 const &rhs) noexcept
    {
        return fmt::format("({}:{}, {}:{})", rhs.value.x(), rhs.value.z(), rhs.value.y(), rhs.value.w());
    }

    friend std::ostream &operator<<(std::ostream &lhs, interval_extent2 const &rhs)
    {
        return lhs << to_string(rhs);
    }

    /** Intersect two intervals.
     * The returned interval only includes the part that overlap.
     * 
     * It is undefined behavior if the given intervals do not overlap.
     */
    [[nodiscard]] friend interval_extent2 intersect(interval_extent2 const &lhs, interval_extent2 const &rhs) noexcept
    {
        return make(min(lhs.value, rhs.value));
    }

    /** Merge two intervals.
     * The returned interval includes both intervals fully.
     */
    [[nodiscard]] friend interval_extent2 merge(interval_extent2 const &lhs, interval_extent2 const &rhs) noexcept
    {
        return make(max(lhs.value, rhs.value));
    }

    /** Get the maximum interval of both operants.
     */
    [[nodiscard]] friend interval_extent2 min(interval_extent2 const &lhs, interval_extent2 const &rhs) noexcept
    {
        ttlet tmp_max = max(lhs.value, rhs.value);
        ttlet tmp_min = min(lhs.value, rhs.value);
        return make(tmp_max.xy00() + tmp_min._00zw());
    }

    /** Get the minimum interval of both operants.
     */
    [[nodiscard]] friend interval_extent2 max(interval_extent2 const &lhs, interval_extent2 const &rhs) noexcept
    {
        ttlet tmp_max = max(lhs.value, rhs.value);
        ttlet tmp_min = min(lhs.value, rhs.value);
        return make(tmp_min.xy00() + tmp_max._00zw());
    }

private:
    [[nodiscard]] static interval_extent2 make(f32x4 other) noexcept
    {
        tt_axiom(-other.x() <= other.z());
        tt_axiom(-other.y() <= other.w());

        interval_extent2 r;
        r.value = other;
        return r;
    }

    /** The value as (-x_min, -y_min, y_max, x_max)
     */
    f32x4 value;
};

} // namespace tt