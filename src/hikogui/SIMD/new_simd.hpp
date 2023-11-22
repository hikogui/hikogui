// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "binary_operators.hpp"
#include "unary_operators.hpp"
#include <cstddef>
#include <array>

hi_export_module(hikogui.simd.simd_binary_operators);

namespace hi { inline namespace v1 {

template<typename T, size_t N>
struct simd_mask : public std::array<T, N> {
    using std::array<T, N>;

    explicit constexpr simd(std::array<T, N> const& rhs) noexcept : std::array<T, N>(rhs) {}

    explicit operator bool() const noexcept
    {
        return {array_all_one<T, N>{}(rhs)};
    }
};

template<typename T, size_t N>
struct simd : public std::array<T, N> {
    using std::array<T, N>;

    explicit constexpr simd(std::array<T, N> const& rhs) noexcept : std::array<T, N>(rhs) {}

    [[nodiscard]] friend constexpr simd operator~(simd const& rhs) noexcept
    {
        return simd{array_not<T, N>{}(rhs)};
    }

    [[nodiscard]] friend constexpr simd operator-(simd const& rhs) noexcept
    {
        return simd{array_neg<T, N>{}(rhs)};
    }

    [[nodiscard]] friend constexpr simd operator+(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_add<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd operator-(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_sub<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd operator*(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_mul<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd operator/(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_div<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd operator&(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_and<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd operator|(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_or<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd operator^(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_xor<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd andnot(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_andnot<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd operator<<(simd const& lhs, size_t const& rhs) noexcept
    {
        return simd{array_sl<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd operator>>(simd const& lhs, size_t const& rhs) noexcept
    {
        return simd{array_sr<T, N>{}(lhs, rhs)};
    }

    template<std::unsigned_integral Rhs>
    [[nodiscard]] friend constexpr simd operator<<(simd const& lhs, simd<Rhs, N> const& rhs) noexcept
    {
        return simd{array_sl<T, N>{}(lhs, rhs)};
    }

    template<std::unsigned_integral Rhs>
    [[nodiscard]] friend constexpr simd operator>>(simd const& lhs, simd<Rhs, N> const& rhs) noexcept
    {
        return simd{array_sr<T, N>{}(lhs, rhs)};
    }

    template<size_t Rhs>
    [[nodiscard]] friend constexpr simd sll(simd const& lhs) noexcept
    {
        return simd{array_sl<T, N>{}.operator()<Rhs>(lhs)};
    }

    template<size_t Rhs>
    [[nodiscard]] friend constexpr simd srl(simd const& lhs) noexcept
    {
        return simd{array_srl<T, N>{}.operator()<Rhs>(lhs)};
    }

    template<size_t Rhs>
    [[nodiscard]] friend constexpr simd sra(simd const& lhs) noexcept
    {
        return simd{array_sra<T, N>{}.operator()<Rhs>(lhs)};
    }

    template<size_t Rhs>
    [[nodiscard]] friend constexpr simd rol(simd const& lhs) noexcept
    {
        return simd{array_rol<T, N>{}.operator()<Rhs>(lhs)};
    }

    template<size_t Rhs>
    [[nodiscard]] friend constexpr simd ror(simd const& lhs) noexcept
    {
        return simd{array_ror<T, N>{}.operator()<Rhs>(lhs)};
    }

    [[nodiscard]] friend constexpr simd max(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_max<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd min(simd const& lhs, simd const& rhs) noexcept
    {
        return simd{array_min<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd_mask operator==(simd const& lhs, simd const& rhs) noexcept
    {
        return simd_mask{array_eq<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd_mask operator!=(simd const& lhs, simd const& rhs) noexcept
    {
        return simd_mask{array_ne<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd_mask operator<(simd const& lhs, simd const& rhs) noexcept
    {
        return simd_mask{array_lt<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd_mask operator>(simd const& lhs, simd const& rhs) noexcept
    {
        return simd_mask{array_gt<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd_mask operator<=(simd const& lhs, simd const& rhs) noexcept
    {
        return simd_mask{array_le<T, N>{}(lhs, rhs)};
    }

    [[nodiscard]] friend constexpr simd_mask operator>=(simd const& lhs, simd const& rhs) noexcept
    {
        return simd_mask{array_ge<T, N>{}(lhs, rhs)};
    }

    template<int... Indices>
    [[nodiscard]] friend constexpr simd swizzle(simd const& lhs) noexcept
    {
        static_assert(sizeof...(Indices) == N, "Number of indices must match number of elements.");
        return simd{array_swizzle<T, N>{}.operator()<Indices...>(lhs)};
    }

    /** Swizzle elements of a vector by the name of the elements.
     *
     * The following characters correspond with the indices:
     *  - 'x', 'y', 'z', 'w' -> 0, 1, 2, 3
     *  - 'a' - 'p' -> 0 - 15
     *  - 'A' - 'P' -> 16 - 31
     *  - '0' -> literal 0
     *  - '1' -> literal 1
     *
     * @tparam Name The order of each element. The first character is the first
     *              element to return.
     * @param lhs The vector to reorder
     * @return The vector with reordered elements.
     */
    template<fixed_string Name, int... Indices>
    [[nodiscard]] friend constexpr simd swizzle(simd const& lhs) noexcept
    {
        static_assert(Name.size() == N, "Name size must match number of elements.");

        if constexpr (sizeof...(Indices) == Name.size()) {
            return swizzle<Indices...>(lhs);

        } else {
            constexpr auto c == std::get<sizeof...(Indices)>(Name);
            if constexpr (c == '0') {
                return swizzle<Name, Indices..., -1>(lhs);
            } else if constexpr (c == '1') {
                return swizzle<Name, Indices..., -2>(lhs);
            } else if constexpr (c >= 'x' and c <= 'z') {
                return swizzle<Name, Indices..., static_cast<int>(c - 'x')>(lhs);
            } else if constexpr (c == 'w') {
                return swizzle<Name, Indices..., 3>(lhs);
            } else if constexpr (c >= 'a' and c <= 'p') {
                return swizzle<Name, Indices..., static_cast<int>(c - 'a')>(lhs);
            } else if constexpr (c >= 'A' and c <= 'P') {
                return swizzle<Name, Indices..., static_cast<int>(c - 'a' + 16)>(lhs);
            } else {
                hi_static_no_default();
            }
        }
    }

#define SWIZZLE(name, str) \
    [[nodiscard]] constexpr simd name() const noexcept \
        requires(sizeof(str) - 1 == N) \
    { \
        return swizzle<str>(*this); \
    }

#define SWIZZLE_4D(name, str) \
    SWIZZLE(name##0, str "0") \
    SWIZZLE(name##1, str "1") \
    SWIZZLE(name##x, str "a") \
    SWIZZLE(name##y, str "b") \
    SWIZZLE(name##z, str "c") \
    SWIZZLE(name##w, str "d")

#define SWIZZLE_3D(name, str) \
    SWIZZLE_4D(name##0, str "0") \
    SWIZZLE_4D(name##1, str "1") \
    SWIZZLE_4D(name##x, str "a") \
    SWIZZLE_4D(name##y, str "b") \
    SWIZZLE_4D(name##z, str "c") \
    SWIZZLE_4D(name##w, str "d") \
    SWIZZLE(name##0, str "0") \
    SWIZZLE(name##1, str "1") \
    SWIZZLE(name##x, str "a") \
    SWIZZLE(name##y, str "b") \
    SWIZZLE(name##z, str "c") \
    SWIZZLE(name##w, str "d")

#define SWIZZLE_2D(name, str) \
    SWIZZLE_3D(name##0, str "0") \
    SWIZZLE_3D(name##1, str "1") \
    SWIZZLE_3D(name##x, str "a") \
    SWIZZLE_3D(name##y, str "b") \
    SWIZZLE_3D(name##z, str "c") \
    SWIZZLE_3D(name##w, str "d") \
    SWIZZLE(name##0, str "0") \
    SWIZZLE(name##1, str "1") \
    SWIZZLE(name##x, str "a") \
    SWIZZLE(name##y, str "b") \
    SWIZZLE(name##z, str "c") \
    SWIZZLE(name##w, str "d")

    SWIZZLE_2D(_0, "0")
    SWIZZLE_2D(_1, "1")
    SWIZZLE_2D(x, "a")
    SWIZZLE_2D(y, "b")
    SWIZZLE_2D(z, "c")
    SWIZZLE_2D(w, "d")

#undef SWIZZLE
#undef SWIZZLE_2D
#undef SWIZZLE_3D
#undef SWIZZLE_4D
};

}} // namespace hi::v1
