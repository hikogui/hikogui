// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"

#if PROCESSOR == CPU_X64
#include <xmmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#endif

namespace TTauri {

template<typename T, int N>
struct simd {
    static_assert(N >= 2);

    alignas(sizeof(T) * N) std::array<T,N> v;

    constexpr simd() = default;

    template<typename A, typename B,
        std::enable_if_t<std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, int> = 0>
    simd(A const &a, B const &b) noexcept {
        get<0>(v) = numeric_cast<T>(a);
        get<1>(v) = numeric_cast<T>(b);
        for (int i = 2; i < N; ++i) {
            v[i] = T{};
        }
    }

#if PROCESSOR == CPU_X64
    simd(__m128 other) noexcept {
        std::array<float,4> tmp;
        _mm_storeu_ps(tmp.data(), other);
        for (int i = 0; i != N; ++i) {
            if (i < 4) {
                v[i] = numeric_cast<T>(tmp[i]);
            } else {
                v[i] = T{};
            }
        }
    }
#endif

#if PROCESSOR == CPU_X64
    operator __m128 () const noexcept {
        std::array<float,4> tmp;
        for (int i = 0; i != 4; ++i) {
            if (i < N) {
                tmp[i] = numeric_cast<float>(v[i]);
            } else {
                tmp[i] = 0.0f;
            }
        }
        return _mm_loadu_ps(tmp.data());
    }
#endif
  
    [[nodiscard]] constexpr T const &operator[](size_t i) const noexcept {
        return v[i];
    }

    [[nodiscard]] constexpr T &operator[](size_t i) noexcept {
        return v[i];
    }

    template<int I>
    [[nodiscard]] friend constexpr T const &get(simd const &rhs) noexcept {
        return get<I>(rhs.v);
    }

    template<int I>
    [[nodiscard]] friend constexpr T &get(simd &rhs) noexcept {
        return get<I>(rhs.v);
    }

#define BINARY_OP(op)\
    [[nodiscard]] friend constexpr simd operator op (simd const &lhs, simd const &rhs) noexcept {\
        simd r;\
        for (int i = 0; i != N; ++i) {\
            r[i] = lhs[i] op rhs[i];\
        }\
        return r;\
    }

    BINARY_OP(+);
    BINARY_OP(-);
    BINARY_OP(*);
    BINARY_OP(/);

    [[nodiscard]] friend constexpr simd min(simd const &lhs, simd const &rhs) noexcept {
        simd r;
        for (int i = 0; i != N; ++i) {
            r[i] = lhs[i] < rhs[i] ? lhs[i] : rhs[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr simd max(simd const &lhs, simd const &rhs) noexcept {
        simd r;
        for (int i = 0; i != N; ++i) {
            r[i] = lhs[i] > rhs[i] ? lhs[i] : rhs[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr simd clamp(simd const &lhs, simd const &minimum, simd const &maximum) noexcept {
        simd r;
        for (int i = 0; i != N; ++i) {
            r[i] = 
                (lhs[i] < minimum[i]) ? minimum[i] :
                (lhs[i] > maximum[i]) ? maximum[i] :
                lhs[i];
        }
        return r;
    }

    [[nodiscard]] friend bool operator==(simd const &lhs, simd const &rhs) {
        bool result = true;
        for (int i = 0; i != N; ++i) {
            result &= static_cast<int>(lhs[i] == rhs[i]);
        }
        return result;
    }

    [[nodiscard]] friend bool operator!=(simd const &lhs, simd const &rhs) {
        return !(lhs == rhs);
    }
};

#undef BINARY_OP

using f32x2_t = simd<float,2>;
using f32x4_t = simd<float,4>;
using f32x8_t = simd<float,8>;
using f32x16_t = simd<float,16>;

using f64x2_t = simd<float,2>;
using f64x4_t = simd<float,4>;
using f64x8_t = simd<float,8>;

using i8x2_t = simd<int8_t,2>;
using i8x4_t = simd<int8_t,4>;
using i8x8_t = simd<int8_t,8>;
using i8x16_t = simd<int8_t,16>;
using i8x32_t = simd<int8_t,32>;
using i8x64_t = simd<int8_t,64>;

using i16x2_t = simd<int16_t,2>;
using i16x4_t = simd<int16_t,4>;
using i16x8_t = simd<int16_t,8>;
using i16x16_t = simd<int16_t,16>;
using i16x32_t = simd<int16_t,32>;

using i32x2_t = simd<int32_t,2>;
using i32x4_t = simd<int32_t,4>;
using i32x8_t = simd<int32_t,8>;
using i32x16_t = simd<int32_t,16>;

using i64x2_t = simd<int64_t,2>;
using i64x4_t = simd<int64_t,4>;
using i64x8_t = simd<int64_t,8>;

using u8x2_t = simd<uint8_t,2>;
using u8x4_t = simd<uint8_t,4>;
using u8x8_t = simd<uint8_t,8>;
using u8x16_t = simd<uint8_t,16>;
using u8x32_t = simd<uint8_t,32>;
using u8x64_t = simd<uint8_t,64>;

using u16x2_t = simd<uint16_t,2>;
using u16x4_t = simd<uint16_t,4>;
using u16x8_t = simd<uint16_t,8>;
using u16x16_t = simd<uint16_t,16>;
using u16x32_t = simd<uint16_t,32>;

using u32x2_t = simd<uint32_t,2>;
using u32x4_t = simd<uint32_t,4>;
using u32x8_t = simd<uint32_t,8>;
using u32x16_t = simd<uint32_t,16>;

using u64x2_t = simd<uint64_t,2>;
using u64x4_t = simd<uint64_t,4>;
using u64x8_t = simd<uint64_t,8>;
}
