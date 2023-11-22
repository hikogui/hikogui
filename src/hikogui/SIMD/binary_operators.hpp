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

#define HI_X(NAME, SIMD_NAME, OPERATOR) \
    template<typename T, size_t N> \
    struct NAME { \
        using array_type = std::array<T, N>; \
        [[nodiscard]] constexpr array_type operator()(array_type const& lhs, array_type const& rhs) const noexcept \
        { \
            if (not std::is_constant_evaluated()) { \
                if constexpr (requires { SIMD_NAME<T, N>{}(lhs, rhs); }) { \
                    return SIMD_NAME<T, N>{}(lhs, rhs); \
                } \
            } \
\
            auto r = array_type{}; \
            for (auto i = 0_uz; i != N; ++i) { \
                r[i] = lhs[i] OPERATOR rhs[i]; \
            } \
            return r; \
        } \
    }

HI_X(array_add, simd_add, +);
HI_X(array_sub, simd_sub, -);
HI_X(array_mul, simd_mul, *);
HI_X(array_div, simd_div, /);
HI_X(array_or, simd_or, |);
HI_X(array_and, simd_and, &);

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


#undef HI_X

}} // namespace hi::v1
