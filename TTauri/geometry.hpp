// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <cstdint>
#include <optional>

namespace TTauri {

constexpr glm::mat3x3 mat3x3Identity = glm::mat3x3{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

template <int S, typename T, glm::qualifier Q>
struct extent;

template <typename T, glm::qualifier Q>
struct extent<2, T, Q> : public glm::vec<2, T, Q> {
    constexpr extent() : glm::vec<2, T, Q>() {}
    constexpr extent(glm::vec<2, T, Q> const &other) : glm::vec<2, T, Q>(other) {}
    constexpr extent(T width, T height) : glm::vec<2, T, Q>({width, height}) {}
    constexpr extent(double width, double height) : glm::vec<2, T, Q>({width, height}) {}

    constexpr T const &width() const { return this->x; }
    constexpr T const &height() const { return this->y; }
    constexpr T &width() { return this->x; }
    constexpr T &height() { return this->y; }
};

template <typename T, glm::qualifier Q>
struct extent<3, T, Q> : public glm::vec<3, T, Q> {
    constexpr extent() : glm::vec<3, T, Q>() {}
    constexpr extent(glm::vec<3, T, Q> const& other) : glm::vec<3, T, Q>(other) {}
    constexpr extent(T width, T height, T depth) : glm::vec<3, T, Q>({width, height, depth}) {}

    constexpr T const &width() const { return this->x; }
    constexpr T const &height() const { return this->y; }
    constexpr T const &depth() const { return this->z; }
    constexpr T &width() { return this->x; }
    constexpr T &height() { return this->y; }
    constexpr T &depth() { return this->z; }
};

template <int S, typename T, glm::qualifier Q>
struct rect {
    glm::vec<S, T, glm::defaultp> offset = {};
    extent<S, T, glm::defaultp> extent = {};

    bool operator==(const rect<S, T, Q> &other) const {
        return offset == other.offset && extent == other.extent;
    }

    bool contains(const glm::vec<S, T, glm::defaultp> &position) const {
        return
            (position.x >= offset.x) &&
            (position.y >= offset.y) &&
            (position.x < (offset.x + extent.width())) &&
            (position.y < (offset.y + extent.height()));
    }
};


using u32extent2 = extent<2, uint32_t, glm::defaultp>;
using u64extent2 = extent<2, uint64_t, glm::defaultp>;
using extent2 = extent<2, float, glm::defaultp>;
using u16rect2 = rect<2, uint16_t, glm::defaultp>;
using u32rect2 = rect<2, uint32_t, glm::defaultp>;
using u64rect2 = rect<2, uint64_t, glm::defaultp>;
using rect2 = rect<2, float, glm::defaultp>;

inline rect2 &operator*=(rect2 &lhs, glm::mat3x3 const &rhs)
{
    lhs.offset = (rhs * glm::vec3(lhs.offset, 1.0f)).xy;
    // XXX is this correct during rotation?
    lhs.extent = extent2{(rhs * glm::vec3(lhs.extent, 0.0f)).xy};
    return lhs;
}

inline rect2 &operator+=(rect2 &lhs, glm::vec2 const &rhs)
{
    lhs.offset += rhs;
    return lhs;
}

template<typename T, typename U>
inline T rect2_cast(U other)
{
    T r;

    return { {
        boost::numeric_cast<decltype(r.offset.x)>(other.offset.x),
        boost::numeric_cast<decltype(r.offset.y)>(other.offset.y)
    }, {
        boost::numeric_cast<decltype(r.extent.x)>(other.extent.x),
        boost::numeric_cast<decltype(r.extent.y)>(other.extent.y)
    } };;
}

inline glm::vec2 midpoint(glm::vec2 a, glm::vec2 b)
{
    return (a + b) * 0.5f;
}

inline glm::vec2 midpoint(rect2 r)
{
    return midpoint(r.offset, r.offset + r.extent);
}

inline float viktorCross(glm::vec2 const a, glm::vec2 const b)
{
    return a.x * b.y - a.y * b.x;
}

inline glm::vec2 normal(glm::vec2 const a)
{
    return glm::normalize(glm::vec2{-a.y, a.x});
}


inline glm::vec2 bezierPointAt(glm::vec2 P1, glm::vec2 P2, float t) {
    let v = P2 - P1;
    return v * t + P1;
}

inline glm::vec2 bezierPointAt(glm::vec2 P1, glm::vec2 C, glm::vec2 P2, float t)
{
    let a = P1 - C * 2.0f + P2;
    let b = (C - P1) * 2.0f;
    let c = P1;
    return a*t*t + b*t + c;
}

inline glm::vec2 bezierPointAt(glm::vec2 P1, glm::vec2 C1, glm::vec2 C2, glm::vec2 P2, float t)
{
    let a = -P1 + C1 * 3.0f - C2 * 3.0f + P2;
    let b = P1 * 3.0f - C1 * 6.0f + C2 * 3.0f;
    let c = P1 * -3.0f + C1 * 3.0f;
    let d = P1;
    return a*t*t*t + b*t*t + c*t + d;
}

/*! Return the flatness of a curve.
 * \return 1.0 when completely flat, < 1.0 when curved.
 */
inline float bezierFlatness(glm::vec2 P1, glm::vec2 P2)
{
    return 1.0f;
}

/*! Return the flatness of a curve.
* \return 1.0 when completely flat, < 1.0 when curved.
*/
inline float bezierFlatness(glm::vec2 P1, glm::vec2 C, glm::vec2 P2)
{
    let P1P2 = glm::length(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    let P1C1 = glm::length(C - P1);
    let C1P2 = glm::length(P2 - C);
    return P1P2 / (P1C1 + C1P2);
}

/*! Return the flatness of a curve.
* \return 1.0 when completely flat, < 1.0 when curved.
*/
inline float bezierFlatness(glm::vec2 P1, glm::vec2 C1, glm::vec2 C2, glm::vec2 P2)
{
    let P1P2 = glm::length(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    let P1C1 = glm::length(C1 - P1);
    let C1C2 = glm::length(C2 - C1);
    let C2P2 = glm::length(P2 - C2);
    return P1P2 / (P1C1 + C1C2 + C2P2);
}

inline std::pair<glm::vec2, glm::vec2> parrallelLine(glm::vec2 P1, glm::vec2 P2, float distance)
{
    let v = P2 - P1;
    let n = normal(v);
    return {
        P1 + n * distance,
        P2 + n * distance
    };
}

/*! Find the intersect points between two line segments.
 */
inline std::optional<glm::vec2> getIntersectionPoint(glm::vec2 A1, glm::vec2 A2, glm::vec2 B1, glm::vec2 B2)
{
    // convert points to vectors.
    let p = A1;
    let r = A2 - A1;
    let q = B1;
    let s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    let crossRS = viktorCross(r, s);
    if (crossRS == 0.0f) {
        // Parrallel, other non, or a range of points intersect.
        return {};

    } else {
        let q_min_p = q - p;
        let t = viktorCross(q_min_p, s) / crossRS;
        let u = viktorCross(q_min_p, r) / crossRS;

        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            return bezierPointAt(A1, A2, t);
        } else {
            // The lines intesect outside of one or both of the segments.
            return {};
        }
    }
}

/*! Find the intersect points between two line segments.
*/
inline std::optional<glm::vec2> getExtrapolatedIntersectionPoint(glm::vec2 A1, glm::vec2 A2, glm::vec2 B1, glm::vec2 B2)
{
    // convert points to vectors.
    let p = A1;
    let r = A2 - A1;
    let q = B1;
    let s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    let crossRS = viktorCross(r, s);
    if (crossRS == 0.0f) {
        // Parrallel, other non, or a range of points intersect.
        return {};

    } else {
        let q_min_p = q - p;
        let t = viktorCross(q_min_p, s) / crossRS;

        return bezierPointAt(A1, A2, t);
    }
}

inline glm::mat3x3 T2D(glm::vec2 position, float scale=1.0f, float rotation=0.0f)
{
    auto M = mat3x3Identity;
    M = glm::translate(M, position);
    M = glm::rotate(M, rotation);
    M = glm::scale(M, glm::vec2{scale, scale});
    return M;
}

inline glm::mat3x3 T2D(glm::vec2 position, glm::vec2 scale, float rotation=0.0f)
{
    auto M = mat3x3Identity;
    M = glm::translate(M, position);
    M = glm::rotate(M, rotation);
    M = glm::scale(M, scale);
    return M;
}

inline glm::mat3x3 T2D(glm::vec2 position, glm::mat2x2 scale, float rotation=0.0f)
{
    let scale3x3 = glm::mat3x3{
        scale[0][0], scale[0][1], 0.0f,
        scale[1][0], scale[1][1], 0.0f,
        0.0f, 0.0f, 1.0f
    };

    auto M = mat3x3Identity;
    M = glm::translate(M, position);
    M = glm::rotate(M, rotation);
    M = scale3x3 * M;
    return M;
}

}