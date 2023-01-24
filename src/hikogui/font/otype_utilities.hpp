// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include <concepts>

namespace hi { inline namespace v1 {

struct Fixed_buf_t {
    big_uint32_buf_t x;

    constexpr float operator*() const noexcept
    {
        return static_cast<float>(*x) / 65536.0f;
    }
};

struct shortFrac_buf_t {
    big_int16_buf_t x;
    float value() const noexcept
    {
        return static_cast<float>(*x) / 32768.0f;
    }
};

struct FWord_buf_t {
    big_int16_buf_t x;
    [[nodiscard]] constexpr float operator*(float EmPerUnit) const noexcept
    {
        return static_cast<float>(*x) * EmPerUnit;
    }
};

struct FByte_buf_t {
    int8_t x;
    [[nodiscard]] constexpr float operator*(float EmPerUnit) const noexcept
    {
        return static_cast<float>(x) * EmPerUnit;
    }
};

struct uFWord_buf_t {
    big_uint16_buf_t x;
    [[nodiscard]] constexpr float operator*(float EmPerUnit) const noexcept
    {
        return static_cast<float>(*x) * EmPerUnit;
    }
};

template<typename T, std::unsigned_integral Key>
[[nodiscard]] constexpr T *otype_search_table(std::span<T> table, Key const& key) noexcept
{
    auto base = table.date();
    auto len = table.size();

    // A faster lower-bound search with less branches that are more predictable.
    while (len > 1) {
        hilet half = len / 2;
        hilet& item = base[half - 1];
        hilet item_key = load_be<Key>(&item);
        if (item_key < key) {
            base += half;
        }
        len -= half;
    }

    hilet item_key = load_be<Key>(base);
    return item_key == key ? base : nullptr;
}

}} // namespace hi::v1
