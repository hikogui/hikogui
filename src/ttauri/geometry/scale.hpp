// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix.hpp"
#include "identity.hpp"

namespace tt::geo {

template<int D>
class scale {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D scale-matrices are supported");

    constexpr scale(scale const &) noexcept = default;
    constexpr scale(scale &&) noexcept = default;
    constexpr scale &operator=(scale const &) noexcept = default;
    constexpr scale &operator=(scale &&) noexcept = default;

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        tt_axiom(is_valid());
        return _v;
    }

    [[nodiscard]] constexpr explicit scale(f32x4 const &v) noexcept : _v(v)
    {
        tt_axiom(is_valid());
    }

    [[nodiscard]] constexpr operator matrix<D>() const noexcept
    {
        tt_axiom(is_valid());
        return matrix{_v.x000(), _v._0y00(), _v._00z0(), _v._000w()};
    }

    [[nodiscard]] constexpr scale() noexcept : _v(1.0, 1.0, 1.0, 1.0) {}

    [[nodiscard]] constexpr scale(identity const &) noexcept : _v(1.0, 1.0, 1.0, 1.0) {}

    [[nodiscard]] constexpr scale(float value) noexcept requires(D == 2) : _v(value, value, 1.0, 1.0) {}

    [[nodiscard]] constexpr scale(float value) noexcept requires(D == 3) : _v(value, value, value, 1.0) {}

    [[nodiscard]] constexpr scale(float x, float y) noexcept requires(D == 2) : _v(x, y, 1.0, 1.0) {}

    [[nodiscard]] constexpr scale(float x, float y, float z = 1.0) noexcept requires(D == 3) : _v(x, y, z, 1.0) {}

    template<int E>
    [[nodiscard]] constexpr vector<E> operator*(vector<E> const &rhs) const noexcept
    {
        tt_axiom(is_valid() && rhs.is_valid());
        return vector<E>{_v * static_cast<f32x4>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr point<E> operator*(point<E> const &rhs) const noexcept
    {
        tt_axiom(is_valid() && rhs.is_valid());
        return point<E>{_v * static_cast<f32x4>(rhs)};
    }

    [[nodiscard]] constexpr scale operator*(identity const &) const noexcept
    {
        tt_axiom(is_valid());
        return *this;
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(scale<E> const &rhs) const noexcept
    {
        tt_axiom(is_valid() && rhs.is_valid());
        return scale<std::max(D, E)>{_v * static_cast<f32x4>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr bool operator==(scale<E> const &rhs) const noexcept
    {
        tt_axiom(is_valid() && rhs.is_valid());
        return {_v == static_cast<f32x4>(rhs)};
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return _v.w() == 1.0f && (D == 3 || _v.z() == 1.0f);
    }

private:
    f32x4 _v;
};

}
