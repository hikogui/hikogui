// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "numeric_array.hpp"
#include <algorithm>

namespace tt {

[[nodiscard]] constexpr uint32_t make_A2B10G10R10UNormPack32_value(f32x4 const &rhs) noexcept
{
    ttlet r = static_cast<uint32_t>(std::clamp(rhs.r, 0.0f, 1.0f) * 1023.0f);
    ttlet g = static_cast<uint32_t>(std::clamp(rhs.g, 0.0f, 1.0f) * 1023.0f);
    ttlet b = static_cast<uint32_t>(std::clamp(rhs.b, 0.0f, 1.0f) * 1023.0f);
    ttlet a = static_cast<uint32_t>(std::clamp(rhs.a, 0.0f, 1.0f) * 3.0f);
    return (a << 30) | (b << 20) | (g << 10) | r;
}

struct A2B10G10R10UNormPack32 {
    uint32_t value;

    A2B10G10R10UNormPack32() = default;
    A2B10G10R10UNormPack32(A2B10G10R10UNormPack32 const &rhs) noexcept = default;
    A2B10G10R10UNormPack32(A2B10G10R10UNormPack32 &&rhs) noexcept = default;
    A2B10G10R10UNormPack32 &operator=(A2B10G10R10UNormPack32 const &rhs) noexcept = default;
    A2B10G10R10UNormPack32 &operator=(A2B10G10R10UNormPack32 &&rhs) noexcept = default;
    ~A2B10G10R10UNormPack32() = default;

    explicit A2B10G10R10UNormPack32(f32x4 const &rhs) noexcept :
        value(make_A2B10G10R10UNormPack32_value(rhs)) {}

    A2B10G10R10UNormPack32 &operator=(f32x4 const &rhs) noexcept {
        value = make_A2B10G10R10UNormPack32_value(rhs);
        return *this;
    }

    explicit operator f32x4 () const noexcept {
        return f32x4{
            static_cast<float>((value >> 20) & 0x3ff) / 1023.0f,
            static_cast<float>((value >> 10) & 0x3ff) / 1023.0f,
            static_cast<float>(value & 0x3ff) / 1023.0f,
            static_cast<float>(value >> 30) / 3.0f
        };
    }
};

}
