

#pragma once

#include "../numeric_array.hpp"

namespace tt {

class scale2 {
public:
    scale2(scale2 const &) noexcept = default;
    scale2(scale2 &&) noexcept = default;
    scale2 &operator=(scale2 const &) noexcept = default;
    scale2 &operator=(scale2 &&) noexcept = default;

    [[nodiscard]] explicit scale2(f32x4 other) noexcept : v(std::move(other)) { tt_axiom(v.z == 1.0 && v.w() == 1.0); }
    [[nodiscard]] explicit operator f32x4 () const noexcept { return v; }

    [[nodiscard]] scale2() noexcept : v(1.0, 1.0, 1.0, 1.0) {}
    [[nodiscard]] scale2(float v) noexcept : v(v, v, 1.0, 1.0) {}
    [[nodiscard]] scale2(float x, float y ) noexcept : v(x, y, 1.0, 1.0) {}

    [[nodiscard]] friend vec2 operator*(vec2 const &lhs, scale2 const &rhs) noexcept { return vec2{lhs.v * rhs.v}; }
    [[nodiscard]] friend point2 operator*(point2 const &lhs, scale2 const &rhs) noexcept { return vec2{lhs.v * rhs.v}; }
    [[nodiscard]] friend vec2 operator/(vec2 const &lhs, scale2 const &rhs) noexcept { return vec2{lhs.v / rhs.v}; }
    [[nodiscard]] friend point2 operator/(point2 const &lhs, scale2 const &rhs) noexcept { return vec2{lhs.v / rhs.v}; }

private:
    f32x4 v;
};

class scale3 {
public:
    scale3(scale3 const &) noexcept = default;
    scale3(scale3 &&) noexcept = default;
    scale3 &operator=(scale3 const &) noexcept = default;
    scale3 &operator=(scale3 &&) noexcept = default;

    [[nodiscard]] explicit scale3(f32x4 other) noexcept : v(std::move(other)) { tt_axiom(v.w() == 1.0); }
    [[nodiscard]] explicit scale3(scale2 other) noexcept : v(std::move(other.v)) {}
    [[nodiscard]] explicit operator f32x4 () const noexcept { return v; }

    [[nodiscard]] scale3() noexcept : v(1.0, 1.0, 1.0, 1.0) {}
    [[nodiscard]] scale3(float v) noexcept : v(v, v, v, 1.0) {}
    [[nodiscard]] scale3(float x, float y, float z=1.0) noexcept : v(x, y, z, 1.0) {}

    [[nodiscard]] friend vec3 operator*(vec3 const &lhs, scale3 const &rhs) noexcept { return vec3{lhs.v * rhs.v}; }
    [[nodiscard]] friend point3 operator*(point3 const &lhs, scale3 const &rhs) noexcept { return vec3{lhs.v * rhs.v}; }
    [[nodiscard]] friend vec3 operator/(vec3 const &lhs, scale3 const &rhs) noexcept { return vec3{lhs.v / rhs.v}; }
    [[nodiscard]] friend point3 operator/(point3 const &lhs, scale3 const &rhs) noexcept { return vec3{lhs.v / rhs.v}; }

private:
    f32x4 v;
};

}

