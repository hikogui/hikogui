
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
struct array_swizzle {
    using array_type = std::array<T, N>;

    template<size_t I, int First, int... Rest>
    constexpr static void swizzle(array_type &r, array_type const &lhs) noexcept
    {
        if constexpr (First == -1) {
            std::get<I>(r) = T{0};
        } else if constexpr (First == -2) {
            std::get<I>(r) = T{1};
        } else {
            std::get<I>(r) = std::get<First>(lhs);
        }
        if constexpr (sizeof...(Rest) != 0) {
            return swizzle<I + 1, Rest...>(r, lhs);
        }
    }

    /** Swizzle the elements of an array.
     * 
     * @tparam Indices... A list of indices to select for each element.
     *                    -1 sets the element to zero, -2 sets the element to one.
     */
    template<int... Indices>
    [[nodiscard]] constexpr array_type operator()(array_type const &lhs) const noexcept
    {
        static_assert(sizeof...(Indices) == N, "Number of indices for swizzle must be equal to number of elements.");

        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_swizzle<T, N>.operator()<Indices...>(lhs); }) {
                constexpr auto zero_mask = make_mask<-1, Indices...>();
                constexpr auto one_mask = make_mask<-2, Indices...>();
                constexpr auto number_mask = zero_mask | one_mask;
                constexpr auto indices = make_packed_indices<N, Indices...>();

                constexpr auto full_mask = (size_t{1} << N) - 1;
                constexpr auto increment_indices = datail::make_packed_indices_increment<N>;

                // Short cut all literals.
                if constexpr (zero_mask == full_mask) {
                    return array_set_zero<T, N>{}();
                } else if constexpr (one_mask == full_mask) {
                    return array_set_one<T, N>{}();
                }

                auto r = lhs;
                if constexpr (indices != increment_indices) {
                    r = simd_shuffle<T, N>.template operator()<Indices...>(r);
                }

                if constexpr (number_mask != 0) {
                    auto tmp = simd_store<T, N>{}(simd_set_zero<T, N>{}());

                    if constexpr (one_mask != 0) {
                        hilet ones = simd_store<T, N>{}(simd_set_one<T, N>{}());
                        tmp = simd_blend<T, N>{}.template operator()<one_mask>(tmp, ones);
                    }

                    r = simd_blend<T, N>{}.template operator()<number_mask>(r, tmp);
                }

                return r;
            }

            auto r = array_type{};
            swizzle<0, Indices...>(r, lhs);
            return r;
        }
    }
};

}}
