// Copyright 2019 Pokitec
// All rights reserved.

#pragma once


#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/type_traits.hpp"
#include <emmintrin.h>
#include <wmmintrin.h>
#include <utility>
#include <array>

namespace TTauri {

[[nodiscard]] inline size_t hash_mix_two(size_t hash1, size_t hash2) noexcept
{
    auto const round = _mm_set_epi64x(0x123456789abcdef0ULL, 0x0fedcba987654321ULL);

    auto hash = _mm_set_epi64x(hash1, hash2);
    hash = _mm_aesenc_si128(hash, round);
    hash = _mm_aesenc_si128(hash, round);

    std::array<uint64_t,2> buffer;
    _mm_storeu_si64(buffer.data(), hash);
    return buffer[0];
}

template<typename First, typename Second, typename... Args>
[[nodiscard]] size_t hash_mix(First &&first, Second &&second, Args &&... args) noexcept {
    if constexpr (sizeof...(args) == 0) {
        return hash_mix_two(
            std::hash<remove_cvref_t<First>>{}(std::forward<First>(first)),
            std::hash<remove_cvref_t<Second>>{}(std::forward<Second>(second))
        );
    } else {
        return hash_mix_two(
            std::hash<remove_cvref_t<First>>{}(std::forward<First>(first)),
            hash_mix(std::forward<Second>(second), std::forward<Args>(args)...)
        );
    }
}


}