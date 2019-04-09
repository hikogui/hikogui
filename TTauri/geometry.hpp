
#pragma once

#include <glm/glm.hpp>

#include <cstdint>

namespace TTauri {

template <int S, typename T, glm::qualifier Q>
struct extent;

template <typename T, glm::qualifier Q>
struct extent<2, T, Q> : public glm::vec<2, T, Q> {
    extent() : glm::vec<2, T, Q>() {}
    extent(T width, T height) : glm::vec<2, T, Q>(width, height) {}

    T width() { return this->x; }
    T height() { return this->y; }
};

template <typename T, glm::qualifier Q>
struct extent<3, T, Q> : public glm::vec<3, T, Q> {
    extent() : glm::vec<3, T, Q>() {}
    extent(T width, T height, T depth) : glm::vec<3, T, Q>(width, height, depth) {}

    T width() { return this->x; }
    T height() { return this->y; }
    T depth() { return this->z; }
};

template <int S, typename T, glm::qualifier Q>
struct rect {
    glm::vec<S, T, glm::defaultp> offset = {};
    extent<S, T, glm::defaultp> extent = {};
};

using u16vec2 = glm::vec<2, uint16_t, glm::defaultp>;
using u16vec3 = glm::vec<3, uint16_t, glm::defaultp>;
using u16vec4 = glm::vec<4, uint16_t, glm::defaultp>;
using u16rect = rect<2, uint16_t, glm::defaultp>;
using u32rect = rect<2, uint32_t, glm::defaultp>;

}