// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../architecture.hpp"
#include "../concepts.hpp"
#include "../cast.hpp"
#include "../type_traits.hpp"
#include "../float16.hpp"

#if defined(TT_HAS_AVX)
#include "swizzle_avx.hpp"
#include <immintrin.h> // AVX, AVX2, FMA
#endif
#if defined(TT_HAS_SSE4_2)
#include <nmmintrin.h> // SSE4.2
#endif
#if defined(TT_HAS_SSE4_1)
#include "float16_sse4_1.hpp"
#include <smmintrin.h> // SSE4.1
#include <ammintrin.h> // SSE4A
#endif
#if defined(TT_HAS_SSSE3)
#include <tmmintrin.h> // SSSE3
#endif
#if defined(TT_HAS_SSE3)
#include <pmmintrin.h> // SSE3
#endif
#if defined(TT_HAS_SSE2)
#include <emmintrin.h> // SSE2
#endif
#if defined(TT_HAS_SSE)
#include <xmmintrin.h> // SSE
#endif

#include <cstdint>
#include <ostream>
#include <string>
#include <array>
#include <type_traits>
#include <concepts>
#include <bit>
#include <climits>

tt_warning_push();
// C4702 unreachable code: Suppressed due intrinsics and std::is_constant_evaluated()
tt_msvc_suppress(4702);

namespace tt::inline v1 {

template<numeric_limited T, std::size_t N>
struct numeric_array {
    using container_type = std::array<T, N>;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
    using difference_type = typename container_type::difference_type;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;
    using pointer = typename container_type::pointer;
    using const_pointer = typename container_type::const_pointer;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    constexpr static bool is_i8x1 = std::is_same_v<T, int8_t> && N == 1;
    constexpr static bool is_i8x2 = std::is_same_v<T, int8_t> && N == 2;
    constexpr static bool is_i8x4 = std::is_same_v<T, int8_t> && N == 4;
    constexpr static bool is_i8x8 = std::is_same_v<T, int8_t> && N == 8;
    constexpr static bool is_i8x16 = std::is_same_v<T, int8_t> && N == 16;
    constexpr static bool is_i8x32 = std::is_same_v<T, int8_t> && N == 32;
    constexpr static bool is_i8x64 = std::is_same_v<T, int8_t> && N == 64;
    constexpr static bool is_u8x1 = std::is_same_v<T, uint8_t> && N == 1;
    constexpr static bool is_u8x2 = std::is_same_v<T, uint8_t> && N == 2;
    constexpr static bool is_u8x4 = std::is_same_v<T, uint8_t> && N == 4;
    constexpr static bool is_u8x8 = std::is_same_v<T, uint8_t> && N == 8;
    constexpr static bool is_u8x16 = std::is_same_v<T, uint8_t> && N == 16;
    constexpr static bool is_u8x32 = std::is_same_v<T, uint8_t> && N == 32;
    constexpr static bool is_u8x64 = std::is_same_v<T, uint8_t> && N == 64;

    constexpr static bool is_i16x1 = std::is_same_v<T, int16_t> && N == 1;
    constexpr static bool is_i16x2 = std::is_same_v<T, int16_t> && N == 2;
    constexpr static bool is_i16x4 = std::is_same_v<T, int16_t> && N == 4;
    constexpr static bool is_i16x8 = std::is_same_v<T, int16_t> && N == 8;
    constexpr static bool is_i16x16 = std::is_same_v<T, int16_t> && N == 16;
    constexpr static bool is_i16x32 = std::is_same_v<T, int16_t> && N == 32;
    constexpr static bool is_u16x1 = std::is_same_v<T, uint16_t> && N == 1;
    constexpr static bool is_u16x2 = std::is_same_v<T, uint16_t> && N == 2;
    constexpr static bool is_u16x4 = std::is_same_v<T, uint16_t> && N == 4;
    constexpr static bool is_u16x8 = std::is_same_v<T, uint16_t> && N == 8;
    constexpr static bool is_u16x16 = std::is_same_v<T, uint16_t> && N == 16;
    constexpr static bool is_u16x32 = std::is_same_v<T, uint16_t> && N == 32;
    constexpr static bool is_f16x4 = std::is_same_v<T, float16> && N == 4;

    constexpr static bool is_i32x1 = std::is_same_v<T, int32_t> && N == 1;
    constexpr static bool is_i32x2 = std::is_same_v<T, int32_t> && N == 2;
    constexpr static bool is_i32x4 = std::is_same_v<T, int32_t> && N == 4;
    constexpr static bool is_i32x8 = std::is_same_v<T, int32_t> && N == 8;
    constexpr static bool is_i32x16 = std::is_same_v<T, int32_t> && N == 16;
    constexpr static bool is_u32x1 = std::is_same_v<T, uint32_t> && N == 1;
    constexpr static bool is_u32x2 = std::is_same_v<T, uint32_t> && N == 2;
    constexpr static bool is_u32x4 = std::is_same_v<T, uint32_t> && N == 4;
    constexpr static bool is_u32x8 = std::is_same_v<T, uint32_t> && N == 8;
    constexpr static bool is_u32x16 = std::is_same_v<T, uint32_t> && N == 16;
    constexpr static bool is_f32x1 = std::is_same_v<T, float> && N == 1;
    constexpr static bool is_f32x2 = std::is_same_v<T, float> && N == 2;
    constexpr static bool is_f32x4 = std::is_same_v<T, float> && N == 4;
    constexpr static bool is_f32x8 = std::is_same_v<T, float> && N == 8;
    constexpr static bool is_f32x16 = std::is_same_v<T, float> && N == 16;

    constexpr static bool is_i64x1 = std::is_same_v<T, int64_t> && N == 1;
    constexpr static bool is_i64x2 = std::is_same_v<T, int64_t> && N == 2;
    constexpr static bool is_i64x4 = std::is_same_v<T, int64_t> && N == 4;
    constexpr static bool is_i64x8 = std::is_same_v<T, int64_t> && N == 8;
    constexpr static bool is_u64x1 = std::is_same_v<T, uint64_t> && N == 1;
    constexpr static bool is_u64x2 = std::is_same_v<T, uint64_t> && N == 2;
    constexpr static bool is_u64x4 = std::is_same_v<T, uint64_t> && N == 4;
    constexpr static bool is_u64x8 = std::is_same_v<T, uint64_t> && N == 8;
    constexpr static bool is_f64x1 = std::is_same_v<T, double> && N == 1;
    constexpr static bool is_f64x2 = std::is_same_v<T, double> && N == 2;
    constexpr static bool is_f64x4 = std::is_same_v<T, double> && N == 4;
    constexpr static bool is_f64x8 = std::is_same_v<T, double> && N == 8;

    container_type v;

    constexpr numeric_array() noexcept : v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_i64x4 or is_u64x4 or is_i32x8 or is_u32x8 or is_i16x16 or is_u16x16 or is_i8x32 or is_u8x32) {
                _mm256_storeu_si256(reinterpret_cast<__m256i *>(v.data()), _mm256_setzero_si256());
                return;
            } else if constexpr (is_f64x4) {
                _mm256_storeu_pd(reinterpret_cast<__m256d *>(v.data()), _mm256_setzero_pd());
                return;
            } else if constexpr (is_f32x8) {
                _mm256_storeu_ps(v.data(), _mm256_setzero_ps());
                return;
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_i64x2 or is_u64x2 or is_i32x4 or is_u32x4 or is_i16x8 or is_u16x8 or is_i8x16 or is_u8x16) {
                _mm_storeu_si128(reinterpret_cast<__m128i *>(v.data()), _mm_setzero_si128());
                return;
            } else if constexpr (is_f64x2) {
                _mm_storeu_pd(reinterpret_cast<__m128d *>(v.data()), _mm_setzero_pd());
                return;
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                _mm_storeu_ps(v.data(), _mm_setzero_ps());
                return;
            }
#endif
        }

        for (auto i = 0_uz; i != N; ++i) {
            v[i] = T{};
        }
    }

    constexpr numeric_array(numeric_array const &rhs) noexcept = default;
    constexpr numeric_array(numeric_array &&rhs) noexcept = default;
    constexpr numeric_array &operator=(numeric_array const &rhs) noexcept = default;
    constexpr numeric_array &operator=(numeric_array &&rhs) noexcept = default;

    template<numeric_limited U, std::size_t M>
    [[nodiscard]] constexpr explicit numeric_array(numeric_array<U, M> const &other) noexcept : v()
    {
        if (!std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4 and other.is_f32x4) {
                v = numeric_array{_mm256_cvteps_pd(other.reg())};
                return;
            } else if constexpr (is_f64x4 and other.is_i32x4) {
                v = numeric_array{_mm256_cvtepi32_pd(other.reg())};
                return;
            } else if constexpr (is_f32x4 and other.is_f64x4) {
                v = numeric_array{_mm256_cvtpd_ps(other.reg())};
                return;
            } else if constexpr (is_i32x4 and other.is_f64x4) {
                v = numeric_array{_mm256_cvtpd_epi32(other.reg())};
                return;
            } else if constexpr (is_i32x8 and other.is_f32x8) {
                v = numeric_array{_mm256_cvtps_epi32(other.reg())};
                return;
            } else if constexpr (is_f32x8 and other.is_i32x8) {
                v = numeric_array{_mm256_cvtepi32_ps(other.reg())};
                return;
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_u8x4 and other.is_f32x4) {
                ttlet i32_4 = _mm_cvtps_epi32(other.reg());
                ttlet i16_8 = _mm_packs_epi32(i32_4, _mm_setzero_si128());
                ttlet u8_16 = _mm_packus_epi16(i16_8, _mm_setzero_si128());
                v = numeric_array{u8_16};
                return;
            } else if constexpr (is_i64x4 and other.is_i32x4) {
                v = numeric_array{_mm_cvtepi32_epi64(other.reg())};
                return;
            } else if constexpr (is_i64x4 and other.is_i16x8) {
                v = numeric_array{_mm_cvtepi16_epi64(other.reg())};
                return;
            } else if constexpr (is_i32x4 and other.is_i16x8) {
                v = numeric_array{_mm_cvtepi16_epi32(other.reg())};
                return;
            } else if constexpr (is_i64x2 and other.is_i8x16) {
                v = numeric_array{_mm_cvtepi8_epi64(other.reg())};
                return;
            } else if constexpr (is_i32x4 and other.is_i8x16) {
                v = numeric_array{_mm_cvtepi8_epi32(other.reg())};
                return;
            } else if constexpr (is_i16x8 and other.is_i8x16) {
                v = numeric_array{_mm_cvtepi8_epi16(other.reg())};
                return;
            } else if constexpr (is_f16x4 and other.is_f32x4) {
                v = numeric_array{_mm_cvtps_ph_sse4_1(other.reg())};
                return;
            } else if constexpr (is_f32x4 and other.is_f16x4) {
                v = numeric_array{_mm_cvtph_ps_sse2(other.reg())};
                return;
            }

#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2 and other.is_i32x4) {
                v = numeric_array{_mm_cvtepi32_pd(other.reg())};
                return;
            } else if constexpr (is_f32x4 and other.is_i32x4) {
                v = numeric_array{_mm_cvtepi32_ps(other.reg())};
                return;
            } else if constexpr (is_i32x4 and other.is_f32x4) {
                v = numeric_array{_mm_cvtps_epi32(other.reg())};
                return;
            }
