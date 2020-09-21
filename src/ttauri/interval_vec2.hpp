// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "vec.hpp"

namespace tt {

/** A 2D vector using interval arithmetic.
 * This class can be used to work calculate 2D minimum/maximum size.
 *
 * For proper interval arithmetic the floating point rounding direction
 * should be set to +infinity.
 */
class interval_vec2 {
public:
    [[nodiscard]] interval_vec2(interval_vec2 const &other) noexcept = default;
    [[nodiscard]] interval_vec2(interval_vec2 &&other) noexcept = default;
    [[nodiscard]] interval_vec2 &operator=(interval_vec2 const &other) noexcept = default;
    [[nodiscard]] interval_vec2 &operator=(interval_vec2 &&other) noexcept = default;

    [[nodiscard]] interval_vec2(vec min, vec max) noexcept : value(-min + max._00xy())
    {
        tt_assume(min.z() == 0.0f && min.w() == 0.0f);
        tt_assume(max.z() == 0.0f && max.w() == 0.0f);
    }

    [[nodiscard]] interval_vec2() noexcept :
        interval_vec2(
            vec{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()},
            vec{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()})
    {
    }

    [[nodiscard]] interval_vec2(vec other) noexcept : interval_vec2(other, other) {}

    [[nodiscard]] interval_vec2(float x, float y) noexcept : interval_vec2(vec{x, y}) {}

    [[nodiscard]] static interval_vec2 make_minimum(vec other) noexcept
    {
        return {other, vec{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
    }

    [[nodiscard]] static interval_vec2 make_minimum(float x, float y) noexcept
    {
        return make_minimum(vec{x, y});
    }

    [[nodiscard]] static interval_vec2 make_maximum(vec other) noexcept
    {
        return {vec{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()}, other};
    }

    [[nodiscard]] static interval_vec2 make_maximum(float x, float y) noexcept
    {
        return make_maximum(vec{x, y});
    }

    [[nodiscard]] static interval_vec2 make_zero_to_maximum(vec other) noexcept
    {
        return {vec{}, other};
    }

    [[nodiscard]] static interval_vec2 make_zero_to_maximum(float x, float y) noexcept
    {
        return make_zero_to_maximum(vec{x, y});
    }

    [[nodiscard]] vec minimum() const noexcept
    {
        return (-value).xy00();
    }

    [[nodiscard]] vec maximum() const noexcept
    {
        return value.zw00();
    }

    [[nodiscard]] interval_vec2 x0() const noexcept
    {
        return make(value.x0z0());
    }

    [[nodiscard]] interval_vec2 _0y() const noexcept
    {
        return make(value._0y0w());
    }

    [[nodiscard]] interval_vec2 &operator+=(interval_vec2 const &rhs) noexcept
    {
        value += rhs.value;
        return *this;
    }

    [[nodiscard]] interval_vec2 &operator-=(interval_vec2 const &rhs) noexcept
    {
        value -= rhs.value.yxwz();
        return *this;
    }

    [[nodiscard]] friend interval_vec2 operator+(interval_vec2 const &lhs, interval_vec2 const &rhs) noexcept
    {
        return make(lhs.value + rhs.value);
    }

    [[nodiscard]] friend interval_vec2 operator-(interval_vec2 const &lhs, interval_vec2 const &rhs) noexcept
    {
        return make(lhs.value - rhs.value.yxwz());
    }

    /** Intersect two intervals.
     * The returned interval only includes the part that overlap.
     * 
     * It is undefined behavior if the given intervals do not overlap.
     */
    [[nodiscard]] friend interval_vec2 intersect(interval_vec2 const &lhs, interval_vec2 const &rhs) noexcept
    {
        return make(min(lhs.value, rhs.value));
    }

    /** Merge two intervals.
     * The returned interval includes both intervals fully.
     */
    [[nodiscard]] friend interval_vec2 merge(interval_vec2 const &lhs, interval_vec2 const &rhs) noexcept
    {
        return make(max(lhs.value, rhs.value));
    }

private:
    [[nodiscard]] static interval_vec2 make(vec other) noexcept
    {
        interval_vec2 r;
        r.value = other;
        return r;
    }

    /** The value as (-x_min, -y_min, y_max, x_max)
     */
    vec value;
};

} // namespace tt