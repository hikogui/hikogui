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

/** Pack the msb of each element into a integer
*/
template<typename T, size_t N>
struct array_get_mask {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr size_t operator()(array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_get_mask<T, N>{}(lhs, rhs); }) {
                return simd_get_mask<T, N>{}(rhs);
            }
        }

        auto r = size_t{};
        auto mask = size_t{1};
        for (auto i = 0_uz; i != N; ++i) {
            hilet rhs_ = std::bit_cast<make_intxx_t<sizeof(T) * CHAR_BIT>>(rhs[i]);
            r |= (rhs_ >> sizeof(T) * CHAR_BIT - 1) & mask;
            mask <<= 1;
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_not {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr array_type operator()(array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_not<T, N>{}(lhs, rhs); }) {
                return simd_not<T, N>{}(rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = ~rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_neg {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr array_type operator()(array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_neg<T, N>{}(lhs, rhs); }) {
                return simd_neg<T, N>{}(rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = -rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_test_all_ones {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr bool operator()(array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_test_all_ones<T, N>{}(lhs, rhs); }) {
                return simd_test_all_ones<T, N>{}(rhs);
            }
        }

        auto r = true;
        for (auto i = 0_uz; i != N; ++i) {
            hilet rhs_ = ~std::bit_cast<make_uintxx_t<sizeof(T) * CHAR_BIT>>(rhs[i]);
            r &= rhs_ == 0;
        }
        return r;
    }
};

}}
