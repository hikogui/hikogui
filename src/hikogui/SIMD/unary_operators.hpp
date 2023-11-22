// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_binary_operators.hpp"
#include "simd_unary_operators.hpp"
#include "simd_set.hpp"
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
struct array_not {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr array_type operator()(array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_le<T, N>{}(lhs, rhs); }) {
                return simd_not<T, N>{}(rhs);
            }
            if constexpr (requires { simd_xor<T, N>{}(rhs, simd_set_ones<T, N>()); }) {
                return simd_ge<T, N>{}(rhs, lhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = ~rhs[i];
        }
        return r;
    }
};

}}
