// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace TTauri {

[[nodiscard]] inline glm::u16vec4 make_R16G16B16A16SFloat_value(glm::vec4 const &rhs) noexcept
{
    __m128 floats = _mm_load_ps(reinterpret_cast<float const *>(&rhs));
    __m128i halfs = _mm_cvtps_ph(floats, _MM_FROUND_CUR_DIRECTION);

    // The first 4 entries are the converted floats, the second 4 are not used.
    __m128i buffer;
    _mm_store_si128(&buffer, halfs);

    glm::u16vec4 ret;
    std::memcpy(&ret, &buffer, sizeof(ret));
    return ret;
}

[[nodiscard]] inline glm::u16vec4 make_R16G16B16A16SFloat_value(glm::vec3 const &rhs) noexcept
{
    return make_R16G16B16A16SFloat_value(glm::vec4{rhs, 1.0f});
}

struct R16G16B16A16SFloat {
    // Red, Green, Blue, Alpha in binary16 (native endian).
    glm::u16vec4 value;

    R16G16B16A16SFloat() = default;
    R16G16B16A16SFloat(R16G16B16A16SFloat const &rhs) noexcept = default;
    R16G16B16A16SFloat(R16G16B16A16SFloat &&rhs) noexcept = default;
    R16G16B16A16SFloat &operator=(R16G16B16A16SFloat const &rhs) noexcept = default;
    R16G16B16A16SFloat &operator=(R16G16B16A16SFloat &&rhs) noexcept = default;
    ~R16G16B16A16SFloat() = default;

    explicit R16G16B16A16SFloat(glm::vec4 const &rhs) noexcept :
        value(make_R16G16B16A16SFloat_value(rhs)) {}

    explicit R16G16B16A16SFloat(glm::vec3 const &rhs) noexcept :
        value(make_R16G16B16A16SFloat_value(rhs)) {}

    explicit R16G16B16A16SFloat(wsRGBA const &rhs) noexcept :
        R16G16B16A16SFloat(static_cast<glm::vec4>(rhs)) {}

    explicit R16G16B16A16SFloat(float r, float g, float b, float a) noexcept :
        R16G16B16A16SFloat(glm::vec4{r, g, b, a}) {}

    explicit R16G16B16A16SFloat(float r, float g, float b) noexcept :
        R16G16B16A16SFloat(glm::vec3{r, g, b}) {}

    R16G16B16A16SFloat &operator=(glm::vec4 const &rhs) noexcept {
        value = make_R16G16B16A16SFloat_value(rhs);
        return *this;
    }

    R16G16B16A16SFloat &operator=(glm::vec3 const &rhs) noexcept {
        value = make_R16G16B16A16SFloat_value(rhs);
        return *this;
    }

    explicit operator glm::vec4 () const noexcept {
        std::array<uint16_t,8> buffer;
        buffer[0] = value.r;
        buffer[1] = value.g;
        buffer[2] = value.b;
        buffer[3] = value.a;

        __m128i halfs = _mm_loadu_si16(buffer.data());
        __m128 floats = _mm_cvtph_ps(halfs);

        glm::vec4 r;
        std::memcpy(&r, &floats, sizeof(r));
        return r;
    }

    explicit operator glm::vec3 () const noexcept {
        return glm::xyz(static_cast<glm::vec4>(*this));
    }
};

}