#endif
        }

        for (std::size_t i = 0; i != N; ++i) {
            if (i < M) {
                if constexpr (std::is_integral_v<T> and std::is_floating_point_v<U>) {
                    // SSE conversion round floats before converting to integer.
                    v[i] = static_cast<value_type>(std::round(other[i]));
                } else {
                    v[i] = static_cast<value_type>(other[i]);
                }
            } else {
                v[i] = T{};
            }
        }
    }

    template<numeric_limited U, std::size_t M>
    [[nodiscard]] constexpr explicit numeric_array(numeric_array<U, M> const &other1, numeric_array<U, M> const &other2) noexcept
        :
        v()
    {
        if (!std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4 and other1.is_f64x2 and other2.is_f64x2) {
                v = numeric_array{_mm256_set_m128d(other2.reg(), other1.reg())};
            } else if constexpr (is_f32x8 and other1.is_f32x4 and other2.is_f32x4) {
                v = numeric_array{_mm256_set_m128(other2.reg(), other1.reg())};
            } else if constexpr (
                std::is_integral_v<T> and std::is_integral_v<U> and (sizeof(T) * N == 32) and (sizeof(U) * M == 16)) {
                v = numeric_array{_mm256_set_m128i(other2.reg(), other1.reg())};
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_u16x8 and other1.is_u32x4 and other2.is_u32x4) {
                v = numeric_array{_mm_packus_epu32(other2.reg(), other1.reg())};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_i16x8 and other1.is_i32x4 and other2.is_i32x4) {
                v = numeric_array{_mm_packs_epi32(other2.reg(), other1.reg())};
            } else if constexpr (is_i8x16 and other1.is_i16x8 and other2.is_i16x8) {
                v = numeric_array{_mm_packs_epi16(other2.reg(), other1.reg())};
            } else if constexpr (is_u8x16 and other1.is_u16x8 and other2.is_u16x8) {
                v = numeric_array{_mm_packus_epu16(other2.reg(), other1.reg())};
            }
#endif
        }

        for (std::size_t i = 0; i != N; ++i) {
            if (i < M) {
                if constexpr (std::is_integral_v<T> and std::is_floating_point_v<U>) {
                    // SSE conversion round floats before converting to integer.
                    v[i] = static_cast<value_type>(std::round(other1[i]));
                } else {
                    v[i] = static_cast<value_type>(other1[i]);
                }
            } else if (i < M * 2) {
                if constexpr (std::is_integral_v<T> and std::is_floating_point_v<U>) {
                    // SSE conversion round floats before converting to integer.
                    v[i] = static_cast<value_type>(std::round(other2[i - M]));
                } else {
                    v[i] = static_cast<value_type>(other2[i - M]);
                }
            } else {
                v[i] = U{};
            }
        }
    }

    [[nodiscard]] constexpr explicit numeric_array(T const &x) noexcept : v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                *this = numeric_array{_mm_set_ss(x)};
                return;
            }
#endif
        }
        get<0>(v) = x;
    }

    [[nodiscard]] constexpr explicit numeric_array(T const &x, T const &y) noexcept requires(N >= 2) : v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE2)
            if constexpr (is_i32x4) {
                *this = numeric_array{_mm_set_epi32(0, 0, y, x)};
                return;
            }
#endif
        }
        get<0>(v) = x;
        get<1>(v) = y;
    }

    [[nodiscard]] constexpr explicit numeric_array(T const &x, T const &y, T const &z) noexcept requires(N >= 3) : v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE2)
            if constexpr (is_i32x4) {
                *this = numeric_array{_mm_set_epi32(0, z, y, x)};
                return;
            }
#endif
        }
        get<0>(v) = x;
        get<1>(v) = y;
        get<2>(v) = z;
    }

    [[nodiscard]] constexpr explicit numeric_array(T const &x, T const &y, T const &z, T const &w) noexcept requires(N >= 4) : v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE2)
            if constexpr (is_i32x4) {
                *this = numeric_array{_mm_set_epi32(w, z, y, x)};
                return;
            }
#endif
        }
        get<0>(v) = x;
        get<1>(v) = y;
        get<2>(v) = z;
        get<3>(v) = w;
    }

    [[nodiscard]] static constexpr numeric_array broadcast(T rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_set1_pd(rhs)};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_set1_ps(rhs)};
            } else if constexpr (is_i64x4) {
                return numeric_array{_mm256_set1_epi64x(rhs)};
            } else if constexpr (is_i32x8) {
                return numeric_array{_mm256_set1_epi32(rhs)};
            } else if constexpr (is_i16x16) {
                return numeric_array{_mm256_set1_epi16(rhs)};
            } else if constexpr (is_i8x32) {
                return numeric_array{_mm256_set1_epi8(rhs)};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_set1_pd(rhs)};
            } else if constexpr (is_i64x2) {
                return numeric_array{_mm_set1_epi64x(rhs)};
            } else if constexpr (is_i32x4) {
                return numeric_array{_mm_set1_epi32(rhs)};
            } else if constexpr (is_i16x8) {
                return numeric_array{_mm_set1_epi16(rhs)};
            } else if constexpr (is_i8x16) {
                return numeric_array{_mm_set1_epi8(rhs)};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_set1_ps(rhs)};
            }
#endif
        }
        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = rhs;
        }
        return r;
    }

    [[nodiscard]] static constexpr numeric_array epsilon() noexcept
    {
        if constexpr (std::is_floating_point_v<T>) {
            return broadcast(std::numeric_limits<T>::min());
        } else {
            return broadcast(T{0});
        }
    }

    [[nodiscard]] numeric_array(std::array<T, N> const &rhs) noexcept : v(rhs) {}

    numeric_array &operator=(std::array<T, N> const &rhs) noexcept
    {
        v = rhs;
        return *this;
    }

    [[nodiscard]] operator std::array<T, N>() const noexcept
    {
        return v;
    }

#if defined(TT_HAS_SSE2)
    [[nodiscard]] __m128i reg() const noexcept requires(std::is_integral_v<T> and sizeof(T) * N == 16)
    {
        return _mm_loadu_si128(reinterpret_cast<__m128i const *>(v.data()));
    }

    [[nodiscard]] __m128i reg() const noexcept requires(is_f16x4)
    {
        return _mm_set_epi16(0, 0, 0, 0, get<3>(v).get(), get<2>(v).get(), get<1>(v).get(), get<0>(v).get());
    }
#endif

#if defined(TT_HAS_SSE2)
    [[nodiscard]] __m128 reg() const noexcept requires(is_f32x4)
    {
        return _mm_loadu_ps(v.data());
    }
#endif

#if defined(TT_HAS_SSE2)
    [[nodiscard]] __m128d reg() const noexcept requires(is_f64x2)
    {
        return _mm_loadu_pd(v.data());
    }
#endif

#if defined(TT_HAS_SSE2)
    [[nodiscard]] explicit numeric_array(__m128i const &rhs) noexcept requires(std::is_integral_v<T> and sizeof(T) * N == 16)
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(v.data()), rhs);
    }
#endif

#if defined(TT_HAS_SSE4_1)
    [[nodiscard]] explicit numeric_array(__m128i const &rhs) noexcept requires(is_f16x4) :
        v(std::bit_cast<decltype(v)>(_mm_extract_epi64(rhs, 0)))
    {
    }
#endif

#if defined(TT_HAS_SSE4_1)
    [[nodiscard]] explicit numeric_array(__m128i const &rhs) noexcept requires(is_u8x4) :
        v(std::bit_cast<decltype(v)>(_mm_extract_epi32(rhs, 0)))
    {
    }
#endif

#if defined(TT_HAS_SSE2)
    [[nodiscard]] explicit numeric_array(__m128 const &rhs) noexcept requires(is_f32x4)
    {
        _mm_storeu_ps(v.data(), rhs);
    }
#endif

#if defined(TT_HAS_SSE2)
    [[nodiscard]] explicit numeric_array(__m128d const &rhs) noexcept requires(is_f64x2)
    {
        _mm_storeu_pd(v.data(), rhs);
    }
#endif

#if defined(TT_HAS_SSE2)
    numeric_array &operator=(__m128i const &rhs) noexcept requires(std::is_integral_v<T> and sizeof(T) * N == 16)
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(v.data()), rhs);
        return *this;
    }
#endif

#if defined(TT_HAS_SSE2)
    numeric_array &operator=(__m128 const &rhs) noexcept requires(is_f32x4)
    {
        _mm_storeu_ps(v.data(), rhs);
        return *this;
    }
#endif

#if defined(TT_HAS_SSE2)
    numeric_array &operator=(__m128d const &rhs) noexcept requires(is_f64x2)
    {
        _mm_storeu_pd(v.data(), rhs);
        return *this;
    }
#endif

#if defined(TT_HAS_AVX)
    [[nodiscard]] __m256i reg() const noexcept requires(std::is_integral_v<T> and sizeof(T) * N == 32)
    {
        return _mm256_loadu_si256(reinterpret_cast<__m256i const *>(v.data()));
    }
#endif

#if defined(TT_HAS_AVX)
    [[nodiscard]] __m256 reg() const noexcept requires(is_f32x8)
    {
        return _mm256_loadu_ps(v.data());
    }
#endif

#if defined(TT_HAS_AVX)
    [[nodiscard]] __m256d reg() const noexcept requires(is_f64x4)
    {
        return _mm256_loadu_pd(v.data());
    }
#endif

#if defined(TT_HAS_AVX)
    [[nodiscard]] explicit numeric_array(__m256i const &rhs) noexcept requires(std::is_integral_v<T> and sizeof(T) * N == 32)
    {
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(v.data()), rhs);
    }
#endif

#if defined(TT_HAS_AVX)
    [[nodiscard]] explicit numeric_array(__m256 const &rhs) noexcept requires(is_f32x8)
    {
        _mm256_storeu_ps(v.data(), rhs);
    }
#endif

#if defined(TT_HAS_AVX)
    [[nodiscard]] explicit numeric_array(__m256d const &rhs) noexcept requires(is_f64x4)
    {
        _mm256_storeu_pd(v.data(), rhs);
    }
#endif

#if defined(TT_HAS_AVX)
    numeric_array &operator=(__m256i const &rhs) noexcept requires(std::is_integral_v<T> and sizeof(T) * N == 32)
    {
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(v.data()), rhs);
        return *this;
    }
#endif

#if defined(TT_HAS_AVX)
    numeric_array &operator=(__m256 const &rhs) noexcept requires(is_f32x8)
    {
        _mm256_storeu_ps(v.data(), rhs);
        return *this;
    }
#endif

#if defined(TT_HAS_AVX)
    numeric_array &operator=(__m256d const &rhs) noexcept requires(is_f64x4)
    {
        _mm256_storeu_pd(v.data(), rhs);
        return *this;
    }
