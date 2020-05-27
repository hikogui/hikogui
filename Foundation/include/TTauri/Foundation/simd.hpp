// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"

namespace TTauri {

template<typename T, int N>
struct simd {
    static_assert(N >= 2);

    alignas(sizeof(T) * N);
    std::array<T,N> v;

    simd(vec other) {
        get<0>(v) = other.x();
        get<1>(v) = other.y();
        if constexpr (N >= 3) {
            get<2>(v) = other.z();
        } else {
            ttauri_assume(other.z() == 0.0f);
        }
        if constexpr (N >= 4) {
            get<3>(v) = other.w();
        } else {
            ttauri_assume(other.w() == 0.0f);
        }
        for (int i = 4; i < N; ++i) {
            v[i] = T{};
        }
    }

    operator vec

    [[nodiscard]] T const &operator[](size_t i) const noexcept {
        return v[i];
    }

    [[nodiscard]] T &operator[](size_t i) noexcept {
        return v[i];
    }

#define BINARY_OP(op)\
    [[nodiscard]] friend simd operator op (simd const &lhs, simd const &rhs) noexcept {\
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

    [[nodiscard]] friend simd min(simd const &lhs, simd const &rhs) noexcept {
        simd r;
        for (int i = 0; i != N; ++i) {
            r[i] = lhs[i] < rhs[i] ? lhs[i] : rhs[i];
        }
        return r;
    }

    [[nodiscard]] friend simd max(simd const &lhs, simd const &rhs) noexcept {
        simd r;
        for (int i = 0; i != N; ++i) {
            r[i] = lhs[i] > rhs[i] ? lhs[i] : rhs[i];
        }
        return r;
    }

    [[nodiscard]] friend simd clamp(simd const &lhs, simd const &minimum, simd const &maximum) noexcept {
        simd r;
        for (int i = 0; i != N; ++i) {
            r[i] = 
                (lhs[i] < minimum[i]) ? minimum[i] :
                (lhs[i] > maximum[i]) ? maximum[i] :
                rhs[i];
        }
        return r;
    }
};

#undef BINARY_OP

using f32x4_t = simd<float,4>;


}
