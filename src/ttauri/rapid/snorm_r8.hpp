// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../cast.hpp"
#include <algorithm>

namespace tt {

[[nodiscard]] constexpr int8_t make_snorm_r8_value(float rhs) noexcept
{
    return narrow_cast<int8_t>(std::clamp(rhs, -1.0f, 1.0f) * 127.0f);
}

struct snorm_r8 {
    int8_t value;

    snorm_r8() = default;
    snorm_r8(snorm_r8 const &rhs) noexcept = default;
    snorm_r8(snorm_r8 &&rhs) noexcept = default;
    snorm_r8 &operator=(snorm_r8 const &rhs) noexcept = default;
    snorm_r8 &operator=(snorm_r8 &&rhs) noexcept = default;
    ~snorm_r8() = default;

    explicit snorm_r8(float rhs) noexcept :
        value(make_snorm_r8_value(rhs)) {}

    snorm_r8 &operator=(float rhs) noexcept {
        value = make_snorm_r8_value(rhs);
        return *this;
    }

    explicit operator float () const noexcept {
        return narrow_cast<float>(value) / 127.0f;
    }
};

}
