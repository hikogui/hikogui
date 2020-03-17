// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include <algorithm>

namespace TTauri {

[[nodiscard]] constexpr uint32_t make_A2B10G10R10UNorm_value(vec const &rhs) noexcept
{
    let r = static_cast<uint32_t>(std::clamp(rhs.r, 0.0f, 1.0f) * 1023.0f);
    let g = static_cast<uint32_t>(std::clamp(rhs.g, 0.0f, 1.0f) * 1023.0f);
    let b = static_cast<uint32_t>(std::clamp(rhs.b, 0.0f, 1.0f) * 1023.0f);
    let a = static_cast<uint32_t>(std::clamp(rhs.a, 0.0f, 1.0f) * 3.0f);
    return (a << 30) | (b << 20) | (g << 10) | r;
}

struct A2B10G10R10UNorm {
    uint32_t value;

    A2B10G10R10UNorm() = default;
    A2B10G10R10UNorm(A2B10G10R10UNorm const &rhs) noexcept = default;
    A2B10G10R10UNorm(A2B10G10R10UNorm &&rhs) noexcept = default;
    A2B10G10R10UNorm &operator=(A2B10G10R10UNorm const &rhs) noexcept = default;
    A2B10G10R10UNorm &operator=(A2B10G10R10UNorm &&rhs) noexcept = default;
    ~A2B10G10R10UNorm() = default;

    explicit A2B10G10R10UNorm(vec const &rhs) noexcept :
        value(make_A2B10G10R10UNorm_value(rhs)) {}

    A2B10G10R10UNorm &operator=(vec const &rhs) noexcept {
        value = make_A2B10G10R10UNorm_value(rhs);
        return *this;
    }

    explicit operator vec () const noexcept {
        return vec{
            static_cast<float>((value >> 20) & 0x3ff) / 1023.0f,
            static_cast<float>((value >> 10) & 0x3ff) / 1023.0f,
            static_cast<float>(value & 0x3ff) / 1023.0f,
            static_cast<float>(value >> 30) / 3.0f
        };
    }
};

}
