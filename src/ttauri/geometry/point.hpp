

#pragma once

#include "numeric_array.hpp"
#include "vec3.hpp"

namespace tt {

template<int D> requires(D == 2 || D == 3)
class point {
public:
    constexpr point(point const &) noexcept = default;
    constexpr point(point &&) noexcept = default;
    constexpr point &operator=(point const &) noexcept = default;
    constexpr point &operator=(point &&) noexcept = default;

    template<int E> requires (E < D)
    [[nodiscard]] constexpr point(point<E> const &other) noexcept : _v(other._v) {}

    [[nodiscard]] constexpr explicit operator f32x4 () const noexcept { return _v; }
    [[nodiscard]] constexpr explicit point(f32x4 const &other) noexcept : _v(other)
    {
        if constexpr (D == 3) {
            tt_axiom(_v.z() == 0.0);
        }
        tt_axiom(_v.w() == 1.0);
    }

    [[nodiscard]] constexpr point() noexcept : _v(0.0, 0.0, 0.0, 1.0) {} 
    [[nodiscard]] constexpr point(float x, float y) noexcept requires(D == 2) : _v(x, y, 0.0, 1.0) {} 
    [[nodiscard]] constexpr point(float x, float y, float z=0.0) noexcept requires(D == 3): _v(x, y, z, 1.0) {} 

    [[nodiscard]] constexpr float &x() noexcept { return _v.x(); }
    [[nodiscard]] constexpr float &y() noexcept { return _v.y(); }
    [[nodiscard]] constexpr float &z() noexcept requires(D == 3) { return _v.z(); }
    [[nodiscard]] constexpr float const &x() const noexcept { return _v.x(); }
    [[nodiscard]] constexpr float const &y() const noexcept { return _v.y(); }
    [[nodiscard]] constexpr float const &z() const noexcept requires(D == 3) { return _v.z(); }

    [[nodiscard]] constexpr friend point operator+(point const &lhs, vec<D> const &rhs) noexcept { return {lhs._v + rhs._v}; }
    [[nodiscard]] constexpr friend point operator-(point const &lhs, vec<D> const &rhs) noexcept { return {lhs._v - rhs._v}; }
    [[nodiscard]] constexpr friend vec<D> operator-(point const &lhs, point const &rhs) noexcept { return {lhs._v - rhs._v}; }
    [[nodiscard]] constexpr friend float hypot(point const &lhs, point const &rhs) noexcept { return hypot(rhs - lhs); }
    [[nodiscard]] constexpr friend float rcp_hypot(point const &lhs, point const &rhs) noexcept { return rcp_hypot(rhs - lhs); }

private:
    f32x4 _v;

    template<int D>
    friend class mat;
};

}

