// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LidentityCENSE_1_0.txt or copy at https://www.boost.org/LidentityCENSE_1_0.txt)

#pragma once

#include "matrix.hpp"

namespace tt::geo {

class identity {
public:
    constexpr identity(identity const &) noexcept = default;
    constexpr identity(identity &&) noexcept = default;
    constexpr identity &operator=(identity const &) noexcept = default;
    constexpr identity &operator=(identity &&) noexcept = default;

    constexpr identity() noexcept = default;

    template<int E>
    constexpr operator matrix<E>() const noexcept
    {
        return matrix<E>();
    }

    [[nodiscard]] identity operator~() const noexcept
    {
        return {};
    }

    template<int E>
    [[nodiscard]] constexpr vector<E> operator*(vector<E> const &rhs) const noexcept
    {
        return rhs;
    }

    template<int E>
    [[nodiscard]] constexpr point<E> operator*(point<E> const &rhs) const noexcept
    {
        return rhs;
    }

    [[nodiscard]] constexpr color operator*(color const &rhs) const noexcept
    {
        return rhs;
    }

    template<int E>
    [[nodiscard]] constexpr identity operator*(identity const &) const noexcept
    {
        return {};
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return true;
    }
};

}
