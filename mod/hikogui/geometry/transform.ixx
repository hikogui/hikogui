// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <type_traits>
#include <functional>
#include <exception>
#include <compare>

export module hikogui_geometry : transform;
import : aarectangle;
import : circle;
import : corner_radii;
import : extent2;
import : extent3;
import : line_segment;
import : matrix2;
import : matrix3;
import : perspective;
import : point2;
import : point3;
import : quad;
import : rectangle;
import : rotate2;
import : rotate3;
import : scale2;
import : scale3;
import : translate2;
import : translate3;
import : vector2;
import : vector3;

export namespace hi { inline namespace v1 {

template<typename T>
struct transform2 : public std::false_type {};
template<typename T>
struct transform3 : public std::false_type {};

// clang-format off
template<> struct transform2<matrix2> : public std::true_type {};
template<> struct transform3<matrix3> : public std::true_type {};
template<> struct transform2<translate2> : public std::true_type {};
template<> struct transform3<translate3> : public std::true_type {};
template<> struct transform2<rotate2> : public std::true_type {};
template<> struct transform3<rotate3> : public std::true_type {};
template<> struct transform2<scale2> : public std::true_type {};
template<> struct transform3<scale3> : public std::true_type {};
template<> struct transform3<perspective> : public std::true_type {};
// clang-format on

template<typename T>
constexpr bool transform2_v = transform2<T>::value;
template<typename T>
constexpr bool transform3_v = transform3<T>::value;

template<typename T>
concept transformer2 = transform2_v<T>;
template<typename T>
concept transformer3 = transform3_v<T>;
template<typename T>
concept transformer = transformer2<T> or transformer3<T>;

/** Matrix/Matrix multiplication.
 */
[[nodiscard]] constexpr matrix2 operator*(matrix2 const& lhs, matrix2 const& rhs) noexcept
{
    return matrix2{lhs * get<0>(rhs), lhs * get<1>(rhs), lhs * get<2>(rhs), lhs * get<3>(rhs)};
}

/** Matrix/Matrix multiplication.
 */
[[nodiscard]] constexpr matrix3 operator*(matrix3 const& lhs, matrix3 const& rhs) noexcept
{
    return matrix3{lhs * get<0>(rhs), lhs * get<1>(rhs), lhs * get<2>(rhs), lhs * get<3>(rhs)};
}

[[nodiscard]] constexpr matrix2 operator*(translate2 const& lhs, matrix2 const& rhs) noexcept
{
    return matrix2{get<0>(rhs), get<1>(rhs), get<2>(rhs), get<3>(rhs) + f32x4{lhs}};
}

[[nodiscard]] constexpr matrix3 operator*(translate3 const& lhs, matrix3 const& rhs) noexcept
{
    return matrix3{get<0>(rhs), get<1>(rhs), get<2>(rhs), get<3>(rhs) + f32x4{lhs}};
}

[[nodiscard]] constexpr translate2 operator*(translate2 const& lhs, translate2 const& rhs) noexcept
{
    return translate2{f32x4{lhs} + f32x4{rhs}};
}

[[nodiscard]] constexpr translate3 operator*(translate3 const& lhs, translate3 const& rhs) noexcept
{
    return translate3{f32x4{lhs} + f32x4{rhs}};
}

[[nodiscard]] constexpr matrix2 operator*(translate2 const& lhs, scale2 const& rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return matrix2{f32x4{rhs}.x000(), f32x4{rhs}._0y00(), f32x4{rhs}._00z0(), f32x4{lhs}.xyz1()};
}

[[nodiscard]] constexpr matrix3 operator*(translate3 const& lhs, scale3 const& rhs) noexcept
{
    return matrix3{f32x4{rhs}.x000(), f32x4{rhs}._0y00(), f32x4{rhs}._00z0(), f32x4{lhs}.xyz1()};
}

[[nodiscard]] constexpr matrix2 operator*(translate2 const& lhs, rotate2 const& rhs) noexcept
{
    return lhs * matrix2(rhs);
}

[[nodiscard]] constexpr matrix3 operator*(translate3 const& lhs, rotate3 const& rhs) noexcept
{
    return lhs * matrix3(rhs);
}

[[nodiscard]] constexpr matrix2 operator*(scale2 const& lhs, translate2 const& rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return matrix2{f32x4{lhs}.x000(), f32x4{lhs}._0y00(), f32x4{lhs}._00z0(), f32x4{lhs} * f32x4{rhs}.xyz1()};
}

[[nodiscard]] constexpr matrix3 operator*(scale3 const& lhs, translate3 const& rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return matrix3{f32x4{lhs}.x000(), f32x4{lhs}._0y00(), f32x4{lhs}._00z0(), f32x4{lhs} * f32x4{rhs}.xyz1()};
}

[[nodiscard]] constexpr scale2 operator*(scale2 const& lhs, scale2 const& rhs) noexcept
{
    return scale2{f32x4{lhs} * f32x4{rhs}};
}

[[nodiscard]] constexpr scale3 operator*(scale3 const& lhs, scale3 const& rhs) noexcept
{
    return scale3{f32x4{lhs} * f32x4{rhs}};
}

/******************* Shape operations ******************/

/** Transform a float by the scaling factor of the matrix.
 *
 * The floating point number is transformed into a vector laying on the x-axis,
 * then transformed, then extracting the hypot from it.
 */
[[nodiscard]] float operator*(matrix2 const& lhs, float const& rhs) noexcept
{
    // As if _col0 * rhs.xxxx() in operator*(f32x4 rhs)
    hilet abs_scale = hypot<0b0011>(get<0>(lhs) * f32x4::broadcast(rhs));

    // We want to keep the sign of the original scaler, even if the matrix has rotation.
    return std::copysign(abs_scale, rhs);
}

/** Transform a float by the scaling factor of the matrix.
 *
 * The floating point number is transformed into a vector laying on the x-axis,
 * then transformed, then extracting the hypot from it.
 */
[[nodiscard]] float operator*(matrix3 const &lhs, float const& rhs) noexcept
{
    // As if _col0 * rhs.xxxx() in operator*(f32x4 rhs)
    hilet abs_scale = hypot<0b0111>(get<0>(lhs) * f32x4::broadcast(rhs));

    // We want to keep the sign of the original scaler, even if the matrix has rotation.
    return std::copysign(abs_scale, rhs);
}

[[nodiscard]] constexpr float operator*(translate2 const&, float const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr float operator*(translate3 const&, float const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr float operator*(scale2 const& lhs, float const& rhs) noexcept
{
    return lhs.x() * rhs;
}

[[nodiscard]] constexpr float operator*(scale3 const& lhs, float const& rhs) noexcept
{
    return lhs.x() * rhs;
}

[[nodiscard]] constexpr float operator*(rotate2 const&, float const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr float operator*(rotate3 const&, float const& rhs) noexcept
{
    return rhs;
}

/** Transform a vector by the matrix.
 *
 * Vectors will not be translated.
 *
 * @param rhs The vector to be transformed.
 * @return The transformed vector.
 */
[[nodiscard]] constexpr vector2 operator*(matrix2 const& lhs, vector2 const& rhs) noexcept
{
    return vector2{get<0>(lhs) * f32x4{rhs}.xxxx() + get<1>(lhs) * f32x4{rhs}.yyyy()};
}

[[nodiscard]] constexpr vector2 operator*(translate2 const&, vector2 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr vector2 operator*(translate3 const&, vector2 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr vector2 operator*(scale2 const& lhs, vector2 const& rhs) noexcept
{
    return vector2{f32x4{lhs} * f32x4{rhs}};
}

[[nodiscard]] constexpr vector2 operator*(scale3 const& lhs, vector2 const& rhs) noexcept
{
    return vector2{f32x4{lhs} * f32x4{rhs}};
}

/** Transform a vector by the matrix.
 *
 * Vectors will not be translated.
 *
 * @param rhs The vector to be transformed.
 * @return The transformed vector.
 */
[[nodiscard]] constexpr vector3 operator*(matrix3 const& lhs, vector3 const& rhs) noexcept
{
    return vector3{get<0>(lhs) * f32x4{rhs}.xxxx() + get<1>(lhs) * f32x4{rhs}.yyyy() + get<2>(lhs) * f32x4{rhs}.zzzz()};
}

[[nodiscard]] constexpr vector3 operator*(translate3 const&, vector3 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr vector3 operator*(scale3 const& lhs, vector3 const& rhs) noexcept
{
    return vector3{f32x4{lhs} * f32x4{rhs}};
}

/** Transform a point by the matrix.
 *
 * @param rhs The point to be transformed.
 * @return The transformed point.
 */
[[nodiscard]] constexpr point2 operator*(matrix2 const& lhs, point2 const& rhs) noexcept
{
    return point2{get<0>(lhs) * f32x4{rhs}.xxxx() + get<1>(lhs) * f32x4{rhs}.yyyy() + get<3>(lhs)};
}

[[nodiscard]] constexpr point2 operator*(translate2 const& lhs, point2 const& rhs) noexcept
{
    return point2{f32x4{lhs} + f32x4{rhs}};
}

[[nodiscard]] constexpr point2 operator*(scale2 const& lhs, point2 const& rhs) noexcept
{
    return point2{f32x4{lhs} * f32x4{rhs}};
}

[[nodiscard]] constexpr point2 operator*(scale3 const& lhs, point2 const& rhs) noexcept
{
    return point2{f32x4{lhs} * f32x4{rhs}};
}

/** Transform a point by the matrix.
 *
 * @param rhs The point to be transformed.
 * @return The transformed point.
 */
[[nodiscard]] constexpr point3 operator*(matrix3 const& lhs, point3 const& rhs) noexcept
{
    return point3{
        get<0>(lhs) * f32x4{rhs}.xxxx() + get<1>(lhs) * f32x4{rhs}.yyyy() + get<2>(lhs) * f32x4{rhs}.zzzz() +
        get<3>(lhs) * f32x4{rhs}.wwww()};
}

[[nodiscard]] constexpr point3 operator*(translate3 const& lhs, point3 const& rhs) noexcept
{
    return point3{f32x4{lhs} + f32x4{rhs}};
}

[[nodiscard]] constexpr point3 operator*(scale3 const& lhs, point3 const& rhs) noexcept
{
    return point3{f32x4{lhs} * f32x4{rhs}};
}

[[nodiscard]] constexpr extent2 operator*(transformer2 auto const& lhs, extent2 const& rhs) noexcept
{
    return extent2{lhs *vector2{rhs}};
}

[[nodiscard]] constexpr extent3 operator*(transformer3 auto const& lhs, std::convertible_to<extent3> auto const& rhs) noexcept
{
    return extent3{lhs *vector3{rhs}};
}

[[nodiscard]] constexpr aarectangle operator*(translate2 const& lhs, aarectangle const& rhs) noexcept
{
    return aarectangle{lhs * get<0>(rhs), lhs * get<3>(rhs)};
}

/** Scale a rectangle around it's center.
 */
[[nodiscard]] constexpr aarectangle operator*(scale2 const& lhs, aarectangle const& rhs) noexcept
{
    return aarectangle{lhs * get<0>(rhs), lhs * get<3>(rhs)};
}

/** Transform an axis-aligned rectangle by the matrix.
 *
 * After transformation it can not be guaranteed that an axis-aligned rectangle
 * remained aligned to axis, therefor a normal rectangle is returned
 *
 * @note It is undefined behavior to perspective transform a rectangle
 * @param rhs The axis-aligned rectangle to be transformed.
 * @return The transformed rectangle
 */
[[nodiscard]] constexpr rectangle operator*(transformer auto const& lhs, aarectangle const& rhs) noexcept
{
    hilet rhs_ = rectangle{rhs};
    return rectangle{lhs * rhs_.origin, lhs * rhs_.right, lhs * rhs_.up};
}

/** Transform an axis-aligned rectangle without rotation by the matrix.
 *
 * @note It is undefined behaviour to perspective, rotate or skew transform a rectangle
 * @param rhs The axis-aligned rectangle to be transformed.
 * @return The transformed rectangle
 */
[[deprecated("Use full_mul() or fast_mul() instead.")]] [[nodiscard]] constexpr rectangle
operator*(matrix2 const& lhs, aarectangle const& rhs) noexcept
{
    hilet rhs_ = rectangle{rhs};
    return rectangle{lhs * rhs_.origin, lhs * rhs_.right, lhs * rhs_.up};
}

/** Transform an axis-aligned rectangle without rotation by the matrix.
 *
 * @note It is undefined behaviour to perspective, rotate or skew transform a rectangle
 * @param rhs The axis-aligned rectangle to be transformed.
 * @return The transformed rectangle
 */
[[nodiscard]] constexpr rectangle full_mul(matrix2 const& lhs, aarectangle const& rhs) noexcept
{
    hilet rhs_ = rectangle{rhs};
    return rectangle{lhs * rhs_.origin, lhs * rhs_.right, lhs * rhs_.up};
}

/** Transform an axis-aligned rectangle without rotation by the matrix.
 *
 * @note It is undefined behaviour to perspective, rotate or skew transform a rectangle
 * @param rhs The axis-aligned rectangle to be transformed.
 * @return The transformed rectangle
 */
[[nodiscard]] constexpr aarectangle fast_mul(matrix2 const& lhs, aarectangle const& rhs) noexcept
{
    return aarectangle{lhs * get<0>(rhs), lhs * get<3>(rhs)};
}

/** Transform a rectangle by the matrix.
 *
 * @note It is undefined behavior to perspective transform a rectangle
 * @param rhs The rectangle to be transformed.
 * @return The transformed rectangle
 */
[[nodiscard]] constexpr rectangle operator*(transformer auto const& lhs, rectangle const& rhs) noexcept
{
    return rectangle{lhs * rhs.origin, lhs * rhs.right, lhs * rhs.up};
}

/** Transform a quad by the matrix.
 *
 * @param rhs The quad to be transformed.
 * @return The transformed quad
 */
[[nodiscard]] constexpr quad operator*(transformer auto const& lhs, quad const& rhs) noexcept
{
    return quad{lhs * rhs.p0, lhs * rhs.p1, lhs * rhs.p2, lhs * rhs.p3};
}

/** Transform a circle by the matrix.
 *
 * @param rhs The circle to be transformed.
 * @return The transformed circle
 */
[[nodiscard]] constexpr circle operator*(transformer auto const& lhs, circle const& rhs) noexcept
{
    return circle{lhs * midpoint(rhs), lhs * rhs.radius()};
}

/** Transform a line-segment by the matrix.
 *
 * @param rhs The line-segment to be transformed.
 * @return The transformed line-segment
 */
[[nodiscard]] constexpr line_segment operator*(transformer auto const& lhs, line_segment const& rhs) noexcept
{
    return line_segment{lhs * rhs.origin(), lhs * rhs.direction()};
}

/** Transform a float by the scaling factor of the matrix.
 *
 * The floating point number is transformed into a vector laying on the x-axis,
 * then transformed, then extracting the hypot from it.
 */
[[nodiscard]] constexpr corner_radii operator*(matrix3 const& lhs, corner_radii const& rhs) noexcept
{
    return {lhs * get<0>(rhs), lhs * get<1>(rhs), lhs * get<2>(rhs), lhs * get<3>(rhs)};
}

/** Inplace geometric translation.
 *
 * @param lhs A geometry shape of some kind.
 * @param rhs A geometry transformation of some kind.
 * @return A reference to the modified @a lhs.
 */
template<typename Lhs, transformer Rhs>
constexpr Lhs& operator*=(Lhs& lhs, Rhs const& rhs) noexcept
    requires requires(Lhs& a, Rhs const& b) { a = b * a; }
{
    return lhs = rhs * lhs;
}

[[nodiscard]] constexpr aarectangle fit(aarectangle const& bounds, aarectangle const& rectangle) noexcept
{
    hilet resized_rectangle = aarectangle{
        rectangle.left(),
        rectangle.bottom(),
        std::min(rectangle.width(), bounds.width()),
        std::min(rectangle.height(), bounds.height())};

    hilet translate_from_p0 = max(vector2{}, get<0>(bounds) - get<0>(resized_rectangle));
    hilet translate_from_p3 = min(vector2{}, get<3>(bounds) - get<3>(resized_rectangle));
    return translate2{translate_from_p0 + translate_from_p3} * resized_rectangle;
}

/** scale the quad.
 *
 * Each edge of the quad scaled.
 *
 * @param lhs A quad.
 * @param rhs The width and height to scale each edge with.
 * @return The new quad extended by the size.
 */
[[nodiscard]] constexpr quad scale_from_center(quad const& lhs, scale2 const& rhs) noexcept
{
    hilet top_extra = (lhs.top() * rhs.x() - lhs.top()) * 0.5f;
    hilet bottom_extra = (lhs.bottom() * rhs.x() - lhs.bottom()) * 0.5f;
    hilet left_extra = (lhs.left() * rhs.y() - lhs.left()) * 0.5f;
    hilet right_extra = (lhs.right() * rhs.y() - lhs.right()) * 0.5f;

    return {
        lhs.p0 - bottom_extra - left_extra,
        lhs.p1 + bottom_extra - right_extra,
        lhs.p2 - top_extra + left_extra,
        lhs.p3 + top_extra + right_extra};
}

}} // namespace hi::v1
