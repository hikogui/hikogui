// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once


#include "required.hpp"
#include "assert.hpp"
#include <utility>
#include <array>
#include <type_traits>

namespace tt::inline v1 {

[[nodiscard]] inline size_t hash_mix_two(size_t hash1, size_t hash2) noexcept
{
    if constexpr (sizeof(size_t) == 8) {
        return hash1 + 0x9e3779b97f681800 + (hash2 << 6) + (hash2 >> 2);
    } else if constexpr (sizeof(size_t) == 4) {
        return hash1 + 0x9e3779b9 + (hash2 << 6) + (hash2 >> 2);
    } else {
        tt_not_implemented();
    }
}

template<typename First, typename Second, typename... Args>
[[nodiscard]] size_t hash_mix(First &&first, Second &&second, Args &&... args) noexcept {
    if constexpr (sizeof...(args) == 0) {
        return hash_mix_two(
            std::hash<std::remove_cvref_t<First>>{}(std::forward<First>(first)),
            std::hash<std::remove_cvref_t<Second>>{}(std::forward<Second>(second))
        );
    } else {
        return hash_mix_two(
            std::hash<std::remove_cvref_t<First>>{}(std::forward<First>(first)),
            hash_mix(std::forward<Second>(second), std::forward<Args>(args)...)
        );
    }
}


}