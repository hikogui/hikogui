// Copyright translateake Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix.hpp"
#include "identity.hpp"

namespace tt {
namespace geo {

template<int D>
class translate {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D translation-matrices are supported");

    constexpr translate(translate const &) noexcept = default;
    constexpr translate(translate &&) noexcept = default;
    constexpr translate &operator=(translate const &) noexcept = default;
    constexpr translate &operator=(translate &&) noexcept = default;

    [[nodiscard]] constexpr operator matrix<D>() const noexcept
    {
        tt_axiom(is_valid());
        ttlet ones = f32x4::broadcast(1.0);
        return matrix<D>{ones.x000(), ones._0y00(), ones._00z0(), ones._000w() + _v};
    }

    [[nodiscard]] constexpr translate() noexcept : _v() {}

    [[nodiscard]] constexpr translate(identity const &) noexcept : translate() {}

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        tt_axiom(is_valid());
        return _v;
    }

    [[nodiscard]] constexpr explicit translate(f32x4 const &other) noexcept : _v(other)
    {
        tt_axiom(is_valid());
    }

    template<int E>
    requires(E < D) [[nodiscard]] constexpr translate(translate<E> const &other) noexcept : _v(static_cast<f32x4>(other))
    {
        tt_axiom(is_valid());
    }

    template<int E>
    requires(E <= D) [[nodiscard]] constexpr explicit translate(vector<E> const &other) noexcept : _v(static_cast<f32x4>(other))
    {
        tt_axiom(is_valid());
    }

    [[nodiscard]] constexpr translate(float x, float y) noexcept requires(D == 2) : _v(x, y, 0.0, 0.0) {}

    [[nodiscard]] constexpr translate(float x, float y, float z = 0.0) noexcept requires(D == 3) : _v(x, y, z, 0.0) {}

    template<int E>
    [[nodiscard]] constexpr vector<E> operator*(vector<E> const &rhs) const noexcept
    {
        // Vectors are not translated.
        tt_axiom(is_valid() && rhs.is_valid());
        return rhs;
    }

    template<int E>
    [[nodiscard]] constexpr point<std::max(D, E)> operator*(point<E> const &rhs) const noexcept
    {
        tt_axiom(is_valid() && rhs.is_valid());
        return point<std::max(D, E)>{_v + static_cast<f32x4>(rhs)};
    }

    [[nodiscard]] constexpr translate operator*(identity const &) const noexcept
    {
        tt_axiom(is_valid());
        return *this;
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(translate<E> const &rhs) const noexcept
    {
        tt_axiom(is_valid() && rhs.is_valid());
        return translate<std::max(D, E)>{_v + static_cast<f32x4>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr bool operator==(translate<E> const &rhs) const noexcept
    {
        tt_axiom(is_valid() && rhs.is_valid());
        return {_v == static_cast<f32x4>(rhs)};
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return _v.w() == 0.0f && (D == 3 || _v.z() == 0.0f);
    }

private:
    f32x4 _v;
};

} // namespace geo

using translate2 = geo::translate<2>;
using translate3 = geo::translate<3>;

} // namespace tt
