// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/numeric_array.hpp"
#include <algorithm>

namespace tt {

[[nodiscard]] constexpr uint32_t make_unorm_a2bgr10_pack_value(f32x4 const &rhs) noexcept
{
    ttlet r = static_cast<uint32_t>(std::clamp(rhs.r, 0.0f, 1.0f) * 1023.0f);
    ttlet g = static_cast<uint32_t>(std::clamp(rhs.g, 0.0f, 1.0f) * 1023.0f);
    ttlet b = static_cast<uint32_t>(std::clamp(rhs.b, 0.0f, 1.0f) * 1023.0f);
    ttlet a = static_cast<uint32_t>(std::clamp(rhs.a, 0.0f, 1.0f) * 3.0f);
    return (a << 30) | (b << 20) | (g << 10) | r;
}

struct unorm_a2bgr10_pack {
    uint32_t value;

    unorm_a2bgr10_pack() = default;
    unorm_a2bgr10_pack(unorm_a2bgr10_pack const &rhs) noexcept = default;
    unorm_a2bgr10_pack(unorm_a2bgr10_pack &&rhs) noexcept = default;
    unorm_a2bgr10_pack &operator=(unorm_a2bgr10_pack const &rhs) noexcept = default;
    unorm_a2bgr10_pack &operator=(unorm_a2bgr10_pack &&rhs) noexcept = default;
    ~unorm_a2bgr10_pack() = default;

    explicit unorm_a2bgr10_pack(f32x4 const &rhs) noexcept :
        value(make_unorm_a2bgr10_pack_value(rhs)) {}

    unorm_a2bgr10_pack &operator=(f32x4 const &rhs) noexcept {
        value = make_unorm_a2bgr10_pack_value(rhs);
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