#endif

    template<typename Other>
    [[nodiscard]] constexpr friend Other bit_cast(numeric_array const &rhs) noexcept
        requires(sizeof(Other) == sizeof(container_type))
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE2)
            if constexpr (Other::is_f32x4 and std::is_integral_v<T>) {
                return Other{_mm_castsi128_ps(rhs.reg())};
            } else if constexpr (Other::is_f32x4 and is_f64x2) {
                return Other{_mm_castpd_ps(rhs.reg())};
            } else if constexpr (Other::is_f64x2 and std::is_integral_v<T>) {
                return Other{_mm_castsi128_pd(rhs.reg())};
            } else if constexpr (Other::is_f64x2 and is_f32x4) {
                return Other{_mm_castps_pd(rhs.reg())};
            } else if constexpr (std::is_integral_v<Other::value_type> and is_f32x4) {
                return Other{_mm_castps_si128(rhs.reg())};
            } else if constexpr (std::is_integral_v<Other::value_type> and is_f64x2) {
                return Other{_mm_castpd_si128(rhs.reg())};
            } else if constexpr (std::is_integral_v<Other::value_type> and std::is_integral_v<T>) {
                return Other{rhs.reg()};
            }
#endif
        }
        return std::bit_cast<Other>(rhs);
    }

    /** Interleave the first words in both arrays.
     */
    static constexpr numeric_array interleave_lo(numeric_array a, numeric_array b) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_unpacklo_pd(a.reg(), b.reg())};
            } else if constexpr (is_i64x2 or is_u64x2) {
                return numeric_array{_mm_unpacklo_epi64(a.reg(), b.reg())};
            } else if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{_mm_unpacklo_epi32(a.reg(), b.reg())};
            } else if constexpr (is_i16x8 or is_u16x8) {
                return numeric_array{_mm_unpacklo_epi16(a.reg(), b.reg())};
            } else if constexpr (is_i8x16 or is_u8x16) {
                return numeric_array{_mm_unpacklo_epi8(a.reg(), b.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_unpacklo_ps(a.reg(), b.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = (i % 2 == 0) ? a[i / 2] : b[i / 2];
        }
        return r;
    }

    /** Load a numeric array from memory.
     * @param ptr A Pointer to an array of values in memory.
     * @return A numeric array.
     */
    template<std::size_t S>
    [[nodiscard]] static constexpr numeric_array load(std::byte const *ptr) noexcept
    {
        auto r = numeric_array{};
        std::memcpy(&r, ptr, S);
        return r;
    }

    /** Load a numeric array from memory.
     * @param ptr A Pointer to an array of values in memory.
     * @return A numeric array.
     */
    [[nodiscard]] static constexpr numeric_array load(std::byte const *ptr) noexcept
    {
        auto r = numeric_array{};
        std::memcpy(&r, ptr, sizeof(r));
        return r;
    }

    /** Load a numeric array from memory.
     * @param ptr A Pointer to an array of values in memory.
     * @return A numeric array.
     */
    [[nodiscard]] static constexpr numeric_array load(T const *ptr) noexcept
    {
        auto r = numeric_array{};
        std::memcpy(&r, ptr, sizeof(r));
        return r;
    }

    template<std::size_t S>
    constexpr void store(std::byte *ptr) const noexcept
    {
        std::memcpy(ptr, this, S);
    }

    /** Store a numeric array into memory.
     * @param [out]ptr A pointer to where the numeric array should be stored into memory.
     */
    constexpr void store(std::byte *ptr) const noexcept
    {
        store<sizeof(*this)>(ptr);
    }

    /** Check if the vector is non-zero.
     * @return True if at least one element is non-zero.
     */
    constexpr explicit operator bool() const noexcept
    {
        if constexpr (std::is_floating_point_v<T>) {
            ttlet ep = epsilon();
            // check if any of the elements is outside of epsilon range,
            return static_cast<bool>(gt(-ep, *this) | gt(*this, ep));
        } else {
            return static_cast<bool>(ne(*this, T{0}));
        }
    }

    [[nodiscard]] constexpr T const &operator[](std::size_t i) const noexcept
    {
        static_assert(std::endian::native == std::endian::little, "Indices need to be reversed on big endian machines");
        tt_axiom(i < N);
        return v[i];
    }

    [[nodiscard]] constexpr T &operator[](std::size_t i) noexcept
    {
        static_assert(std::endian::native == std::endian::little, "Indices need to be reversed on big endian machines");
        tt_axiom(i < N);
        return v[i];
    }

    [[nodiscard]] constexpr reference front() noexcept
    {
        return v.front();
    }

    [[nodiscard]] constexpr const_reference front() const noexcept
    {
        return v.front();
    }

    [[nodiscard]] constexpr reference back() noexcept
    {
        return v.back();
    }

    [[nodiscard]] constexpr const_reference back() const noexcept
    {
        return v.back();
    }

    [[nodiscard]] constexpr pointer data() noexcept
    {
        return v.data();
    }

    [[nodiscard]] constexpr const_pointer data() const noexcept
    {
        return v.data();
    }

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return v.begin();
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return v.begin();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return v.cbegin();
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return v.end();
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return v.end();
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return v.cend();
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return v.empty();
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return v.size();
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return v.max_size();
    }

    constexpr bool is_point() const noexcept
    {
        return v.back() != T{};
    }

    constexpr bool is_vector() const noexcept
    {
        return v.back() == T{};
    }

    constexpr bool is_opaque() const noexcept
    {
        return a() == T{1};
    }

    constexpr bool is_transparent() const noexcept
    {
        return a() == T{0};
    }

    [[nodiscard]] constexpr T const &x() const noexcept requires(N >= 1)
    {
        return std::get<0>(v);
    }

    [[nodiscard]] constexpr T const &y() const noexcept requires(N >= 2)
    {
        return std::get<1>(v);
    }

    [[nodiscard]] constexpr T const &z() const noexcept requires(N >= 3)
    {
        return std::get<2>(v);
    }

    [[nodiscard]] constexpr T const &w() const noexcept requires(N >= 4)
    {
        return std::get<3>(v);
    }

    [[nodiscard]] constexpr T &x() noexcept requires(N >= 1)
    {
        return std::get<0>(v);
    }

    [[nodiscard]] constexpr T &y() noexcept requires(N >= 2)
    {
        return std::get<1>(v);
    }

    [[nodiscard]] constexpr T &z() noexcept requires(N >= 3)
    {
        return std::get<2>(v);
    }

    [[nodiscard]] constexpr T &w() noexcept requires(N >= 4)
    {
        return std::get<3>(v);
    }

    [[nodiscard]] constexpr T const &r() const noexcept requires(N >= 1)
    {
        return std::get<0>(v);
    }

    [[nodiscard]] constexpr T const &g() const noexcept requires(N >= 2)
    {
        return std::get<1>(v);
    }

    [[nodiscard]] constexpr T const &b() const noexcept requires(N >= 3)
    {
        return std::get<2>(v);
    }

    [[nodiscard]] constexpr T const &a() const noexcept requires(N >= 4)
    {
        return std::get<3>(v);
    }

    [[nodiscard]] constexpr T &r() noexcept requires(N >= 1)
    {
        return std::get<0>(v);
    }

    [[nodiscard]] constexpr T &g() noexcept requires(N >= 2)
    {
        return std::get<1>(v);
    }

    [[nodiscard]] constexpr T &b() noexcept requires(N >= 3)
    {
        return std::get<2>(v);
    }

    [[nodiscard]] constexpr T &a() noexcept requires(N >= 4)
    {
        return std::get<3>(v);
    }

    [[nodiscard]] constexpr T const &width() const noexcept requires(N >= 1)
    {
        return std::get<0>(v);
    }

    [[nodiscard]] constexpr T const &height() const noexcept requires(N >= 2)
    {
        return std::get<1>(v);
    }

    [[nodiscard]] constexpr T const &depth() const noexcept requires(N >= 3)
    {
        return std::get<2>(v);
    }

    [[nodiscard]] constexpr T &width() noexcept requires(N >= 1)
    {
        return std::get<0>(v);
    }

    [[nodiscard]] constexpr T &height() noexcept requires(N >= 2)
    {
        return std::get<1>(v);
    }

    [[nodiscard]] constexpr T &depth() noexcept requires(N >= 3)
    {
        return std::get<2>(v);
    }

    constexpr numeric_array &operator<<=(unsigned int rhs) noexcept
    {
        return *this = *this << rhs;
    }

    constexpr numeric_array &operator>>=(unsigned int rhs) noexcept
    {
        return *this = *this >> rhs;
    }

    constexpr numeric_array &operator|=(numeric_array const &rhs) noexcept
    {
        return *this = *this | rhs;
    }

    constexpr numeric_array &operator|=(T const &rhs) noexcept
    {
        return *this = *this | rhs;
    }

    constexpr numeric_array &operator&=(numeric_array const &rhs) noexcept
    {
        return *this = *this & rhs;
    }

    constexpr numeric_array &operator&=(T const &rhs) noexcept
    {
        return *this = *this & rhs;
    }

    constexpr numeric_array &operator^=(numeric_array const &rhs) noexcept
    {
        return *this = *this ^ rhs;
    }

    constexpr numeric_array &operator^=(T const &rhs) noexcept
    {
        return *this = *this ^ rhs;
    }

    constexpr numeric_array &operator+=(numeric_array const &rhs) noexcept
    {
        return *this = *this + rhs;
    }

    constexpr numeric_array &operator+=(T const &rhs) noexcept
    {
        return *this = *this + rhs;
    }

    constexpr numeric_array &operator-=(numeric_array const &rhs) noexcept
    {
        return *this = *this - rhs;
    }

    constexpr numeric_array &operator-=(T const &rhs) noexcept
    {
        return *this = *this - rhs;
    }

    constexpr numeric_array &operator*=(numeric_array const &rhs) noexcept
    {
        return *this = *this * rhs;
    }

    constexpr numeric_array &operator*=(T const &rhs) noexcept
    {
        return *this = *this * rhs;
    }

    constexpr numeric_array &operator/=(numeric_array const &rhs) noexcept
    {
        return *this = *this / rhs;
    }

    constexpr numeric_array &operator/=(T const &rhs) noexcept
    {
        return *this = *this / rhs;
    }

    constexpr numeric_array &operator%=(numeric_array const &rhs) noexcept
    {
        return *this = *this % rhs;
    }

    constexpr numeric_array &operator%=(T const &rhs) noexcept
    {
        return *this = *this % rhs;
    }

    constexpr static ssize_t get_zero = -1;
    constexpr static ssize_t get_one = -2;

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array
     */
    template<std::size_t I>
    [[nodiscard]] friend constexpr T &get(numeric_array &rhs) noexcept
    {
        static_assert(I < N, "Index out of bounds");
        return std::get<I>(rhs.v);
    }

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array,
     *           or the special indices: -1 yields a literal 0, -2 yields a literal 1.
     */
    template<ssize_t I>
    [[nodiscard]] friend constexpr T get(numeric_array &&rhs) noexcept
    {
        static_assert(std::endian::native == std::endian::little, "Indices need to be reversed on big endian machines");
        static_assert(I >= -2 && I < narrow_cast<ssize_t>(N), "Index out of bounds");
        if constexpr (I == get_zero) {
            return T{0};
        } else if constexpr (I == get_one) {
            return T{1};
        } else {
            return std::get<I>(rhs.v);
        }
    }

    /** Extract an element from the array.
     *
     * @tparam I The index into the array.
     * @param rhs The vector to extract the value from.
     * @return The value extracted.
     */
    template<std::size_t I>
    [[nodiscard]] constexpr friend T extract(numeric_array const &rhs) noexcept
    {
        static_assert(I < N);

        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i16x16 or is_u16x16) {
                return static_cast<T>(_mm256_extract_epi16(rhs.v.reg(), I));
            } else if constexpr (is_i8x32 or is_u8x32) {
                return static_cast<T>(_mm256_extract_epi8(rhs.v.reg(), I));
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return bit_cast<T>(_mm256_extract_epi64(_mm256_castpd_si256(rhs.v.reg()), I));
            } else if constexpr (is_f32x8) {
                return bit_cast<T>(_mm256_extract_epi32(_mm256_castps_si256(rhs.v.reg()), I));
            } else if constexpr (is_i64x4 or is_u64x4) {
                return static_cast<T>(_mm256_extract_epi64(rhs.v.reg(), I));
            } else if constexpr (is_i32x8 or is_u32x8) {
                return static_cast<T>(_mm256_extract_epi32(rhs.v.reg(), I));
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f64x2) {
                return bit_cast<T>(_mm_extract_epi64(_mm_castpd_si128(rhs.v.reg()), I));
            } else if constexpr (is_f32x4) {
                return std::bit_cast<T>(_mm_extract_ps(rhs.v.reg(), I));
            } else if constexpr (is_i64x2 or is_u64x2) {
                return static_cast<T>(_mm_extract_epi64(rhs.v.reg(), I));
            } else if constexpr (is_i32x4 or is_u32x4) {
                return static_cast<T>(_mm_extract_epi32(rhs.v.reg(), I));
            } else if constexpr (is_i8x16 or is_u8x16) {
                return static_cast<T>(_mm_extract_epi8(rhs.v.reg(), I));
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_i16x8 or is_u16x8) {
                return static_cast<T>(_mm_extract_epi16(rhs.v.reg(), I));
            }
#endif
        }

        return get<I>(rhs);
    }

    /** Insert a value in the array.
     *
     * @tparam I The index into the array.
     * @tparam ZeroMask When a bit in the mask is '1' set that element to zero.
     * @param lhs The vector to insert the value into.
     * @param rhs The value to insert.
     * @return The vector with the inserted value.
     */
    template<std::size_t I, std::size_t ZeroMask = 0>
    [[nodiscard]] constexpr friend numeric_array insert(numeric_array const &lhs, T rhs) noexcept
        requires(is_f32x4 or is_i32x4 or is_u32x4)
    {
        static_assert(I < N);
        static_assert(ZeroMask <= ((1 << N) - 1));

        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f32x4) {
                constexpr int imm8 = (I << 4) | ZeroMask;
                return numeric_array{_mm_insert_ps(lhs.reg(), _mm_set_ss(rhs), imm8)};
            } else if constexpr (is_i32x4 or is_u32x4) {
                constexpr int imm8 = (I << 4) | ZeroMask;
                return numeric_array{
                    _mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(lhs.reg()), _mm_castsi128_ps(_mm_set1_epi32(rhs)), imm8))};
            }
