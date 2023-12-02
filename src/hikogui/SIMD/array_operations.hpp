// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_binary_operators.hpp"
#include "simd_unary_operators.hpp"
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
struct array_add {
    using array_type = std::array<T, N>;

    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_add<T, N>{}(lhs, rhs); }) {
                return simd_add<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] + rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_sub {
    using array_type = std::array<T, N>;

    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_sub<T, N>{}(lhs, rhs); }) {
                return simd_sub<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] - rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_mul {
    using array_type = std::array<T, N>;

    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_mul<T, N>{}(lhs, rhs); }) {
                return simd_mul<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] * rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_div {
    using array_type = std::array<T, N>;

    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_div<T, N>{}(lhs, rhs); }) {
                return simd_div<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] / rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_or {
    using array_type = std::array<T, N>;

    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_or<T, N>{}(lhs, rhs); }) {
                return simd_or<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] | rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_and {
    using array_type = std::array<T, N>;

    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_and<T, N>{}(lhs, rhs); }) {
                return simd_and<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] & rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_xor {
    using array_type = std::array<T, N>;

    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_xor<T, N>{}(lhs, rhs); }) {
                return simd_xor<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] ^ rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_andnot {
    using array_type = std::array<T, N>;

    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_addnot<T, N>{}(lhs, rhs); }) {
                return simd_addnot<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = ~lhs[i] & rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct simd_sl {
    using array_type = std::array<int64_t, 2>;

    template<size_t Rhs>
    [[nodiscard]] constexpr array_type operator()(array_type const &lhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_sl<T, N>{}.operator()<Rhs>(lhs); }) {
                return simd_sl<T, N>{}.operator()<Rhs>(lhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] << Rhs;
        }
        return r;
    }

    [[nodiscard]] constexpr array_type operator()(array_type const &lhs, size_t rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_sl<T, N>{}(lhs, rhs); }) {
                return simd_sl<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] << rhs;
        }
        return r;
    }

    template<std::integral Rhs>
    [[nodiscard]] constexpr array_type operator()(array_type const &lhs, array_type<Rhs, N> rhs) const noexcept
        requires (sizeof(Rhs) == sizeof(T))
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_sl<T, N>{}(lhs, rhs); }) {
                return simd_sl<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = lhs[i] << rhs[i];
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_lt {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_lt<T, N>{}(lhs, rhs); }) {
                return simd_lt<T, N>{}(lhs, rhs);
            }
            if constexpr (requires { simd_gt<T, N>{}(rhs, lhs); }) {
                return simd_gt<T, N>{}(rhs, lhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = to_mask<T>(lhs[i] < rhs[i]);
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_gt {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_gt<T, N>{}(lhs, rhs); }) {
                return simd_gt<T, N>{}(lhs, rhs);
            }
            if constexpr (requires { simd_lt<T, N>{}(rhs, lhs); }) {
                return simd_ltt<T, N>{}(rhs, lhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = to_mask<T>(lhs[i] > rhs[i]);
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_eq {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_eq<T, N>{}(lhs, rhs); }) {
                return simd_eq<T, N>{}(lhs, rhs);
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = to_mask<T>(lhs[i] == rhs[i]);
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_ne {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_ne<T, N>{}(lhs, rhs); }) {
                return simd_ne<T, N>{}(lhs, rhs);
            }
            if constexpr (requires { simd_not<T, N>{}(simd_eq(lhs, rhs)); }) {
                return simd_not<T, N>{}(simd_eq(lhs, rhs));
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = to_mask<T>(lhs[i] != rhs[i]);
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_le {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_le<T, N>{}(lhs, rhs); }) {
                return simd_le<T, N>{}(lhs, rhs);
            }
            if constexpr (requires { simd_ge<T, N>{}(rhs, lhs); }) {
                return simd_ge<T, N>{}(rhs, lhs);
            }
            if constexpr (requires { simd_not<T, N>{}(simd_gt(lhs, rhs)); }) {
                return simd_not<T, N>{}(simd_gt(lhs, rhs));
            }
            if constexpr (requires { simd_not<T, N>{}(simd_lt(rhs, lhs)); }) {
                return simd_not<T, N>{}(simd_lt(rhs, lhs));
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = to_mask<T>(lhs[i] <= rhs[i]);
        }
        return r;
    }
};

template<typename T, size_t N>
struct array_ge {
    using array_type = std::array<T, N>;
    [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_ge<T, N>{}(lhs, rhs); }) {
                return simd_ge<T, N>{}(lhs, rhs);
            }
            if constexpr (requires { simd_le<T, N>{}(rhs, lhs); }) {
                return simd_le<T, N>{}(rhs, lhs);
            }
            if constexpr (requires { simd_not<T, N>{}(simd_lt(lhs, rhs)); }) {
                return simd_not<T, N>{}(simd_lt(lhs, rhs));
            }
            if constexpr (requires { simd_not<T, N>{}(simd_gt(rhs, lhs)); }) {
                return simd_not<T, N>{}(simd_gt(rhs, lhs));
            }
        }

        auto r = array_type{};
        for (auto i = 0_uz; i != N; ++i) {
            r[i] = to_mask<T>(lhs[i] >= rhs[i]);
        }
        return r;
    }
};


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

}} // namespace hi::v1
