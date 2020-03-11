// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
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
    constexpr extent() noexcept : glm::vec<2, T, Q>() {}
    constexpr extent(glm::vec<2, T, Q> const &other) noexcept : glm::vec<2, T, Q>(other) {}
    constexpr extent(T width, T height) noexcept : glm::vec<2, T, Q>({width, height}) {}
    constexpr extent(double width, double height) noexcept : glm::vec<2, T, Q>({width, height}) {}

    constexpr T const &width() const noexcept { return this->x; }
    constexpr T const &height() const noexcept { return this->y; }
    constexpr T &width() noexcept { return this->x; }
    constexpr T &height() noexcept { return this->y; }
};

template <typename T, glm::qualifier Q>
struct extent<3, T, Q> : public glm::vec<3, T, Q> {
    constexpr extent() noexcept : glm::vec<3, T, Q>() {}
    constexpr extent(glm::vec<3, T, Q> const& other) noexcept : glm::vec<3, T, Q>(other) {}
    constexpr extent(T width, T height, T depth) noexcept : glm::vec<3, T, Q>({width, height, depth}) {}

    constexpr T const &width() const noexcept { return this->x; }
    constexpr T const &height() const noexcept { return this->y; }
    constexpr T const &depth() const noexcept { return this->z; }
    constexpr T &width() noexcept { return this->x; }
    constexpr T &height() noexcept { return this->y; }
    constexpr T &depth() noexcept { return this->z; }
};

template <int S, typename T, glm::qualifier Q>
struct _rect {
    glm::vec<S, T, glm::defaultp> offset = {};
    extent<S, T, glm::defaultp> extent = {};

    bool contains(const glm::vec<S, T, glm::defaultp> &position) const noexcept {
        return
            (position.x >= offset.x) &&
            (position.y >= offset.y) &&
            (position.x < (offset.x + extent.width())) &&
            (position.y < (offset.y + extent.height()));
    }

    T width() const noexcept {
        return extent.width();
    }

    T height() const noexcept {
        return extent.height();
    }

    /** Get coordinate of a corner
     *    2 <-- 3
     *    | \   ^
     *    |  \  |
     *    v   \ |
     *    0 --> 1
     */
     template<int N, std::enable_if_t<S==2,int> = 0>
     glm::vec<S,T,Q> corner() const noexcept {
        static_assert(N >= 0 && N < 4);
        if constexpr (N == 0) {
            return offset;
        } else if constexpr (N == 1) {
            return offset + glm::vec<S,T,Q>{extent.width(), 0.0f};
        } else if constexpr (N == 2) {
            return offset + glm::vec<S,T,Q>{0.0f, extent.height()};
        } else  {
            return offset + extent;
        }
     }

     template<int N, std::enable_if_t<S==2,int> = 0>
     glm::vec<S+1,T,Q> homogeneous_corner() const noexcept {
        return glm::vec<S+1,T,Q>{corner<N>(), 1.0f};
     }

     _rect expand(float e) {
         _rect r;
         for (int i = 0; i != S; ++i) {
            r.offset[i] = offset[i] - e;
            r.extent[i] = extent[i] + e * 2.0f;
         }
         return r;
     }

     [[nodiscard]] friend bool operator==(_rect<S, T, Q> const &lhs, _rect<S, T, Q> const &rhs) noexcept {
         return lhs.offset == rhs.offset && lhs.extent == rhs.extent;
     }

     [[nodiscard]] friend bool operator!=(_rect<S, T, Q> const &lhs, _rect<S, T, Q> const &rhs) noexcept {
        return !(lhs == rhs);
     }

};


using extent2 = extent<2, float, glm::defaultp>;
using iextent2 = extent<2, int, glm::defaultp>;
using irect2 = _rect<2, int, glm::defaultp>;
using rect2 = _rect<2, float, glm::defaultp>;


inline rect2 &operator*=(rect2 &lhs, glm::mat3x3 const &rhs) noexcept
{
    lhs.offset = glm::xy(rhs * glm::vec3(lhs.offset, 1.0f));
    // XXX is this correct during rotation?
    lhs.extent = extent2{glm::xy(rhs * glm::vec3(lhs.extent, 0.0f))};
    return lhs;
}

inline rect2 &operator*=(rect2 &lhs, float const rhs) noexcept
{
    lhs.offset *= rhs;
    lhs.extent = extent2{rhs * lhs.extent};
    return lhs;
}

inline rect2 &operator+=(rect2 &lhs, glm::vec2 const &rhs) noexcept
{
    lhs.offset += rhs;
    return lhs;
}

template<typename T, typename U>
inline T rect2_cast(U other) noexcept
{
    T r;

    return { {
        numeric_cast<decltype(r.offset.x)>(other.offset.x),
        numeric_cast<decltype(r.offset.y)>(other.offset.y)
    }, {
        numeric_cast<decltype(r.extent.x)>(other.extent.x),
        numeric_cast<decltype(r.extent.y)>(other.extent.y)
    } };;
}

inline glm::vec2 midpoint(glm::vec2 a, glm::vec2 b) noexcept
{
    return (a + b) * 0.5f;
}

inline glm::vec2 midpoint(rect2 r) noexcept
{
    return midpoint(r.offset, r.offset + r.extent);
}

inline float viktorCross(glm::vec2 a, glm::vec2 b) noexcept
{
    return a.x * b.y - a.y * b.x;
}


inline glm::vec2 normal(glm::vec2 const a) noexcept
{
    return glm::normalize(glm::vec2{-a.y, a.x});
}


inline glm::mat3x3 T2D(glm::vec2 position, float scale=1.0f, float rotation=0.0f) noexcept
{
    auto M = mat3x3Identity;
    M = glm::translate(M, position);
    M = glm::rotate(M, rotation);
    M = glm::scale(M, glm::vec2{scale, scale});
    return M;
}


inline glm::mat3x3 T2D(glm::vec2 position, glm::vec2 scale, float rotation=0.0f) noexcept
{
    auto M = mat3x3Identity;
    M = glm::translate(M, position);
    M = glm::rotate(M, rotation);
    M = glm::scale(M, scale);
    return M;
}


inline glm::mat3x3 T2D(glm::vec2 position, glm::mat2x2 scale, float rotation=0.0f) noexcept
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