#endif
        }

        auto r = lhs;
        std::get<I>(r.v) = rhs;
        for (std::size_t i = 0; i != N; ++i) {
            if ((ZeroMask >> i) & 1) {
                r.v[i] = T{};
            }
        }
        return r;
    }

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array,
     *           or the special indices: -1 yields a literal 0, -2 yields a literal 1.
     */
    template<ssize_t I>
    [[nodiscard]] friend constexpr T get(numeric_array const &rhs) noexcept
    {
        static_assert(std::endian::native == std::endian::little, "Indices need to be reversed on big endian machines");
        static_assert(I >= -2 && I < narrow_cast<ssize_t>(N), "Index out of bounds");
        if constexpr (I == get_zero) {
            return T{0};
        } else if constexpr (I == get_one) {
            return T{1};
        } else {
            return std::get<I>(rhs.v);
        }
    }

    /** Set individual elements to zero.
     *
     * @tparam Mask bit mask where '1' means to zero, '0' to keep original.
     */
    template<std::size_t Mask = ~std::size_t{0}>
    [[nodiscard]] friend constexpr numeric_array zero(numeric_array rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_insert_ps(rhs.reg(), rhs.reg(), Mask)};
            } else if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{
                    _mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(rhs.reg()), _mm_castsi128_ps(rhs.reg()), Mask))};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            if (static_cast<bool>((Mask >> i) & 1)) {
                r.v[i] = T{0};
            } else {
                r.v[i] = rhs.v[i];
            }
        }
        return r;
    }

    template<std::size_t Mask>
    [[nodiscard]] friend constexpr numeric_array blend(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i32x8) {
                return numeric_array{_mm256_blend_epi32(lhs.reg(), rhs.reg(), Mask)};
            } else if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{_mm_blend_epi32(lhs.reg(), rhs.reg(), Mask)};
            } else if constexpr (is_i16x16 or is_u16x16) {
                return numeric_array{_mm256_blend_epi16(lhs.reg(), rhs.reg(), Mask)};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_blend_pd(lhs.reg(), rhs.reg(), Mask)};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_blend_ps(lhs.reg(), rhs.reg(), Mask)};
            } else if constexpr (is_i64x4 or is_u64x4) {
                return numeric_array{
                    _mm256_castpd_si256(_mm256_blend_pd(_mm256_castsi256_pd(lhs.reg()), _mm256_castsi256_pd(rhs.reg()), Mask))};
            } else if constexpr (is_i32x8 or is_u32x8) {
                return numeric_array{
                    _mm256_castps_si256(_mm256_blend_ps(_mm256_castsi256_ps(lhs.reg()), _mm256_castsi256_ps(rhs.reg()), Mask))};
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_blend_pd(lhs.reg(), rhs.reg(), Mask)};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_blend_ps(lhs.reg(), rhs.reg(), Mask)};
            } else if constexpr (is_i64x2 or is_u64x2) {
                return numeric_array{
                    _mm_castpd_si128(_mm_blend_pd(_mm_castsi128_pd(lhs.reg()), _mm_castsi128_pd(rhs.reg()), Mask))};
            } else if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{
                    _mm_castps_si128(_mm_blend_ps(_mm_castsi128_ps(lhs.reg()), _mm_castsi128_ps(rhs.reg()), Mask))};
            } else if constexpr (is_i16x8 or is_u16x8) {
                return numeric_array{_mm_blend_epi16(lhs.reg(), rhs.reg(), Mask)};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = static_cast<bool>((Mask >> i) & 1) ? rhs[i] : lhs[i];
        }
        return r;
    }

    /** Blend the values using a dynamic mask.
     */
    [[nodiscard]] friend constexpr numeric_array blend(numeric_array const &a, numeric_array const &b, numeric_array const &mask)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i8x32 or is_u8x32) {
                return numeric_array{_mm256_blendv_epi8(a.reg(), b.reg(), mask.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_blendv_pd(a.reg(), b.reg(), mask.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_blendv_ps(a.reg(), b.reg(), mask.reg())};
            } else if constexpr (is_i64x4 or is_u64x4) {
                return numeric_array{_mm256_castpd_si256(_mm256_blendv_pd(
                    _mm256_castsi256_pd(a.reg()), _mm256_castsi256_pd(b.reg()), _mm256_castsi256_pd(mask.reg())))};
            } else if constexpr (is_i32x8 or is_u32x8) {
                return numeric_array{_mm256_castps_si256(_mm256_blendv_ps(
                    _mm256_castsi256_ps(a.reg()), _mm256_castsi256_ps(b.reg()), _mm256_castsi256_ps(mask.reg())))};
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_blendv_pd(a.reg(), b.reg(), mask.reg())};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_blendv_ps(a.reg(), b.reg(), mask.reg())};
            } else if constexpr (is_i64x2 or is_u64x2) {
                return numeric_array{_mm_castpd_si128(
                    _mm_blendv_pd(_mm_castsi128_pd(a.reg()), _mm_castsi128_pd(b.reg()), _mm_castsi128_pd(mask.reg())))};
            } else if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{_mm_castps_si128(
                    _mm_blendv_ps(_mm_castsi128_ps(a.reg()), _mm_castsi128_ps(b.reg()), _mm_castsi128_ps(mask.reg())))};
            } else if constexpr (is_i8x16 or is_u8x16) {
                return numeric_array{_mm_blendv_epi8(a.reg(), b.reg(), mask.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = mask[i] != T{0} ? b[i] : a[i];
        }
        return r;
    }

    /** Negate individual elements.
     *
     * @tparam Mask bit mask where '1' means to negate, '0' to keep original.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr numeric_array neg(numeric_array rhs) noexcept
    {
        return blend<Mask>(rhs, -rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &rhs) noexcept
    {
        return T{0} - rhs;
    }

    [[nodiscard]] friend constexpr numeric_array abs(numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i32x8) {
                return numeric_array{_mm256_abs_epi32(rhs.reg())};
            } else if constexpr (is_i16x16) {
                return numeric_array{_mm256_abs_epi16(rhs.reg())};
            } else if constexpr (is_i8x32) {
                return numeric_array{_mm256_abs_epi8(rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSSE3)
            if constexpr (is_i32x4) {
                return numeric_array{_mm_abs_epi32(rhs.reg())};
            } else if constexpr (is_i16x8) {
                return numeric_array{_mm_abs_epi16(rhs.reg())};
            } else if constexpr (is_i8x16) {
                return numeric_array{_mm_abs_epi8(rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_castsi128_ps(_mm_srli_epi64(_mm_slli_epi64(_mm_castpd_si128(rhs.reg()), 1), 1))};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_castsi128_ps(_mm_srli_epi32(_mm_slli_epi32(_mm_castps_si128(rhs.reg()), 1), 1))};
            }
#endif
        }

        return max(rhs, -rhs);
    }

    [[nodiscard]] friend constexpr numeric_array rcp(numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f32x8) {
                return numeric_array{_mm256_rcp_ps(rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_rcp_ps(rhs.reg())};
            }
#endif
        }

        return T{1} / rhs;
    }

    [[nodiscard]] friend constexpr numeric_array sqrt(numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_sqrt_pd(rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_sqrt_ps(rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_sqrt_pd(rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_sqrt_ps(rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::sqrt(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array rcp_sqrt(numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f32x8) {
                return numeric_array{_mm256_rsqrt_ps(rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_rsqrt_ps(rhs.reg())};
            }
#endif
        }

        return rcp(sqrt(rhs));
    }

    [[nodiscard]] friend constexpr numeric_array floor(numeric_array const &rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_floor_pd(rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_floor_ps(rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_floor_pd(rhs.reg())};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_floor_ps(rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::floor(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array ceil(numeric_array const &rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_ceil_pd(rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_ceil_ps(rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_ceil_pd(rhs.reg())};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_ceil_ps(rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::ceil(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array round(numeric_array const &rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_round_pd(rhs.reg(), _MM_FROUND_CUR_DIRECTION)};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_round_ps(rhs.reg(), _MM_FROUND_CUR_DIRECTION)};
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_round_pd(rhs.reg(), _MM_FROUND_CUR_DIRECTION)};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_round_ps(rhs.reg(), _MM_FROUND_CUR_DIRECTION)};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::round(rhs.v[i]);
        }
        return r;
    }

    /** Take a dot product.
     *
     * @tparam Mask A mask for which elements participate in the dot product.
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     * @return Result of the dot product.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr T dot(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f64x2) {
                return std::bit_cast<double>(_mm_extract_epi64(_mm_dp_pd(lhs.reg(), rhs.reg(), (Mask << 4) | 0xf), 0));
            } else if constexpr (is_f32x4) {
                return std::bit_cast<float>(_mm_extract_ps(_mm_dp_ps(lhs.reg(), rhs.reg(), (Mask << 4) | 0xf), 0));
            }
#endif
        }

        auto r = T{};
        for (std::size_t i = 0; i != N; ++i) {
            if (static_cast<bool>(Mask & (1_uz << i))) {
                r += lhs.v[i] * rhs.v[i];
            }
        }
        return r;
    }

    /** Take the length of the vector
     *
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     * @return Result of the hypot calculation.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr T hypot(numeric_array const &rhs) noexcept
    {
        return std::sqrt(dot<Mask>(rhs, rhs));
    }

    /** Take the squared length of the vector.
     *
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     * @return Result of the hypot-squared calculation.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr T squared_hypot(numeric_array const &rhs) noexcept
    {
        return dot<Mask>(rhs, rhs);
    }

    /** Take a reciprocal of the length.
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     * @return Result of the hypot-squared calculation.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr T rcp_hypot(numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f32x4) {
                return std::bit_cast<float>(_mm_extract_ps(_mm_rsqrt_ps(_mm_dp_ps(rhs.reg(), rhs.reg(), (Mask << 4) | 0xf)), 0));
            }
#endif
        }

        return 1.0f / hypot<Mask>(rhs);
    }

    /** Normalize a vector.
     * All elements that do not participate in the normalization will be set to zero.
     *
     * @tparam Mask A mask for which elements participate in the normalization calculation.
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     * @return The normalized vector.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr numeric_array normalize(numeric_array const &rhs) noexcept
    {
        tt_axiom(rhs.is_vector());

        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f32x4) {
                ttlet rhs_ = rhs.reg();
                ttlet tmp = _mm_mul_ps(_mm_rsqrt_ps(_mm_dp_ps(rhs_, rhs_, (Mask << 4) | 0xf)), rhs_);
                return numeric_array{_mm_insert_ps(tmp, tmp, ~Mask & 0xf)};
            }
#endif
        }

        ttlet rcp_hypot_ = rcp_hypot<Mask>(rhs);

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            if (static_cast<bool>(Mask & (1_uz << i))) {
                r.v[i] = rhs.v[i] * rcp_hypot_;
            }
        }
        return r;
    }

    [[nodiscard]] friend constexpr std::size_t eq(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i64x4 or is_u64x4) {
                return static_cast<std::size_t>(_mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(lhs.reg(), rhs.reg()))));
            } else if constexpr (is_i32x8 or is_u32x8) {
                return static_cast<std::size_t>(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(lhs.reg(), rhs.reg()))));
            } else if constexpr (is_i8x32 or is_u8x32) {
                return static_cast<std::size_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(lhs.reg(), rhs.reg())));
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return static_cast<std::size_t>(_mm256_movemask_pd(_mm256_cmp_pd(lhs.reg(), rhs.reg(), _CMP_EQ_OQ)));
            } else if constexpr (is_f32x8) {
                return static_cast<std::size_t>(_mm256_movemask_ps(_mm256_cmp_ps(lhs.reg(), rhs.reg(), _CMP_EQ_OQ)));
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_i64x2 or is_u64x2) {
                return static_cast<std::size_t>(_mm_movemask_pd(_mm_castsi128_pd(_mm_cmpeq_epi64(lhs.reg(), rhs.reg()))));
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return static_cast<std::size_t>(_mm_movemask_pd(_mm_cmpeq_pd(lhs.reg(), rhs.reg())));
            } else if constexpr (is_i32x4 or is_u32x4) {
                return static_cast<std::size_t>(_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(lhs.reg(), rhs.reg()))));
            } else if constexpr (is_i8x16 or is_u8x16) {
                return static_cast<std::size_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(lhs.reg(), rhs.reg())));
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return static_cast<std::size_t>(_mm_movemask_ps(_mm_cmpeq_ps(lhs.reg(), rhs.reg())));
            }
#endif
        }

        std::size_t r = 0;
        for (std::size_t i = 0; i != N; ++i) {
            r |= static_cast<std::size_t>(lhs.v[i] == rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr std::size_t ne(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return static_cast<std::size_t>(_mm256_movemask_pd(_mm256_cmp_pd(lhs.reg(), rhs.reg(), _CMP_NEQ_OQ)));
            } else if constexpr (is_f32x8) {
                return static_cast<std::size_t>(_mm256_movemask_ps(_mm256_cmp_ps(lhs.reg(), rhs.reg(), _CMP_NEQ_OQ)));
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return static_cast<std::size_t>(_mm_movemask_pd(_mm_cmpneq_pd(lhs.reg(), rhs.reg())));
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return static_cast<std::size_t>(_mm_movemask_ps(_mm_cmpneq_ps(lhs.reg(), rhs.reg())));
            }
#endif
        }

        constexpr std::size_t not_mask = (1 << N) - 1;
        return eq(lhs, rhs) ^ not_mask;
    }

    [[nodiscard]] friend constexpr std::size_t gt(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i64x4) {
                return static_cast<std::size_t>(_mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpgt_epi64(lhs.reg(), rhs.reg()))));
            } else if constexpr (is_i32x8) {
                return static_cast<std::size_t>(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(lhs.reg(), rhs.reg()))));
            } else if constexpr (is_i8x32) {
                return static_cast<std::size_t>(_mm256_movemask_epi8(_mm256_cmpgt_epi8(lhs.reg(), rhs.reg())));
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return static_cast<std::size_t>(_mm256_movemask_pd(_mm256_cmp_pd(lhs.reg(), rhs.reg(), _CMP_GT_OQ)));
            } else if constexpr (is_f32x8) {
                return static_cast<std::size_t>(_mm256_movemask_ps(_mm256_cmp_ps(lhs.reg(), rhs.reg(), _CMP_GT_OQ)));
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_i64x2) {
                return static_cast<std::size_t>(_mm_movemask_pd(_mm_castsi128_pd(_mm_cmpgt_epi64(lhs.reg(), rhs.reg()))));
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return static_cast<std::size_t>(_mm_movemask_pd(_mm_cmpgt_pd(lhs.reg(), rhs.reg())));
            } else if constexpr (is_i32x4) {
                return static_cast<std::size_t>(_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpgt_epi32(lhs.reg(), rhs.reg()))));
            } else if constexpr (is_i8x16) {
                return static_cast<std::size_t>(_mm_movemask_epi8(_mm_cmpgt_epi8(lhs.reg(), rhs.reg())));
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return static_cast<std::size_t>(_mm_movemask_ps(_mm_cmpgt_ps(lhs.reg(), rhs.reg())));
            }
#endif
        }

        unsigned int r = 0;
        for (std::size_t i = 0; i != N; ++i) {
            r |= static_cast<std::size_t>(lhs.v[i] > rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr std::size_t lt(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return static_cast<std::size_t>(_mm256_movemask_pd(_mm256_cmp_pd(lhs.reg(), rhs.reg(), _CMP_LT_OQ)));
            } else if constexpr (is_f32x8) {
                return static_cast<std::size_t>(_mm256_movemask_ps(_mm256_cmp_ps(lhs.reg(), rhs.reg(), _CMP_LT_OQ)));
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return static_cast<std::size_t>(_mm_movemask_pd(_mm_cmplt_pd(lhs.reg(), rhs.reg())));
            } else if constexpr (is_i32x4) {
                return static_cast<std::size_t>(_mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32(lhs.reg(), rhs.reg()))));
            } else if constexpr (is_i8x16) {
                return static_cast<std::size_t>(_mm_movemask_epi8(_mm_cmplt_epi8(lhs.reg(), rhs.reg())));
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return static_cast<std::size_t>(_mm_movemask_ps(_mm_cmplt_ps(lhs.reg(), rhs.reg())));
            }
#endif
        }

        // gt() and eq() has best x64 support.
        return gt(rhs, lhs);
    }

    [[nodiscard]] friend constexpr std::size_t ge(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return static_cast<std::size_t>(_mm256_movemask_pd(_mm256_cmp_pd(lhs.reg(), rhs.reg(), _CMP_GE_OQ)));
            } else if constexpr (is_f32x8) {
                return static_cast<std::size_t>(_mm256_movemask_ps(_mm256_cmp_ps(lhs.reg(), rhs.reg(), _CMP_GE_OQ)));
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return static_cast<std::size_t>(_mm_movemask_pd(_mm_cmpge_pd(lhs.reg(), rhs.reg())));
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return static_cast<std::size_t>(_mm_movemask_ps(_mm_cmpge_ps(lhs.reg(), rhs.reg())));
            }
#endif
        }

        // gt() and eq() has best x64 support.
        return gt(lhs, rhs) | eq(lhs, rhs);
    }

    [[nodiscard]] friend constexpr std::size_t le(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return static_cast<std::size_t>(_mm256_movemask_pd(_mm256_cmp_pd(lhs.reg(), rhs.reg(), _CMP_LE_OQ)));
            } else if constexpr (is_f32x8) {
                return static_cast<std::size_t>(_mm256_movemask_ps(_mm256_cmp_ps(lhs.reg(), rhs.reg(), _CMP_LE_OQ)));
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return static_cast<std::size_t>(_mm_movemask_pd(_mm_cmple_pd(lhs.reg(), rhs.reg())));
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return static_cast<std::size_t>(_mm_movemask_ps(_mm_cmple_ps(lhs.reg(), rhs.reg())));
            }
#endif
        }

        // gt() and eq() has best x64 support.
        return gt(rhs, lhs) | eq(rhs, lhs);
    }

    [[nodiscard]] friend constexpr numeric_array gt_mask(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE4_2)
            if constexpr (is_i64x2) {
                return numeric_array{_mm_cmpgt_epi64(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_i32x4) {
                return numeric_array{_mm_cmpgt_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x8) {
                return numeric_array{_mm_cmpgt_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x16) {
                return numeric_array{_mm_cmpgt_epi8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_cmpgt_ps(lhs.reg(), rhs.reg())};
            }
#endif
        }

        using uint_type = make_uintxx_t<sizeof(T) * CHAR_BIT>;
        constexpr auto ones = std::bit_cast<T>(~uint_type{0});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = lhs.v[i] > rhs.v[i] ? ones : T{0};
        }
        return r;
    }

    [[nodiscard]] friend constexpr bool operator==(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        return not ne(lhs, rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator<<(numeric_array const &lhs, unsigned int rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_castsi256_pd(_mm256_slli_epi64(_mm256_castpd_si256(lhs.reg()), rhs))};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_castsi256_ps(_mm256_slli_epi32(_mm256_castps_si256(lhs.reg()), rhs))};
            } else if constexpr (is_i64x4 or is_u64x4) {
                return numeric_array{_mm256_slli_epi64(lhs.reg(), rhs)};
            } else if constexpr (is_i32x8 or is_u32x8) {
                return numeric_array{_mm256_slli_epi32(lhs.reg(), rhs)};
            } else if constexpr (is_i16x16 or is_u16x16) {
                return numeric_array{_mm256_slli_epi16(lhs.reg(), rhs)};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_castsi128_pd(_mm_slli_epi64(_mm_castpd_si128(lhs.reg()), rhs))};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_castsi128_ps(_mm_slli_epi32(_mm_castps_si128(lhs.reg()), rhs))};
            } else if constexpr (is_i64x2 or is_u64x2) {
                return numeric_array{_mm_slli_epi64(lhs.reg(), rhs)};
            } else if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{_mm_slli_epi32(lhs.reg(), rhs)};
            } else if constexpr (is_i16x8 or is_u16x8) {
                return numeric_array{_mm_slli_epi16(lhs.reg(), rhs)};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] << rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator>>(numeric_array const &lhs, unsigned int rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_castsi256_pd(_mm256_srli_epi64(_mm256_castpd_si256(lhs.reg()), rhs))};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_castsi256_ps(_mm256_srli_epi32(_mm256_castps_si256(lhs.reg()), rhs))};
            } else if constexpr (is_u64x4) {
                return numeric_array{_mm256_srli_epi64(lhs.reg(), rhs)};
            } else if constexpr (is_i32x8) {
                return numeric_array{_mm256_srai_epi32(lhs.reg(), rhs)};
            } else if constexpr (is_u32x8) {
                return numeric_array{_mm256_srli_epi32(lhs.reg(), rhs)};
            } else if constexpr (is_i16x16) {
                return numeric_array{_mm256_srai_epi16(lhs.reg(), rhs)};
            } else if constexpr (is_u16x16) {
                return numeric_array{_mm256_srli_epi16(lhs.reg(), rhs)};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_castsi128_pd(_mm_srli_epi64(_mm_castpd_si128(lhs.reg()), rhs))};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(lhs.reg()), rhs))};
            } else if constexpr (is_u64x2) {
                return numeric_array{_mm_srli_epi64(lhs.reg(), rhs)};
            } else if constexpr (is_i32x4) {
                return numeric_array{_mm_srai_epi32(lhs.reg(), rhs)};
            } else if constexpr (is_u32x4) {
                return numeric_array{_mm_srli_epi32(lhs.reg(), rhs)};
            } else if constexpr (is_i16x8) {
                return numeric_array{_mm_srai_epi16(lhs.reg(), rhs)};
            } else if constexpr (is_u16x8) {
                return numeric_array{_mm_srli_epi16(lhs.reg(), rhs)};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] >> rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator|(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i64x4 or is_u64x4 or is_i32x8 or is_u32x8 or is_i16x8 or is_u16x8 or is_i8x32 or is_u8x32) {
                return numeric_array{_mm256_or_si256(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_or_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_or_ps(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i64x4 or is_u64x4 or is_i32x8 or is_u32x8 or is_i16x8 or is_u16x8 or is_i8x32 or is_u8x32) {
                return numeric_array{
                    _mm256_castps_si256(_mm256_or_ps(_mm256_castsi256_ps(lhs.reg()), _mm256_castsi256_ps(rhs.reg())))};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_or_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i64x2 or is_u64x2 or is_i32x4 or is_u32x4 or is_i16x8 or is_u16x8 or is_i8x16 or is_i8x16) {
                return numeric_array{_mm_or_si128(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_castps_pd(_mm_or_ps(_mm_castps_ps(lhs.reg()), _mm_castps_ps(rhs.reg())))};

            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_or_ps(lhs.reg(), rhs.reg())};

            } else if constexpr (is_i64x2 or is_u64x2 or is_i32x4 or is_u32x4 or is_i16x8 or is_u16x8 or is_i8x16 or is_i8x16) {
                return numeric_array{_mm_castps_si128(_mm_or_ps(_mm_castsi128_ps(lhs.reg()), _mm_castsi128_ps(rhs.reg())))};
            }
#endif
        }

        using uint_type = make_uintxx_t<sizeof(T) * CHAR_BIT>;

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] =
                std::bit_cast<T>(static_cast<uint_type>(std::bit_cast<uint_type>(lhs.v[i]) | std::bit_cast<uint_type>(rhs.v[i])));
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator|(numeric_array const &lhs, T const &rhs) noexcept
    {
        return lhs | broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator|(T const &lhs, numeric_array const &rhs) noexcept
    {
        return broadcast(lhs) | rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator&(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i64x4 or is_u64x4 or is_i32x8 or is_u32x8 or is_i16x8 or is_u16x8 or is_i8x32 or is_u8x32) {
                return numeric_array{_mm256_and_si256(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_and_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_and_ps(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i64x4 or is_u64x4 or is_i32x8 or is_u32x8 or is_i16x8 or is_u16x8 or is_i8x32 or is_u8x32) {
                return numeric_array{
                    _mm256_castps_si256(_mm256_and_ps(_mm256_castsi256_ps(lhs.reg()), _mm256_castsi256_ps(rhs.reg())))};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_and_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i64x2 or is_u64x2 or is_i32x4 or is_u32x4 or is_i16x8 or is_u16x8 or is_i8x16 or is_i8x16) {
                return numeric_array{_mm_and_si128(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_castps_pd(_mm_and_ps(_mm_castps_ps(lhs.reg()), _mm_castps_ps(rhs.reg())))};

            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_and_ps(lhs.reg(), rhs.reg())};

            } else if constexpr (is_i64x2 or is_u64x2 or is_i32x4 or is_u32x4 or is_i16x8 or is_u16x8 or is_i8x16 or is_i8x16) {
                return numeric_array{_mm_castps_si128(_mm_and_ps(_mm_castsi128_ps(lhs.reg()), _mm_castsi128_ps(rhs.reg())))};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] & rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator&(numeric_array const &lhs, T const &rhs) noexcept
    {
        return lhs & broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator&(T const &lhs, numeric_array const &rhs) noexcept
    {
        return broadcast(lhs) & rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator^(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i64x4 or is_u64x4 or is_i32x8 or is_u32x8 or is_i16x8 or is_u16x8 or is_i8x32 or is_u8x32) {
                return numeric_array{_mm256_xor_si256(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_xor_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_xor_ps(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i64x4 or is_u64x4 or is_i32x8 or is_u32x8 or is_i16x8 or is_u16x8 or is_i8x32 or is_u8x32) {
                return numeric_array{
                    _mm256_castps_si256(_mm256_xor_ps(_mm256_castsi256_ps(lhs.reg()), _mm256_castsi256_ps(rhs.reg())))};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_xor_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i64x2 or is_u64x2 or is_i32x4 or is_u32x4 or is_i16x8 or is_u16x8 or is_i8x16 or is_i8x16) {
                return numeric_array{_mm_xor_si128(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_castps_pd(_mm_xor_ps(_mm_castps_ps(lhs.reg()), _mm_castps_ps(rhs.reg())))};

            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_xor_ps(lhs.reg(), rhs.reg())};

            } else if constexpr (is_i64x2 or is_u64x2 or is_i32x4 or is_u32x4 or is_i16x8 or is_u16x8 or is_i8x16 or is_i8x16) {
                return numeric_array{_mm_castps_si128(_mm_xor_ps(_mm_castsi128_ps(lhs.reg()), _mm_castsi128_ps(rhs.reg())))};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] ^ rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator^(numeric_array const &lhs, T const &rhs) noexcept
    {
        return lhs ^ broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator^(T const &lhs, numeric_array const &rhs) noexcept
    {
        return broadcast(lhs) ^ rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator+(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i64x4 or is_u64x4) {
                return numeric_array{_mm256_add_epi64(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i32x8 or is_u32x8) {
                return numeric_array{_mm256_add_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x16 or is_u16x16) {
                return numeric_array{_mm256_add_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x32 or is_u8x32) {
                return numeric_array{_mm256_add_epi8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_add_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_add_ps(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_add_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i64x2 or is_u64x2) {
                return numeric_array{_mm_add_epi64(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{_mm_add_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x8 or is_u16x8) {
                return numeric_array{_mm_add_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x16 or is_u8x16) {
                return numeric_array{_mm_add_epi8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_add_ps(lhs.reg(), rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] + rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator+(numeric_array const &lhs, T const &rhs) noexcept
    {
        return lhs + broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator+(T const &lhs, numeric_array const &rhs) noexcept
    {
        return broadcast(lhs) + rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i64x4 or is_u64x4) {
                return numeric_array{_mm256_sub_epi64(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i32x8 or is_u32x8) {
                return numeric_array{_mm256_sub_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x16 or is_u16x16) {
                return numeric_array{_mm256_sub_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x32 or is_u8x32) {
                return numeric_array{_mm256_sub_epi8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_sub_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_sub_ps(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_sub_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i64x2 or is_u64x2) {
                return numeric_array{_mm_sub_epi64(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{_mm_sub_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x8 or is_u16x8) {
                return numeric_array{_mm_sub_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x16 or is_u8x16) {
                return numeric_array{_mm_sub_epi8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_sub_ps(lhs.reg(), rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] - rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &lhs, T const &rhs) noexcept
    {
        return lhs - broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator-(T const &lhs, numeric_array const &rhs) noexcept
    {
        return broadcast(lhs) - rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator*(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i32x8) {
                return numeric_array{_mm256_mul_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u32x8) {
                return numeric_array{_mm256_mul_epu32(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_mul_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_mul_ps(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_i32x4) {
                return numeric_array{_mm_mul_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f16x4) {
                return numeric_array{numeric_array<float, 4>{lhs} * numeric_array<float, 4>{rhs}};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_mul_pd(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_mul_ps(lhs.reg(), rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] * rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator*(numeric_array const &lhs, T const &rhs) noexcept
    {
        return lhs * broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator*(T const &lhs, numeric_array const &rhs) noexcept
    {
        return broadcast(lhs) * rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator/(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_div_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_div_ps(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_div_pd(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_div_ps(lhs.reg(), rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] / rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator/(numeric_array const &lhs, T const &rhs) noexcept
    {
        return lhs / broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator/(T const &lhs, numeric_array const &rhs) noexcept
    {
        return broadcast(lhs) / rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator%(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        ttlet div_result = floor(lhs / rhs);
        return lhs - (div_result * rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator%(numeric_array const &lhs, T const &rhs) noexcept
    {
        return lhs % broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator%(T const &lhs, numeric_array const &rhs) noexcept
    {
        return broadcast(lhs) % rhs;
    }

    [[nodiscard]] friend constexpr numeric_array min(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i32x8) {
                return numeric_array{_mm256_min_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u32x8) {
                return numeric_array{_mm256_min_epu32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x16) {
                return numeric_array{_mm256_min_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u16x16) {
                return numeric_array{_mm256_min_epu16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x32) {
                return numeric_array{_mm256_min_epi8(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u8x32) {
                return numeric_array{_mm256_min_epu8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_min_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_min_ps(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_i32x4) {
                return numeric_array{_mm_min_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u32x4) {
                return numeric_array{_mm_min_epu32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u16x8) {
                return numeric_array{_mm_min_epu16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x16) {
                return numeric_array{_mm_min_epi8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_min_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x8) {
                return numeric_array{_mm_min_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u8x16) {
                return numeric_array{_mm_min_epu8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_min_ps(lhs.reg(), rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = std::min(lhs.v[i], rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array max(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i32x8) {
                return numeric_array{_mm256_max_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u32x8) {
                return numeric_array{_mm256_max_epu32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x16) {
                return numeric_array{_mm256_max_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u16x16) {
                return numeric_array{_mm256_max_epu16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x32) {
                return numeric_array{_mm256_max_epi8(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u8x32) {
                return numeric_array{_mm256_max_epu8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_max_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_max_ps(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_i32x4) {
                return numeric_array{_mm_max_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u32x4) {
                return numeric_array{_mm_max_epu32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u16x8) {
                return numeric_array{_mm_max_epu16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x16) {
                return numeric_array{_mm_max_epi8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_max_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x8) {
                return numeric_array{_mm_max_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_u8x16) {
                return numeric_array{_mm_max_epu8(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4) {
                return numeric_array{_mm_max_ps(lhs.reg(), rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = std::max(lhs.v[i], rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array
    clamp(numeric_array const &lhs, numeric_array const &low, numeric_array const &high) noexcept
    {
        return min(max(lhs, low), high);
    }

    [[nodiscard]] friend constexpr numeric_array hadd(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i32x8 or is_u32x8) {
                return numeric_array{_mm256_hadd_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x16 or is_u16x16) {
                return numeric_array{_mm256_hadd_epi16(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_hadd_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_hadd_ps(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSSE3)
            if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{_mm_hadd_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x8 or is_u16x8) {
                return numeric_array{_mm_hadd_epi16(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE3)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_hadd_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_hadd_ps(lhs.reg(), rhs.reg())};
            }
#endif
        }

        tt_axiom(N % 2 == 0);

        auto r = numeric_array{};

        std::size_t src_i = 0;
        std::size_t dst_i = 0;
        while (src_i != N) {
            auto tmp = lhs[src_i++];
            tmp += lhs[src_i++];
            r.v[dst_i++] = tmp;
        }

        src_i = 0;
        while (src_i != N) {
            auto tmp = rhs[src_i++];
            tmp += rhs[src_i++];
            r.v[dst_i++] = tmp;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array hsub(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX2)
            if constexpr (is_i32x8 or is_u32x8) {
                return numeric_array{_mm256_hsub_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x16 or is_u16x16) {
                return numeric_array{_mm256_hsub_epi16(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x4) {
                return numeric_array{_mm256_hsub_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x8) {
                return numeric_array{_mm256_hsub_ps(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSSE3)
            if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{_mm_hsub_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x8 or is_u16x8) {
                return numeric_array{_mm_hsub_epi16(lhs.reg(), rhs.reg())};
            }
#endif
#if defined(TT_HAS_SSE3)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_hsub_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_hsub_ps(lhs.reg(), rhs.reg())};
            }
#endif
        }

        tt_axiom(N % 2 == 0);

        auto r = numeric_array{};

        std::size_t src_i = 0;
        std::size_t dst_i = 0;
        while (src_i != N) {
            auto tmp = lhs[src_i++];
            tmp -= lhs[src_i++];
            r.v[dst_i++] = tmp;
        }

        src_i = 0;
        while (src_i != N) {
            auto tmp = rhs[src_i++];
            tmp -= rhs[src_i++];
            r.v[dst_i++] = tmp;
        }
        return r;
    }

    /** Add or subtract individual elements.
     *
     * @tparam Mask bit mask where '1' means to add, '0' means to subtract.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr numeric_array addsub(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        constexpr std::size_t not_mask = (1 << N) - 1;
        return lhs + neg<Mask ^ not_mask>(rhs);
    }

    /** Calculate the 2D normal on a 2D vector.
     */
    [[nodiscard]] friend constexpr numeric_array cross_2D(numeric_array const &rhs) noexcept requires(N >= 2)
    {
        tt_axiom(rhs.z() == 0.0f && rhs.is_vector());
        return numeric_array{-rhs.y(), rhs.x()};
    }

    /** Calculate the 2D unit-normal on a 2D vector.
     */
    [[nodiscard]] friend constexpr numeric_array normal_2D(numeric_array const &rhs) noexcept requires(N >= 2)
    {
        return normalize<0b0011>(cross_2D(rhs));
    }

    /** Calculate the cross-product between two 2D vectors.
     * a.x * b.y - a.y * b.x
     */
    [[nodiscard]] friend constexpr float cross_2D(numeric_array const &lhs, numeric_array const &rhs) noexcept requires(N >= 2)
    {
        ttlet tmp1 = rhs.yxwz();
        ttlet tmp2 = lhs * tmp1;
        ttlet tmp3 = hsub(tmp2, tmp2);
        return get<0>(tmp3);
    }

    // x=a.y*b.z - a.z*b.y
    // y=a.z*b.x - a.x*b.z
    // z=a.x*b.y - a.y*b.x
    // w=a.w*b.w - a.w*b.w
    [[nodiscard]] constexpr friend numeric_array cross_3D(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N == 4)
    {
        ttlet a_left = lhs.yzxw();
        ttlet b_left = rhs.zxyw();
        ttlet left = a_left * b_left;

        ttlet a_right = lhs.zxyw();
        ttlet b_right = rhs.yzxw();
        ttlet right = a_right * b_right;
        return left - right;
    }

    [[nodiscard]] static constexpr numeric_array byte_srl_shuffle_indices(unsigned int rhs) requires(is_i8x16)
    {
        static_assert(std::endian::native == std::endian::little);

        auto r = numeric_array{};
        for (auto i = 0; i != 16; ++i) {
            if ((i + rhs) < 16) {
                r[i] = narrow_cast<int8_t>(i + rhs);
            } else {
                // Indices set to -1 result in a zero after a byte shuffle.
                r[i] = -1;
            }
        }
        return r;
    }

    [[nodiscard]] static constexpr numeric_array byte_sll_shuffle_indices(unsigned int rhs) requires(is_i8x16)
    {
        static_assert(std::endian::native == std::endian::little);

        auto r = numeric_array{};
        for (auto i = 0; i != 16; ++i) {
            if ((i - rhs) >= 0) {
                r[i] = narrow_cast<int8_t>(i - rhs);
            } else {
                // Indices set to -1 result in a zero after a byte shuffle.
                r[i] = -1;
            }
        }
        return r;
    }

    /** Shuffle a 16x byte array, using the indices from the right-hand-side.
     */
    [[nodiscard]] friend constexpr numeric_array shuffle(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(std::is_integral_v<value_type>)
    {
        if (!std::is_constant_evaluated()) {
#if defined(TT_HAS_SSSE3)
            if constexpr (is_i8x16 or is_u8x16) {
                return numeric_array{_mm_shuffle_epi8(lhs.reg(), rhs.reg())};
            }
#endif
        }

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            if (rhs[i] >= 0) {
                r[i] = lhs[rhs[i] & 0xf];
            } else {
                r[i] = 0;
            }
        }

        return r;
    }

    /** Find a point at the midpoint between two points.
     */
    [[nodiscard]] friend constexpr numeric_array midpoint(numeric_array const &p1, numeric_array const &p2) noexcept
    {
        tt_axiom(p1.is_point());
        tt_axiom(p2.is_point());
        return (p1 + p2) * 0.5f;
    }

    /** Find the point on the other side and at the same distance of an anchor-point.
     */
    [[nodiscard]] friend constexpr numeric_array reflect_point(numeric_array const &p, numeric_array const anchor) noexcept
    {
        tt_axiom(p.is_point());
        tt_axiom(anchor.is_point());
        return anchor - (p - anchor);
    }

    template<typename... Columns>
    [[nodiscard]] friend constexpr std::array<numeric_array, N> transpose(Columns const &...columns) noexcept
    {
        static_assert(sizeof...(Columns) == N, "Can only transpose square matrices");

        if (not std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE)
            if constexpr (is_f32x4 and sizeof...(Columns) == 4) {
                auto tmp = std::array<__m128, N>{columns.reg()...};
                _MM_TRANSPOSE4_PS(std::get<0>(tmp), std::get<1>(tmp), std::get<2>(tmp), std::get<3>(tmp));
                return {
                    numeric_array{get<0>(tmp)},
                    numeric_array{get<1>(tmp)},
                    numeric_array{get<2>(tmp)},
                    numeric_array{get<3>(tmp)}};
#endif
            }
        }

        auto r = std::array<numeric_array, N>{};
        transpose_detail<0, Columns...>(columns..., r);
        return r;
    }

    [[nodiscard]] constexpr friend numeric_array composit(numeric_array const &under, numeric_array const &over) noexcept
        requires(N == 4 && std::is_floating_point_v<T>)
    {
        if (over.is_transparent()) {
            return under;
        }
        if (over.is_opaque()) {
            return over;
        }

        ttlet over_alpha = over.wwww();
        ttlet under_alpha = under.wwww();

        ttlet over_color = over.xyz1();
        ttlet under_color = under.xyz1();

        ttlet output_color = over_color * over_alpha + under_color * under_alpha * (T{1} - over_alpha);

        return output_color / output_color.www1();
    }

    [[nodiscard]] constexpr friend numeric_array composit(numeric_array const &under, numeric_array const &over) noexcept
        requires(is_f16x4)
    {
        return numeric_array{composit(static_cast<numeric_array<float, 4>>(under), static_cast<numeric_array<float, 4>>(over))};
    }

    [[nodiscard]] friend std::string to_string(numeric_array const &rhs) noexcept
    {
        auto r = std::string{};

        r += '(';
        for (std::size_t i = 0; i != N; ++i) {
            if (i != 0) {
                r += "; ";
            }
            r += std::format("{}", rhs[i]);
        }
        r += ')';
        return r;
    }

    friend std::ostream &operator<<(std::ostream &lhs, numeric_array const &rhs)
    {
        return lhs << to_string(rhs);
    }

    /** Insert an element from rhs into the result.
     * This function copies the lhs, then inserts one element from rhs into the result.
     * It also can clear any of the elements to zero.
     */
    template<std::size_t FromElement, std::size_t ToElement>
    [[nodiscard]] constexpr friend numeric_array insert(numeric_array const &lhs, numeric_array const &rhs)
    {
        auto r = numeric_array{};

        if (!std::is_constant_evaluated()) {
#if defined(TT_HAS_SSE4_1)
            if constexpr (is_f32x4) {
                constexpr uint8_t insert_mask = static_cast<uint8_t>((FromElement << 6) | (ToElement << 4));
                return numeric_array{_mm_insert_ps(lhs.reg(), rhs.reg(), insert_mask)};

            } else if constexpr (is_i32x4 or is_u32x4) {
                constexpr uint8_t insert_mask = static_cast<uint8_t>((FromElement << 6) | (ToElement << 4));
                return numeric_array{
                    _mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(lhs.reg()), _mm_castsi128_ps(rhs.reg()), insert_mask))};
            }
