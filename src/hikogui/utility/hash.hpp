// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "assert.hpp"
#include <utility>
#include <array>
#include <type_traits>

namespace hi::inline v1 {

[[nodiscard]] constexpr std::size_t hash_mix_two(std::size_t hash1, std::size_t hash2) noexcept
{
    if constexpr (sizeof(std::size_t) == 8) {
        return hash1 + 0x9e3779b97f681800 + (hash2 << 6) + (hash2 >> 2);
    } else if constexpr (sizeof(std::size_t) == 4) {
        return hash1 + 0x9e3779b9 + (hash2 << 6) + (hash2 >> 2);
    } else {
        hi_not_implemented();
    }
}

template<typename First, typename Second, typename... Args>
[[nodiscard]] constexpr std::size_t hash_mix(First &&first, Second &&second, Args &&...args) noexcept
{
    if constexpr (sizeof...(args) == 0) {
        return hash_mix_two(
            std::hash<std::remove_cvref_t<First>>{}(std::forward<First>(first)),
            std::hash<std::remove_cvref_t<Second>>{}(std::forward<Second>(second)));
    } else {
        return hash_mix_two(
            std::hash<std::remove_cvref_t<First>>{}(std::forward<First>(first)),
            hash_mix(std::forward<Second>(second), std::forward<Args>(args)...));
    }
}

} // namespace hi::inline v1