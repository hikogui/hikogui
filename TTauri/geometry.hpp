// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <glm/glm.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <cstdint>

namespace TTauri {

template <int S, typename T, glm::qualifier Q>
struct extent;

template <typename T, glm::qualifier Q>
struct extent<2, T, Q> : public glm::vec<2, T, Q> {
    constexpr extent() : glm::vec<2, T, Q>() {}
    constexpr extent(glm::vec<2, T, Q> const &other) : glm::vec<2, T, Q>(other) {}
    constexpr extent(T width, T height) : glm::vec<2, T, Q>(width, height) {}

    constexpr T width() const { return this->x; }
    constexpr T height() const { return this->y; }
};

template <typename T, glm::qualifier Q>
struct extent<3, T, Q> : public glm::vec<3, T, Q> {
    constexpr extent() : glm::vec<3, T, Q>() {}
    constexpr extent(glm::vec<3, T, Q> const& other) : glm::vec<3, T, Q>(other) {}
    constexpr extent(T width, T height, T depth) : glm::vec<3, T, Q>(width, height, depth) {}

    constexpr T width() const { return this->x; }
    constexpr T height() const { return this->y; }
    constexpr T depth() const { return this->z; }
};

template <int S, typename T, glm::qualifier Q>
struct rect {
    glm::vec<S, T, glm::defaultp> offset = {};
    extent<S, T, glm::defaultp> extent = {};

    bool operator==(const rect<S, T, Q> &other) {
        return offset == other.offset && extent == other.extent;
    }
};



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
    }};;
}

inline glm::vec2 midpoint(glm::vec2 a, glm::vec2 b) {
    return { (a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f };
}

using u16vec2 = glm::vec<2, uint16_t, glm::defaultp>;
using u16vec3 = glm::vec<3, uint16_t, glm::defaultp>;
using u64vec2 = glm::vec<2, uint64_t, glm::defaultp>;
using u64extent2 = extent<2, uint64_t, glm::defaultp>;
using extent2 = extent<2, float, glm::defaultp>;
using u16rect2 = rect<2, uint16_t, glm::defaultp>;
using u64rect2 = rect<2, uint64_t, glm::defaultp>;
using rect2 = rect<2, float, glm::defaultp>;

inline float viktorCross(glm::vec2 const a, glm::vec2 const b) {
    return a.x* b.y - a.y * b.x;
}

}