#endif
#if defined(TT_HAS_SSE2)
            if constexpr (is_f64x2) {
                if constexpr (FromElement == 0 and ToElement == 0) {
                    return numeric_array{_mm_shuffle_pd(rhs.reg(), lhs.reg(), 0b10)};
                } else if constexpr (FromElement == 1 and ToElement == 0) {
                    return numeric_array{_mm_shuffle_pd(rhs.reg(), lhs.reg(), 0b11)};
                } else if constexpr (FromElement == 0 and ToElement == 1) {
                    return numeric_array{_mm_shuffle_pd(lhs.reg(), rhs.reg(), 0b00)};
                } else {
                    return numeric_array{_mm_shuffle_pd(lhs.reg(), rhs.reg(), 0b10)};
                }

            } else if constexpr (is_i64x2 or is_u64x2) {
                ttlet lhs_ = _mm_castsi128_pd(lhs.reg());
                ttlet rhs_ = _mm_castsi128_pd(rhs.reg());

                if constexpr (FromElement == 0 and ToElement == 0) {
                    return numeric_array{_mm_castpd_si128(_mm_shuffle_pd(rhs_, lhs_, 0b10))};
                } else if constexpr (FromElement == 1 and ToElement == 0) {
                    return numeric_array{_mm_castpd_si128(_mm_shuffle_pd(rhs_, lhs_, 0b11))};
                } else if constexpr (FromElement == 0 and ToElement == 1) {
                    return numeric_array{_mm_castpd_si128(_mm_shuffle_pd(lhs_, rhs_, 0b00))};
                } else {
                    return numeric_array{_mm_castpd_si128(_mm_shuffle_pd(lhs_, rhs_, 0b10))};
                }
            }
