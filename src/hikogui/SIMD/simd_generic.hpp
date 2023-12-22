


#pragma once

#include <array>

hi_export_module(hikogui.simd.simd_generic);

hi_export namespace hi { inline namespace v1 {

template<typename T, size_t N>
struct simd_generic {
    using value_type = T;

    // clang-format off
    using mask_type =
        std::conditional(sizeof(T) * CHAR_BIT ==  8, uint8_t,
        std::conditional(sizeof(T) * CHAR_BIT == 16, uint16_t,
        std::conditional(sizeof(T) * CHAR_BIT == 32, uint32_t
        std::conditional(sizeof(T) * CHAR_BIT == 64, uint64_t, void))));
    // clang-format on

    using array_type = std::array<T, N>;
    constexpr static size_t size = N;


    [[nodiscard]] constexpr static array_type neg(array_type a) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::neg(a); }) {
                return simd_intrinsic<T, N>::neg(a);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = -a[i];
        }
        return r;
    }

    [[nodiscard]] constexpr static array_type inv(array_type a) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::neg(a); }) {
                return simd_intrinsic<T, N>::neg(a);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = ~a[i];
        }
        return r;
    }


    [[nodiscard]] constexpr static array_type add(array_type a, array_type b) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::add(a, b); }) {
                return simd_intrinsic<T, N>::add(a, b);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = a[i] + b[i];
        }
        return r;
    }

    [[nodiscard]] constexpr static array_type add(array_type a, array_type b) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::sub(a, b); }) {
                return simd_intrinsic<T, N>::sub(a, b);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = a[i] - b[i];
        }
        return r;
    }

    [[nodiscard]] constexpr static array_type mul(array_type a, array_type b) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::mul(a, b); }) {
                return simd_intrinsic<T, N>::mul(a, b);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = a[i] * b[i];
        }
        return r;
    }

    [[nodiscard]] constexpr static array_type div(array_type a, array_type b) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::div(a, b); }) {
                return simd_intrinsic<T, N>::div(a, b);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = a[i] / b[i];
        }
        return r;
    }

    [[nodiscard]] constexpr static array_type mod(array_type a, array_type b) const noexcept requires std::integral<T>
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::mod(a, b); }) {
                return simd_intrinsic<T, N>::mod(a, b);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = a[i] % b[i];
        }
        return r;
    }

    [[nodiscard]] constexpr static array_type _or(array_type a, array_type b) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::_or(a, b); }) {
                return simd_intrinsic<T, N>::_or(a, b);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = std::bit_cast<T>(std::bit_cast<mask_type>(a[i]) | std::bit_cast<mask_type>(b[i]));
        }
        return r;
    }

    [[nodiscard]] constexpr static array_type _and(array_type a, array_type b) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::_and(a, b); }) {
                return simd_intrinsic<T, N>::_and(a, b);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = std::bit_cast<T>(std::bit_cast<mask_type>(a[i]) & std::bit_cast<mask_type>(b[i]));
        }
        return r;
    }

    [[nodiscard]] constexpr static array_type _xor(array_type a, array_type b) const noexcept
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { simd_intrinsic<T, N>::_xor(a, b); }) {
                return simd_intrinsic<T, N>::_xor(a, b);
            }
        }

        auto r = array_type{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = std::bit_cast<T>(std::bit_cast<mask_type>(a[i]) ^ std::bit_cast<mask_type>(b[i]));
        }
        return r;
    }



};

}}

