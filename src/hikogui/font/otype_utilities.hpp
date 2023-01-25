// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../placement.hpp"
#include <concepts>

namespace hi { inline namespace v1 {

/** Open-type 16.16 signed fixed point, range between -32768.0 and 32767.999
 */
struct otype_fixed15_16_buf_t {
    big_uint32_buf_t x;

    constexpr float operator*() const noexcept
    {
        return static_cast<float>(*x) / 65536.0f;
    }
};

/** Open-type 16-bit signed fraction, range between -2.0 and 1.999
 */
struct otype_fixed1_14_buf_t {
    big_int16_buf_t x;
    float value() const noexcept
    {
        return static_cast<float>(*x) / 16384.0f;
    }
};

/** Open-type for 16 signed integer that must be scaled by the EM-scale.
 */
struct otype_fword_buf_t {
    big_int16_buf_t x;
    [[nodiscard]] constexpr float operator*(float Em_scale) const noexcept
    {
        return static_cast<float>(*x) * Em_scale;
    }
};

/** Open-type for 8 signed integer that must be scaled by the EM-scale.
 */
struct otype_fbyte_buf_t {
    int8_t x;
    [[nodiscard]] constexpr float operator*(float Em_scale) const noexcept
    {
        return static_cast<float>(x) * Em_scale;
    }
};

/** Open-type for 16 unsigned integer that must be scaled by the EM-scale.
 */
struct otype_fuword_buf_t {
    big_uint16_buf_t x;
    [[nodiscard]] constexpr float operator*(float Em_scale) const noexcept
    {
        return static_cast<float>(*x) * Em_scale;
    }
};

template<typename T, std::unsigned_integral Key>
[[nodiscard]] constexpr T *otype_search_table(std::span<T> table, Key const& key) noexcept
{
    auto base = table.data();
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

template<typename T, std::unsigned_integral Key>
[[nodiscard]] constexpr T const *otype_search_table(placement_array<T> const &table, Key const& key) noexcept
{
    return otype_search_table(std::span<T const>(table), key);
}

}} // namespace hi::v1