#endif
        }

        for (std::size_t i = 0; i != N; ++i) {
            r[i] = (i == ToElement) ? rhs[FromElement] : lhs[i];
        }

        return r;
    }

    /** swizzle around the elements of the numeric array.
     *
     * @tparam Elements a list of indices pointing to an element in this array.
     *         Or the special indices -1: literal zero, -2: literal one.
     * @return A new array with the elements ordered based on the Elements list.
     *         The elements at the end of the array are set to zero.
     */
    template<ssize_t... Elements>
    [[nodiscard]] constexpr numeric_array swizzle() const
    {
        static_assert(sizeof...(Elements) <= N);

        if (!std::is_constant_evaluated()) {
#if defined(TT_HAS_AVX)
            if constexpr (is_f64x2) {
                return numeric_array{_mm_swizzle_pd<Elements...>(reg())};
            } else if constexpr (is_f32x4) {
                return numeric_array{_mm_swizzle_ps<Elements...>(reg())};
            } else if constexpr (is_i64x2 or is_u64x2) {
                return numeric_array{_mm_swizzle_epi64<Elements...>(reg())};
            } else if constexpr (is_i32x4 or is_u32x4) {
                return numeric_array{_mm_swizzle_epi32<Elements...>(reg())};
            }
#endif
        }

        auto r = numeric_array{};
        swizzle_detail<0, Elements...>(r);
        return r;
    }

