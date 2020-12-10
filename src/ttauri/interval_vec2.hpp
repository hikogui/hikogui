// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "interval.hpp"
#include "numeric_array.hpp"

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

    [[nodiscard]] interval_vec2(f32x4 min, f32x4 max) noexcept : value(-min + max._00xy())
    {
        tt_axiom(min.z() == 0.0f && min.w() == 0.0f);
        tt_axiom(max.z() == 0.0f && max.w() == 0.0f);
        tt_axiom(min.x() <= max.x());
        tt_axiom(min.y() <= max.y());
    }

    [[nodiscard]] interval_vec2(finterval x, finterval y) noexcept :
        interval_vec2(f32x4{x.minimum(), y.minimum()}, f32x4{x.maximum(), y.maximum()}) {}

    [[nodiscard]] interval_vec2() noexcept :
        interval_vec2(
            f32x4{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()},
            f32x4{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()})
    {
    }

    [[nodiscard]] interval_vec2(f32x4 other) noexcept : interval_vec2(other, other) {}

    [[nodiscard]] interval_vec2(float x, float y) noexcept : interval_vec2(f32x4{x, y}) {}

    [[nodiscard]] static interval_vec2 make_minimum(f32x4 other) noexcept
    {
        return {other, f32x4{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
    }

    [[nodiscard]] static interval_vec2 make_minimum(float x, float y) noexcept
    {
        return make_minimum(f32x4{x, y});
    }

    [[nodiscard]] static interval_vec2 make_maximum(f32x4 other) noexcept
    {
        return {f32x4{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()}, other};
    }

    [[nodiscard]] static interval_vec2 make_maximum(float x, float y) noexcept
    {
        return make_maximum(f32x4{x, y});
    }

    [[nodiscard]] static interval_vec2 make_zero_to_maximum(f32x4 other) noexcept
    {
        return {f32x4{}, other};
    }

    [[nodiscard]] static interval_vec2 make_zero_to_maximum(float x, float y) noexcept
    {
        return make_zero_to_maximum(f32x4{x, y});
    }

    [[nodiscard]] f32x4 minimum() const noexcept
    {
        return (-value).xy00();
    }

    [[nodiscard]] f32x4 maximum() const noexcept
    {
        return value.zw00();
    }

    [[nodiscard]] finterval x() const noexcept
    {
        return finterval(-value.x(), value.z());
    }

    [[nodiscard]] finterval y() const noexcept
    {
        return finterval(-value.y(), value.w());
    }

    [[nodiscard]] finterval width() const noexcept
    {
        return x();
    }

    [[nodiscard]] finterval height() const noexcept
    {
        return y();
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

    [[nodiscard]] friend bool operator==(interval_vec2 const &lhs, interval_vec2 const &rhs) noexcept
    {
        return lhs.value == rhs.value;
    }

    /** Check if lhs.x or lhs.y is smaller then rhs.minimum.
     */
    [[nodiscard]] friend bool operator<<(f32x4 const &lhs, interval_vec2 const &rhs) noexcept
    {
        return lhs.x() < rhs.minimum().x() || lhs.y() < rhs.minimum().y();
    }

    /** Check if lhs.x or lhs.y is larger then rhs.maximum.
     */
    [[nodiscard]] friend bool operator>>(f32x4 const &lhs, interval_vec2 const &rhs) noexcept
    {
        return lhs.x() > rhs.maximum().x() || lhs.y() > rhs.maximum().y();
    }
    [[nodiscard]] friend std::string to_string(interval_vec2 const &rhs) noexcept
    {
        return fmt::format("({}:{}, {}:{})", rhs.value.x(), rhs.value.z(), rhs.value.y(), rhs.value.w());
    }

    friend std::ostream &operator<<(std::ostream &lhs, interval_vec2 const &rhs)
    {
        return lhs << to_string(rhs);
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

    /** Get the maximum interval of both operants.
     */
    [[nodiscard]] friend interval_vec2 min(interval_vec2 const &lhs, interval_vec2 const &rhs) noexcept
    {
        ttlet tmp_max = max(lhs.value, rhs.value);
        ttlet tmp_min = min(lhs.value, rhs.value);
        return make(tmp_max.xy00() + tmp_min._00zw());
    }

    /** Get the minimum interval of both operants.
     */
    [[nodiscard]] friend interval_vec2 max(interval_vec2 const &lhs, interval_vec2 const &rhs) noexcept
    {
        ttlet tmp_max = max(lhs.value, rhs.value);
        ttlet tmp_min = min(lhs.value, rhs.value);
        return make(tmp_min.xy00() + tmp_max._00zw());
    }

private:
    [[nodiscard]] static interval_vec2 make(f32x4 other) noexcept
    {
        tt_axiom(-other.x() <= other.z());
        tt_axiom(-other.y() <= other.w());

        interval_vec2 r;
        r.value = other;
        return r;
    }

    /** The value as (-x_min, -y_min, y_max, x_max)
     */
    f32x4 value;
};

} // namespace tt