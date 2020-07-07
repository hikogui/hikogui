// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/numeric_cast.hpp"
#include <algorithm>

namespace tt {

[[nodiscard]] constexpr int8_t make_R8SNorm_value(float rhs) noexcept
{
    return numeric_cast<int8_t>(std::clamp(rhs, -1.0f, 1.0f) * 127.0f);
}

struct R8SNorm {
    int8_t value;

    R8SNorm() = default;
    R8SNorm(R8SNorm const &rhs) noexcept = default;
    R8SNorm(R8SNorm &&rhs) noexcept = default;
    R8SNorm &operator=(R8SNorm const &rhs) noexcept = default;
    R8SNorm &operator=(R8SNorm &&rhs) noexcept = default;
    ~R8SNorm() = default;

    explicit R8SNorm(float rhs) noexcept :
        value(make_R8SNorm_value(rhs)) {}

    R8SNorm &operator=(float rhs) noexcept {
        value = make_R8SNorm_value(rhs);
        return *this;
    }

    explicit operator float () const noexcept {
        return numeric_cast<float>(value) / 127.0f;
    }
};

}