#define SWIZZLE(swizzle_name, D, ...) \
    [[nodiscard]] constexpr numeric_array swizzle_name() const noexcept requires(D == N) \
    { \
        return swizzle<__VA_ARGS__>(); \
    }

#define SWIZZLE_4D_GEN1(name, ...) \
    SWIZZLE(name##0, 4, __VA_ARGS__, get_zero) \
    SWIZZLE(name##1, 4, __VA_ARGS__, get_one) \
    SWIZZLE(name##x, 4, __VA_ARGS__, 0) \
    SWIZZLE(name##y, 4, __VA_ARGS__, 1) \
    SWIZZLE(name##z, 4, __VA_ARGS__, 2) \
    SWIZZLE(name##w, 4, __VA_ARGS__, 3)

#define SWIZZLE_4D_GEN2(name, ...) \
    SWIZZLE_4D_GEN1(name##0, __VA_ARGS__, get_zero) \
    SWIZZLE_4D_GEN1(name##1, __VA_ARGS__, get_one) \
    SWIZZLE_4D_GEN1(name##x, __VA_ARGS__, 0) \
    SWIZZLE_4D_GEN1(name##y, __VA_ARGS__, 1) \
    SWIZZLE_4D_GEN1(name##z, __VA_ARGS__, 2) \
    SWIZZLE_4D_GEN1(name##w, __VA_ARGS__, 3)

#define SWIZZLE_4D_GEN3(name, ...) \
    SWIZZLE_4D_GEN2(name##0, __VA_ARGS__, get_zero) \
    SWIZZLE_4D_GEN2(name##1, __VA_ARGS__, get_one) \
    SWIZZLE_4D_GEN2(name##x, __VA_ARGS__, 0) \
    SWIZZLE_4D_GEN2(name##y, __VA_ARGS__, 1) \
    SWIZZLE_4D_GEN2(name##z, __VA_ARGS__, 2) \
    SWIZZLE_4D_GEN2(name##w, __VA_ARGS__, 3)

    SWIZZLE_4D_GEN3(_0, get_zero)
    SWIZZLE_4D_GEN3(_1, get_one)
    SWIZZLE_4D_GEN3(x, 0)
    SWIZZLE_4D_GEN3(y, 1)
    SWIZZLE_4D_GEN3(z, 2)
    SWIZZLE_4D_GEN3(w, 3)

#define SWIZZLE_3D_GEN1(name, ...) \
    SWIZZLE(name##0, 3, __VA_ARGS__, get_zero) \
    SWIZZLE(name##1, 3, __VA_ARGS__, get_one) \
    SWIZZLE(name##x, 3, __VA_ARGS__, 0) \
    SWIZZLE(name##y, 3, __VA_ARGS__, 1) \
    SWIZZLE(name##z, 3, __VA_ARGS__, 2)

#define SWIZZLE_3D_GEN2(name, ...) \
    SWIZZLE_3D_GEN1(name##0, __VA_ARGS__, get_zero) \
    SWIZZLE_3D_GEN1(name##1, __VA_ARGS__, get_one) \
    SWIZZLE_3D_GEN1(name##x, __VA_ARGS__, 0) \
    SWIZZLE_3D_GEN1(name##y, __VA_ARGS__, 1) \
    SWIZZLE_3D_GEN1(name##z, __VA_ARGS__, 2)

    SWIZZLE_3D_GEN2(_0, get_zero)
    SWIZZLE_3D_GEN2(_1, get_one)
    SWIZZLE_3D_GEN2(x, 0)
    SWIZZLE_3D_GEN2(y, 1)
    SWIZZLE_3D_GEN2(z, 2)

#define SWIZZLE_2D_GEN1(name, ...) \
    SWIZZLE(name##0, 2, __VA_ARGS__, get_zero) \
    SWIZZLE(name##1, 2, __VA_ARGS__, get_one) \
    SWIZZLE(name##x, 2, __VA_ARGS__, 0) \
    SWIZZLE(name##y, 2, __VA_ARGS__, 1)

    SWIZZLE_2D_GEN1(_0, get_zero)
    SWIZZLE_2D_GEN1(_1, get_one)
    SWIZZLE_2D_GEN1(x, 0)
    SWIZZLE_2D_GEN1(y, 1)

#undef SWIZZLE
#undef SWIZZLE_4D_GEN1
#undef SWIZZLE_4D_GEN2
#undef SWIZZLE_4D_GEN3
#undef SWIZZLE_3D_GEN1
#undef SWIZZLE_3D_GEN2
#undef SWIZZLE_2D_GEN1

    template<int I, typename First, typename... Rest>
    friend constexpr void transpose_detail(First const &first, Rest const &...rest, std::array<numeric_array, N> &r) noexcept
    {
        for (std::size_t j = 0; j != N; ++j) {
            r[j][I] = first[j];
        }

        if constexpr (sizeof...(Rest) != 0) {
            transpose_detail<I + 1, Rest...>(rest..., r);
        }
    }

    template<ssize_t I, ssize_t FirstElement, ssize_t... RestElements>
    constexpr void swizzle_detail(numeric_array &r) const noexcept
    {
        static_assert(I < narrow_cast<ssize_t>(N));
        static_assert(FirstElement >= -2 && FirstElement < narrow_cast<ssize_t>(N), "Index out of bounds");

        get<I>(r) = get<FirstElement>(*this);
        if constexpr (sizeof...(RestElements) != 0) {
            swizzle_detail<I + 1, RestElements...>(r);
        }
    }
};

using i8x1 = numeric_array<int8_t, 1>;
using i8x2 = numeric_array<int8_t, 2>;
using i8x4 = numeric_array<int8_t, 4>;
using i8x8 = numeric_array<int8_t, 8>;
using i8x16 = numeric_array<int8_t, 16>;
using i8x32 = numeric_array<int8_t, 32>;
using i8x64 = numeric_array<int8_t, 64>;

using u8x1 = numeric_array<uint8_t, 1>;
using u8x2 = numeric_array<uint8_t, 2>;
using u8x4 = numeric_array<uint8_t, 4>;
using u8x8 = numeric_array<uint8_t, 8>;
using u8x16 = numeric_array<uint8_t, 16>;
using u8x32 = numeric_array<uint8_t, 32>;
using u8x64 = numeric_array<uint8_t, 64>;

using i16x1 = numeric_array<int16_t, 1>;
using i16x2 = numeric_array<int16_t, 2>;
using i16x4 = numeric_array<int16_t, 4>;
using i16x8 = numeric_array<int16_t, 8>;
using i16x16 = numeric_array<int16_t, 16>;
using i16x32 = numeric_array<int16_t, 32>;

using u16x1 = numeric_array<uint16_t, 1>;
using u16x2 = numeric_array<uint16_t, 2>;
using u16x4 = numeric_array<uint16_t, 4>;
using u16x8 = numeric_array<uint16_t, 8>;
using u16x16 = numeric_array<uint16_t, 16>;
using u16x32 = numeric_array<uint16_t, 32>;

using f16x4 = numeric_array<float16, 4>;

using i32x1 = numeric_array<int32_t, 1>;
using i32x2 = numeric_array<int32_t, 2>;
using i32x4 = numeric_array<int32_t, 4>;
using i32x8 = numeric_array<int32_t, 8>;
using i32x16 = numeric_array<int32_t, 16>;

using u32x1 = numeric_array<uint32_t, 1>;
using u32x2 = numeric_array<uint32_t, 2>;
using u32x4 = numeric_array<uint32_t, 4>;
using u32x8 = numeric_array<uint32_t, 8>;
using u32x16 = numeric_array<uint32_t, 16>;

using f32x1 = numeric_array<float, 1>;
using f32x2 = numeric_array<float, 2>;
using f32x4 = numeric_array<float, 4>;
using f32x8 = numeric_array<float, 8>;
using f32x16 = numeric_array<float, 16>;

using i64x1 = numeric_array<int64_t, 1>;
using i64x2 = numeric_array<int64_t, 2>;
using i64x4 = numeric_array<int64_t, 4>;
using i64x8 = numeric_array<int64_t, 8>;

using u64x1 = numeric_array<uint64_t, 1>;
using u64x2 = numeric_array<uint64_t, 2>;
using u64x4 = numeric_array<uint64_t, 4>;
using u64x8 = numeric_array<uint64_t, 8>;

using f64x1 = numeric_array<double, 1>;
using f64x2 = numeric_array<double, 2>;
using f64x4 = numeric_array<double, 4>;
using f64x8 = numeric_array<double, 8>;

} // namespace tt::inline v1

template<class T, std::size_t N>
struct std::tuple_size<tt::numeric_array<T, N>> : std::integral_constant<std::size_t, N> {
};

template<std::size_t I, class T, std::size_t N>
struct std::tuple_element<I, tt::numeric_array<T, N>> {
    using type = T;
};

tt_warning_pop();
