// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <cstddef>
#include <array>
#include <cstdint>
#include <type_traits>
#include <concepts>

hi_export_module(hikogui.simd.binary_operators);

namespace hi { inline namespace v1 {

template<typename T, size_t N>
struct simd_setones;

template<>
struct simd_setones<float, 4> {
    [[nodiscard]] hi_force_inline __m128 operator()() const noexcept {
        auto tmp = _mm_undefined_si128();
        return _mm_castsi128_ps(_mm_cmpeq_epi32(tmp, tmp));
    }
};

}}
