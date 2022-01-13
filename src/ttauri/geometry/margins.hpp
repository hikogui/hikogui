// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../rapid/numeric_array.hpp"

namespace tt::inline v1 {

class margins {
public:
    constexpr margins(margins const &) noexcept = default;
    constexpr margins(margins &&) noexcept = default;
    constexpr margins &operator=(margins const &) noexcept = default;
    constexpr margins &operator=(margins &&) noexcept = default;

    [[nodiscard]] constexpr margins() noexcept : _v() {}
    [[nodiscard]] constexpr margins(float margin) noexcept : _v(margin, margin, margin, margin) {}
    [[nodiscard]] constexpr margins(float bottom, float left, float top, float right) noexcept : _v(bottom, left, top, right) {}
    [[nodiscard]] constexpr explicit margins(f32x4 v) noexcept : _v(v) {}

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr float bottom() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float left() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float top() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float right() const noexcept
    {
        return _v.w();
    }

    template<int I>
    [[nodiscard]] constexpr friend float get(margins const &rhs) noexcept
    {
        return get<I>(rhs._v);
    }

    [[nodiscard]] constexpr float operator[](std::size_t i) const noexcept
    {
        return _v[i];
    }

private:
    f32x4 _v;
};

} // namespace tt::inline v1
