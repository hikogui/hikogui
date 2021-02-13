// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "vector.hpp"
#include "point.hpp"

namespace tt::geo {

template<int D>
class matrix {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D rotation-matrices are supported");

    constexpr matrix(matrix const &) noexcept = default;
    constexpr matrix(matrix &&) noexcept = default;
    constexpr matrix &operator=(matrix const &) noexcept = default;
    constexpr matrix &operator=(matrix &&) noexcept = default;

    constexpr matrix() noexcept :
        _col0(1.0f, 0.0f, 0.0f, 0.0f),
        _col1(0.0f, 1.0f, 0.0f, 0.0f),
        _col2(0.0f, 0.0f, 1.0f, 0.0f),
        _col3(0.0f, 0.0f, 0.0f, 1.0f){};

    constexpr matrix(f32x4 col0, f32x4 col1, f32x4 col2, f32x4 col3 = f32x4{0.0f, 0.0f, 0.0f, 0.1f}) noexcept :
        _col0(col0), _col1(col1), _col2(col2), _col3(col3)
    {
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(vector<E> const &rhs) const noexcept
    {
        tt_axiom(rhs.is_valid());
        return vector<std::max(D,E)>{
            _col0 * static_cast<f32x4>(rhs).xxxx() + _col1 * static_cast<f32x4>(rhs).yyyy() +
            _col2 * static_cast<f32x4>(rhs).zzzz() + _col3 * static_cast<f32x4>(rhs).wwww()};
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(point<E> const &rhs) const noexcept
    {
        tt_axiom(rhs.is_valid());
        return point<std::max(D,E)>{
            _col0 * static_cast<f32x4>(rhs).xxxx() + _col1 * static_cast<f32x4>(rhs).yyyy() +
            _col2 * static_cast<f32x4>(rhs).zzzz() + _col3 * static_cast<f32x4>(rhs).wwww()};
    }

private:
    f32x4 _col0;
    f32x4 _col1;
    f32x4 _col2;
    f32x4 _col3;
};

using matrix2 = matrix<2>;
using matrix3 = matrix<3>;

} // namespace tt
