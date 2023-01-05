// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_f32x4_sse.hpp"
#include "simd_f64x4_avx.hpp"
#include "simd_i32x4_sse2.hpp"
#include "simd_i64x4_avx2.hpp"
#include "simd_u32x4_sse2.hpp"
#include "simd_conversions_x86.hpp"

#include "../architecture.hpp"
#include "../concepts.hpp"
#include "../cast.hpp"
#include "../type_traits.hpp"
#include "../float16.hpp"
#include "../math.hpp"
#include "../utility.hpp"
#include "../fixed_string.hpp"

#if defined(HI_HAS_AVX)
#include "swizzle_avx.hpp"
#endif
#if defined(HI_HAS_SSE4_1)
#include "float16_sse4_1.hpp"
#endif

#include <cstdint>
#include <ostream>
#include <string>
#include <array>
#include <type_traits>
#include <concepts>
#include <bit>
#include <climits>
#include <utility>

hi_warning_push();
// C4702 unreachable code: Suppressed due intrinsics and std::is_constant_evaluated()
hi_warning_ignore_msvc(4702);
// C26490: Don't use reinterpret_cast (type.1).
// Needed for casting pointers to or from SSE registers.
hi_warning_ignore_msvc(26490);

namespace hi::inline v1 {

#define HI_X_runtime_evaluate_if_valid(x) \
    do { \
        if (not std::is_constant_evaluated()) { \
            if constexpr (requires { x; }) { \
                return x; \
            } \
        } \
    } while (false)

template<numeric_limited T, std::size_t N>
struct numeric_array {
    using value_type = T;
    constexpr static size_t size = N;

    using array_type = std::array<value_type, size>;
    using simd_type = low_level_simd_t<value_type, size>;
    constexpr static bool has_simd_type = has_low_level_simd_v<value_type, size>;

    using size_type = typename array_type::size_type;
    using difference_type = typename array_type::difference_type;
    using reference = typename array_type::reference;
    using const_reference = typename array_type::const_reference;
    using pointer = typename array_type::pointer;
    using const_pointer = typename array_type::const_pointer;
    using iterator = typename array_type::iterator;
    using const_iterator = typename array_type::const_iterator;

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

    array_type v;

    constexpr numeric_array() noexcept : v()
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { *this = numeric_array{low_level_simd_t<value_type, N>{}}; }) {
                *this = numeric_array{low_level_simd_t<value_type, N>{}};
            }
        }
    }

    constexpr numeric_array(numeric_array const& rhs) noexcept = default;
    constexpr numeric_array(numeric_array&& rhs) noexcept = default;
    constexpr numeric_array& operator=(numeric_array const& rhs) noexcept = default;
    constexpr numeric_array& operator=(numeric_array&& rhs) noexcept = default;

    template<numeric_limited U>
    [[nodiscard]] constexpr explicit numeric_array(numeric_array<U, N> const& other) noexcept : v()
    {
        if (not std::is_constant_evaluated()) {
            if constexpr (requires { *this = numeric_array{low_level_simd_t<value_type, N>{other.simd()}}; }) {
                *this = numeric_array{low_level_simd_t<value_type, N>{other.simd()}};
                return;
            }
        }

        for (std::size_t i = 0; i != N; ++i) {
            if constexpr (std::is_integral_v<T> and std::is_floating_point_v<U>) {
                // SSE conversion round floats before converting to integer.
                v[i] = static_cast<value_type>(std::round(other[i]));
            } else {
                v[i] = static_cast<value_type>(other[i]);
            }
        }
    }

    template<numeric_limited U, std::size_t M>
    [[nodiscard]] constexpr explicit numeric_array(numeric_array<U, M> const& other1, numeric_array<U, M> const& other2) noexcept
        :
        v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_AVX)
            if constexpr (is_f64x4 and other1.is_f64x2 and other2.is_f64x2) {
                v = numeric_array{_mm256_set_m128d(other2.reg(), other1.reg())};
            } else if constexpr (is_f32x8 and other1.is_f32x4 and other2.is_f32x4) {
                v = numeric_array{_mm256_set_m128(other2.reg(), other1.reg())};
            } else if constexpr (
                std::is_integral_v<T> and std::is_integral_v<U> and (sizeof(T) * N == 32) and (sizeof(U) * M == 16)) {
                v = numeric_array{_mm256_set_m128i(other2.reg(), other1.reg())};
            }
#endif
#if defined(HI_HAS_SSE4_1)
            if constexpr (is_u16x8 and other1.is_u32x4 and other2.is_u32x4) {
                v = numeric_array{_mm_packus_epu32(other2.reg(), other1.reg())};
            }
#endif
#if defined(HI_HAS_SSE2)
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

    [[nodiscard]] constexpr explicit numeric_array(T const& x) noexcept : v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE)
            if constexpr (is_f32x4) {
                *this = numeric_array{_mm_set_ss(x)};
                return;
            }
#endif
        }
        get<0>(v) = x;
    }

    [[nodiscard]] constexpr explicit numeric_array(T const& x, T const& y) noexcept
        requires(N >= 2)
        : v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE2)
            if constexpr (is_i32x4) {
                *this = numeric_array{_mm_set_epi32(0, 0, y, x)};
                return;
            }
#endif
        }
        get<0>(v) = x;
        get<1>(v) = y;
    }

    [[nodiscard]] constexpr explicit numeric_array(T const& x, T const& y, T const& z) noexcept
        requires(N >= 3)
        : v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE2)
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

    [[nodiscard]] constexpr explicit numeric_array(T const& x, T const& y, T const& z, T const& w) noexcept
        requires(N >= 4)
        : v()
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE2)
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
        HI_X_runtime_evaluate_if_valid(numeric_array{simd_type::broadcast(rhs)});

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

    [[nodiscard]] numeric_array(std::array<T, N> const& rhs) noexcept : v(rhs) {}

    numeric_array& operator=(std::array<T, N> const& rhs) noexcept
    {
        v = rhs;
        return *this;
    }

    [[nodiscard]] operator std::array<T, N>() const noexcept
    {
        return v;
    }

    [[nodiscard]] explicit numeric_array(simd_type rhs) noexcept
        requires(has_simd_type)
        : v(static_cast<array_type>(rhs))
    {
    }

    [[nodiscard]] auto simd() const noexcept
        requires(has_simd_type)
    {
        return simd_type{v};
    }

#if defined(HI_HAS_SSE2)
    [[nodiscard]] __m128i reg() const noexcept
        requires(std::is_integral_v<T> and sizeof(T) * N == 16)
    {
        return _mm_loadu_si128(reinterpret_cast<__m128i const *>(v.data()));
    }

    [[nodiscard]] __m128i reg() const noexcept
        requires(is_f16x4)
    {
        return _mm_set_epi16(0, 0, 0, 0, get<3>(v).get(), get<2>(v).get(), get<1>(v).get(), get<0>(v).get());
    }
#endif

#if defined(HI_HAS_SSE2)
    [[nodiscard]] __m128 reg() const noexcept
        requires(is_f32x4)
    {
        return _mm_loadu_ps(v.data());
    }
#endif

#if defined(HI_HAS_SSE2)
    [[nodiscard]] __m128d reg() const noexcept
        requires(is_f64x2)
    {
        return _mm_loadu_pd(v.data());
    }
#endif

#if defined(HI_HAS_SSE2)
    [[nodiscard]] explicit numeric_array(__m128i const& rhs) noexcept
        requires(std::is_integral_v<T> and sizeof(T) * N == 16)
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(v.data()), rhs);
    }
#endif

#if defined(HI_HAS_SSE4_1)
    [[nodiscard]] explicit numeric_array(__m128i const& rhs) noexcept
        requires(is_f16x4)
        : v(std::bit_cast<decltype(v)>(_mm_extract_epi64(rhs, 0)))
    {
    }
#endif

#if defined(HI_HAS_SSE4_1)
    [[nodiscard]] explicit numeric_array(__m128i const& rhs) noexcept
        requires(is_u8x4)
        : v(std::bit_cast<decltype(v)>(_mm_extract_epi32(rhs, 0)))
    {
    }
#endif

#if defined(HI_HAS_SSE2)
    [[nodiscard]] explicit numeric_array(__m128 const& rhs) noexcept
        requires(is_f32x4)
    {
        _mm_storeu_ps(v.data(), rhs);
    }
#endif

#if defined(HI_HAS_SSE2)
    [[nodiscard]] explicit numeric_array(__m128d const& rhs) noexcept
        requires(is_f64x2)
    {
        _mm_storeu_pd(v.data(), rhs);
    }
#endif

#if defined(HI_HAS_SSE2)
    numeric_array& operator=(__m128i const& rhs) noexcept
        requires(std::is_integral_v<T> and sizeof(T) * N == 16)
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(v.data()), rhs);
        return *this;
    }
#endif

#if defined(HI_HAS_SSE2)
    numeric_array& operator=(__m128 const& rhs) noexcept
        requires(is_f32x4)
    {
        _mm_storeu_ps(v.data(), rhs);
        return *this;
    }
#endif

#if defined(HI_HAS_SSE2)
    numeric_array& operator=(__m128d const& rhs) noexcept
        requires(is_f64x2)
    {
        _mm_storeu_pd(v.data(), rhs);
        return *this;
    }
#endif

#if defined(HI_HAS_AVX)
    [[nodiscard]] __m256i reg() const noexcept
        requires(std::is_integral_v<T> and sizeof(T) * N == 32)
    {
        return _mm256_loadu_si256(reinterpret_cast<__m256i const *>(v.data()));
    }
#endif

#if defined(HI_HAS_AVX)
    [[nodiscard]] __m256 reg() const noexcept
        requires(is_f32x8)
    {
        return _mm256_loadu_ps(v.data());
    }
#endif

#if defined(HI_HAS_AVX)
    [[nodiscard]] __m256d reg() const noexcept
        requires(is_f64x4)
    {
        return _mm256_loadu_pd(v.data());
    }
#endif

#if defined(HI_HAS_AVX)
    [[nodiscard]] explicit numeric_array(__m256i const& rhs) noexcept
        requires(std::is_integral_v<T> and sizeof(T) * N == 32)
    {
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(v.data()), rhs);
    }
#endif

#if defined(HI_HAS_AVX)
    [[nodiscard]] explicit numeric_array(__m256 const& rhs) noexcept
        requires(is_f32x8)
    {
        _mm256_storeu_ps(v.data(), rhs);
    }
#endif

#if defined(HI_HAS_AVX)
    [[nodiscard]] explicit numeric_array(__m256d const& rhs) noexcept
        requires(is_f64x4)
    {
        _mm256_storeu_pd(v.data(), rhs);
    }
#endif

#if defined(HI_HAS_AVX)
    numeric_array& operator=(__m256i const& rhs) noexcept
        requires(std::is_integral_v<T> and sizeof(T) * N == 32)
    {
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(v.data()), rhs);
        return *this;
    }
#endif

#if defined(HI_HAS_AVX)
    numeric_array& operator=(__m256 const& rhs) noexcept
        requires(is_f32x8)
    {
        _mm256_storeu_ps(v.data(), rhs);
        return *this;
    }
#endif

#if defined(HI_HAS_AVX)
    numeric_array& operator=(__m256d const& rhs) noexcept
        requires(is_f64x4)
    {
        _mm256_storeu_pd(v.data(), rhs);
        return *this;
    }
#endif

    template<typename Other>
    [[nodiscard]] constexpr friend Other bit_cast(numeric_array const& rhs) noexcept
        requires(sizeof(Other) == sizeof(array_type))
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE2)
            if constexpr (Other::is_f32x4 and std::is_integral_v<T>) {
                return Other{_mm_castsi128_ps(rhs.reg())};
            } else if constexpr (Other::is_f32x4 and is_f64x2) {
                return Other{_mm_castpd_ps(rhs.reg())};
            } else if constexpr (Other::is_f64x2 and std::is_integral_v<T>) {
                return Other{_mm_castsi128_pd(rhs.reg())};
            } else if constexpr (Other::is_f64x2 and is_f32x4) {
                return Other{_mm_castps_pd(rhs.reg())};
            } else if constexpr (std::is_integral_v<typename Other::value_type> and is_f32x4) {
                return Other{_mm_castps_si128(rhs.reg())};
            } else if constexpr (std::is_integral_v<typename Other::value_type> and is_f64x2) {
                return Other{_mm_castpd_si128(rhs.reg())};
            } else if constexpr (std::is_integral_v<typename Other::value_type> and std::is_integral_v<T>) {
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
#if defined(HI_HAS_SSE2)
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
#if defined(HI_HAS_SSE)
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
     * @param[out] ptr A pointer to where the numeric array should be stored into memory.
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
            hilet ep = epsilon();
            // check if any of the elements is outside of epsilon range,
            return to_bool(gt(-ep, *this) | gt(*this, ep));
        } else {
            return to_bool(ne(*this, T{0}));
        }
    }

    [[nodiscard]] constexpr T const& operator[](std::size_t i) const noexcept
    {
        static_assert(std::endian::native == std::endian::little, "Indices need to be reversed on big endian machines");
        hi_axiom(i < N);
        return v[i];
    }

    [[nodiscard]] constexpr T& operator[](std::size_t i) noexcept
    {
        static_assert(std::endian::native == std::endian::little, "Indices need to be reversed on big endian machines");
        hi_axiom(i < N);
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

    [[nodiscard]] constexpr T x() const noexcept
        requires(N >= 1)
    {
        HI_X_runtime_evaluate_if_valid(get<0>(simd()));
        return std::get<0>(v);
    }

    [[nodiscard]] constexpr T y() const noexcept
        requires(N >= 2)
    {
        HI_X_runtime_evaluate_if_valid(get<1>(simd()));
        return std::get<1>(v);
    }

    [[nodiscard]] constexpr T z() const noexcept
        requires(N >= 3)
    {
        HI_X_runtime_evaluate_if_valid(get<2>(simd()));
        return std::get<2>(v);
    }

    [[nodiscard]] constexpr T w() const noexcept
        requires(N >= 4)
    {
        HI_X_runtime_evaluate_if_valid(get<3>(simd()));
        return std::get<3>(v);
    }

    [[nodiscard]] constexpr T& x() noexcept
        requires(N >= 1)
    {
        return std::get<0>(v);
    }

    [[nodiscard]] constexpr T& y() noexcept
        requires(N >= 2)
    {
        return std::get<1>(v);
    }

    [[nodiscard]] constexpr T& z() noexcept
        requires(N >= 3)
    {
        return std::get<2>(v);
    }

    [[nodiscard]] constexpr T& w() noexcept
        requires(N >= 4)
    {
        return std::get<3>(v);
    }

    [[nodiscard]] constexpr T r() const noexcept
        requires(N >= 1)
    {
        return x();
    }

    [[nodiscard]] constexpr T g() const noexcept
        requires(N >= 2)
    {
        return y();
    }

    [[nodiscard]] constexpr T b() const noexcept
        requires(N >= 3)
    {
        return z();
    }

    [[nodiscard]] constexpr T a() const noexcept
        requires(N >= 4)
    {
        return w();
    }

    [[nodiscard]] constexpr T& r() noexcept
        requires(N >= 1)
    {
        return x();
    }

    [[nodiscard]] constexpr T& g() noexcept
        requires(N >= 2)
    {
        return y();
    }

    [[nodiscard]] constexpr T& b() noexcept
        requires(N >= 3)
    {
        return z();
    }

    [[nodiscard]] constexpr T& a() noexcept
        requires(N >= 4)
    {
        return w();
    }

    [[nodiscard]] constexpr T width() const noexcept
        requires(N >= 1)
    {
        return x();
    }

    [[nodiscard]] constexpr T height() const noexcept
        requires(N >= 2)
    {
        return y();
    }

    [[nodiscard]] constexpr T depth() const noexcept
        requires(N >= 3)
    {
        return z();
    }

    [[nodiscard]] constexpr T& width() noexcept
        requires(N >= 1)
    {
        return x();
    }

    [[nodiscard]] constexpr T& height() noexcept
        requires(N >= 2)
    {
        return y();
    }

    [[nodiscard]] constexpr T& depth() noexcept
        requires(N >= 3)
    {
        return z();
    }

    constexpr numeric_array& operator<<=(unsigned int rhs) noexcept
    {
        return *this = *this << rhs;
    }

    constexpr numeric_array& operator>>=(unsigned int rhs) noexcept
    {
        return *this = *this >> rhs;
    }

    constexpr numeric_array& operator|=(numeric_array const& rhs) noexcept
    {
        return *this = *this | rhs;
    }

    constexpr numeric_array& operator|=(T const& rhs) noexcept
    {
        return *this = *this | rhs;
    }

    constexpr numeric_array& operator&=(numeric_array const& rhs) noexcept
    {
        return *this = *this & rhs;
    }

    constexpr numeric_array& operator&=(T const& rhs) noexcept
    {
        return *this = *this & rhs;
    }

    constexpr numeric_array& operator^=(numeric_array const& rhs) noexcept
    {
        return *this = *this ^ rhs;
    }

    constexpr numeric_array& operator^=(T const& rhs) noexcept
    {
        return *this = *this ^ rhs;
    }

    constexpr numeric_array& operator+=(numeric_array const& rhs) noexcept
    {
        return *this = *this + rhs;
    }

    constexpr numeric_array& operator+=(T const& rhs) noexcept
    {
        return *this = *this + rhs;
    }

    constexpr numeric_array& operator-=(numeric_array const& rhs) noexcept
    {
        return *this = *this - rhs;
    }

    constexpr numeric_array& operator-=(T const& rhs) noexcept
    {
        return *this = *this - rhs;
    }

    constexpr numeric_array& operator*=(numeric_array const& rhs) noexcept
    {
        return *this = *this * rhs;
    }

    constexpr numeric_array& operator*=(T const& rhs) noexcept
    {
        return *this = *this * rhs;
    }

    constexpr numeric_array& operator/=(numeric_array const& rhs) noexcept
    {
        return *this = *this / rhs;
    }

    constexpr numeric_array& operator/=(T const& rhs) noexcept
    {
        return *this = *this / rhs;
    }

    constexpr numeric_array& operator%=(numeric_array const& rhs) noexcept
    {
        return *this = *this % rhs;
    }

    constexpr numeric_array& operator%=(T const& rhs) noexcept
    {
        return *this = *this % rhs;
    }

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array
     */
    template<std::size_t I>
    [[nodiscard]] friend constexpr T& get(numeric_array& rhs) noexcept
    {
        static_assert(I < N, "Index out of bounds");
        return std::get<I>(rhs.v);
    }

    /** Get a element from the numeric array.
     *
     * @tparam I Index into the array
     */
    template<std::size_t I>
    [[nodiscard]] friend constexpr T get(numeric_array const& rhs) noexcept
    {
        static_assert(I < N, "Index out of bounds");
        HI_X_runtime_evaluate_if_valid(get<I>(rhs.simd()));
        return std::get<I>(rhs.v);
    }

    /** Insert a value in the array.
     *
     * @tparam I The index into the array.
     * @param lhs The vector to insert the value into.
     * @param rhs The value to insert.
     * @return The vector with the inserted value.
     */
    template<std::size_t I>
    [[nodiscard]] constexpr friend numeric_array insert(numeric_array const& lhs, value_type rhs) noexcept
    {
        static_assert(I < size);
        HI_X_runtime_evaluate_if_valid(numeric_array{insert<I>(lhs.simd(), rhs)});

        auto r = lhs;
        std::get<I>(r.v) = rhs;
        return r;
    }

    /** Set individual elements to zero.
     *
     * @tparam Mask bit mask where '1' means to zero, '0' to keep original.
     */
    template<std::size_t Mask = ~std::size_t{0}>
    [[nodiscard]] friend constexpr numeric_array zero(numeric_array rhs) noexcept
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE4_1)
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
            if (to_bool((Mask >> i) & 1)) {
                r.v[i] = T{0};
            } else {
                r.v[i] = rhs.v[i];
            }
        }
        return r;
    }

    /** Blend two numeric arrays.
     *
     * @tparam Mask One bit for each element selects; 0: lhs, 1: rhs.
     * @param lhs The left hand side
     * @param rhs The right hand side
     * @return The blended array.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr numeric_array blend(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{blend<Mask>(lhs.simd(), rhs.simd())});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = to_bool((Mask >> i) & 1) ? rhs[i] : lhs[i];
        }
        return r;
    }

    /** Blend the values using a dynamic mask.
     */
    [[nodiscard]] friend constexpr numeric_array blend(numeric_array const& a, numeric_array const& b, numeric_array const& mask)
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_AVX2)
            if constexpr (is_i8x32 or is_u8x32) {
                return numeric_array{_mm256_blendv_epi8(a.reg(), b.reg(), mask.reg())};
            }
#endif
#if defined(HI_HAS_AVX)
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
#if defined(HI_HAS_SSE4_1)
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

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{-rhs.simd()});
        return T{0} - rhs;
    }

    [[nodiscard]] friend constexpr numeric_array abs(numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{abs(rhs.simd())});
        return max(rhs, -rhs);
    }

    [[nodiscard]] friend constexpr numeric_array rcp(numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{rcp(rhs.simd())});
        return T{1} / rhs;
    }

    [[nodiscard]] friend constexpr numeric_array sqrt(numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{sqrt(rhs.simd())});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::sqrt(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array rcp_sqrt(numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{rcp_sqrt(rhs.simd())});
        return rcp(sqrt(rhs));
    }

    [[nodiscard]] friend constexpr numeric_array floor(numeric_array const& rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{floor(rhs.simd())});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::floor(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array ceil(numeric_array const& rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{ceil(rhs.simd())});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = std::ceil(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array round(numeric_array const& rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{round(rhs.simd())});

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
    [[nodiscard]] hi_force_inline friend constexpr T dot(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(get<0>(dot<Mask>(lhs.simd(), rhs.simd())));

        auto r = T{};
        for (std::size_t i = 0; i != N; ++i) {
            if (to_bool(Mask & (1_uz << i))) {
                r += lhs.v[i] * rhs.v[i];
            }
        }
        return r;
    }

    /** Take the length of the vector
     *
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param rhs The right hand side.
     * @return Result of the hypot calculation.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend T hypot(numeric_array const& rhs) noexcept
        requires(std::is_floating_point_v<value_type>)
    {
        HI_X_runtime_evaluate_if_valid(get<0>(sqrt(dot<Mask>(rhs.simd(), rhs.simd()))));
        return std::sqrt(dot<Mask>(rhs, rhs));
    }

    /** Take the squared length of the vector.
     *
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param rhs The right hand side.
     * @return Result of the hypot-squared calculation.
     */
    template<std::size_t Mask>
    [[nodiscard]] hi_force_inline friend constexpr T squared_hypot(numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(get<0>(dot<Mask>(rhs.simd(), rhs.simd())));
        return dot<Mask>(rhs, rhs);
    }

    /** Take a reciprocal of the length.
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param rhs The right hand side.
     * @return Result of the hypot-squared calculation.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr T rcp_hypot(numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(get<0>(rcp_sqrt(dot<Mask>(rhs.simd(), rhs.simd()))));
        return 1.0f / hypot<Mask>(rhs);
    }

    /** Normalize a vector.
     * All elements that do not participate in the normalization will be set to zero.
     *
     * @tparam Mask A mask for which elements participate in the normalization calculation.
     * @param rhs The right hand side.
     * @return The normalized vector.
     */
    template<std::size_t Mask>
    [[nodiscard]] friend constexpr numeric_array normalize(numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{rhs * rcp_sqrt(dot<Mask>(rhs.simd(), rhs.simd()))});

        hilet rcp_hypot_ = rcp_hypot<Mask>(rhs);

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            if (to_bool(Mask & (1_uz << i))) {
                r.v[i] = rhs.v[i] * rcp_hypot_;
            }
        }
        return r;
    }

    [[nodiscard]] friend constexpr std::size_t eq(numeric_array const& lhs, numeric_array const& rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        HI_X_runtime_evaluate_if_valid(eq(lhs.simd(), rhs.simd()).mask());

        std::size_t r = 0;
        for (std::size_t i = 0; i != N; ++i) {
            r |= static_cast<std::size_t>(lhs.v[i] == rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr std::size_t ne(numeric_array const& lhs, numeric_array const& rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        HI_X_runtime_evaluate_if_valid(ne(lhs.simd(), rhs.simd()).mask());

        constexpr std::size_t not_mask = (1 << N) - 1;
        return eq(lhs, rhs) ^ not_mask;
    }

    [[nodiscard]] friend constexpr std::size_t gt(numeric_array const& lhs, numeric_array const& rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        HI_X_runtime_evaluate_if_valid(gt(lhs.simd(), rhs.simd()).mask());

        unsigned int r = 0;
        for (std::size_t i = 0; i != N; ++i) {
            r |= static_cast<std::size_t>(lhs.v[i] > rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr std::size_t lt(numeric_array const& lhs, numeric_array const& rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        HI_X_runtime_evaluate_if_valid(lt(lhs.simd(), rhs.simd()).mask());
        return gt(rhs, lhs);
    }

    [[nodiscard]] friend constexpr std::size_t ge(numeric_array const& lhs, numeric_array const& rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        HI_X_runtime_evaluate_if_valid(ge(lhs.simd(), rhs.simd()).mask());
        constexpr std::size_t not_mask = (1 << N) - 1;
        return lt(lhs, rhs) ^ not_mask;
    }

    [[nodiscard]] friend constexpr std::size_t le(numeric_array const& lhs, numeric_array const& rhs) noexcept
        requires(N <= sizeof(std::size_t) * CHAR_BIT)
    {
        HI_X_runtime_evaluate_if_valid(le(lhs.simd(), rhs.simd()).mask());
        constexpr std::size_t not_mask = (1 << N) - 1;
        return gt(lhs, rhs) ^ not_mask;
    }

    [[nodiscard]] friend constexpr numeric_array gt_mask(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{gt(lhs.simd(), rhs.simd())});

        using uint_type = make_uintxx_t<sizeof(T) * CHAR_BIT>;
        constexpr auto ones = std::bit_cast<T>(~uint_type{0});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r[i] = lhs.v[i] > rhs.v[i] ? ones : T{0};
        }
        return r;
    }

    [[nodiscard]] friend constexpr bool operator==(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(lhs.simd() == rhs.simd());
        return not ne(lhs, rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator<<(numeric_array const& lhs, unsigned int rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() << rhs});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] << rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator>>(numeric_array const& lhs, unsigned int rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() >> rhs});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] >> rhs;
        }
        return r;
    }

    /** Rotate left.
     *
     * @note It is undefined behavior if: rhs <= 0 or rhs >= sizeof(value_type) * CHAR_BIT.
     */
    [[nodiscard]] friend constexpr numeric_array rotl(numeric_array const& lhs, unsigned int rhs) noexcept
    {
        hi_axiom(rhs > 0 and rhs < sizeof(value_type) * CHAR_BIT);

        hilet remainder = narrow_cast<unsigned int>(sizeof(value_type) * CHAR_BIT - rhs);

        return (lhs << rhs) | (lhs >> remainder);
    }

    /** Rotate right.
     *
     * @note It is undefined behavior if: rhs <= 0 or rhs >= sizeof(value_type) * CHAR_BIT.
     */
    [[nodiscard]] friend constexpr numeric_array rotr(numeric_array const& lhs, unsigned int rhs) noexcept
    {
        hi_axiom(rhs > 0 and rhs < sizeof(value_type) * CHAR_BIT);

        hilet remainder = narrow_cast<unsigned int>(sizeof(value_type) * CHAR_BIT - rhs);

        return (lhs >> rhs) | (lhs << remainder);
    }

    [[nodiscard]] friend constexpr numeric_array operator|(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() | rhs.simd()});

        using uint_type = make_uintxx_t<sizeof(T) * CHAR_BIT>;

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] =
                std::bit_cast<T>(static_cast<uint_type>(std::bit_cast<uint_type>(lhs.v[i]) | std::bit_cast<uint_type>(rhs.v[i])));
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator|(numeric_array const& lhs, T const& rhs) noexcept
    {
        return lhs | broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator|(T const& lhs, numeric_array const& rhs) noexcept
    {
        return broadcast(lhs) | rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator&(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() & rhs.simd()});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] & rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator&(numeric_array const& lhs, T const& rhs) noexcept
    {
        return lhs & broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator&(T const& lhs, numeric_array const& rhs) noexcept
    {
        return broadcast(lhs) & rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator^(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() ^ rhs.simd()});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] ^ rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator^(numeric_array const& lhs, T const& rhs) noexcept
    {
        return lhs ^ broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator^(T const& lhs, numeric_array const& rhs) noexcept
    {
        return broadcast(lhs) ^ rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator+(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() + rhs.simd()});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] + rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator+(numeric_array const& lhs, T const& rhs) noexcept
    {
        return lhs + broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator+(T const& lhs, numeric_array const& rhs) noexcept
    {
        return broadcast(lhs) + rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() - rhs.simd()});
        
        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] - rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const& lhs, T const& rhs) noexcept
    {
        return lhs - broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator-(T const& lhs, numeric_array const& rhs) noexcept
    {
        return broadcast(lhs) - rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator*(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() * rhs.simd()});
        
        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] * rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator*(numeric_array const& lhs, T const& rhs) noexcept
    {
        return lhs * broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator*(T const& lhs, numeric_array const& rhs) noexcept
    {
        return broadcast(lhs) * rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator/(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() / rhs.simd()});
        
        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] / rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator/(numeric_array const& lhs, T const& rhs) noexcept
    {
        return lhs / broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator/(T const& lhs, numeric_array const& rhs) noexcept
    {
        return broadcast(lhs) / rhs;
    }

    [[nodiscard]] friend constexpr numeric_array operator%(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{lhs.simd() % rhs.simd()});
        hilet div_result = floor(lhs / rhs);
        return lhs - (div_result * rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator%(numeric_array const& lhs, T const& rhs) noexcept
    {
        return lhs % broadcast(rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator%(T const& lhs, numeric_array const& rhs) noexcept
    {
        return broadcast(lhs) % rhs;
    }

    [[nodiscard]] friend constexpr numeric_array min(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{min(lhs.simd(), rhs.simd())});
        
        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = std::min(lhs.v[i], rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array max(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{max(lhs.simd(), rhs.simd())});

        auto r = numeric_array{};
        for (std::size_t i = 0; i != N; ++i) {
            r.v[i] = std::max(lhs.v[i], rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array
    clamp(numeric_array const& lhs, numeric_array const& low, numeric_array const& high) noexcept
    {
        return min(max(lhs, low), high);
    }

    [[nodiscard]] friend constexpr numeric_array hadd(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{horizontal_add(lhs.simd(), rhs.simd())});

        hi_axiom(N % 2 == 0);

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

    [[nodiscard]] friend constexpr numeric_array hsub(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        HI_X_runtime_evaluate_if_valid(numeric_array{horizontal_sub(lhs.simd(), rhs.simd())});
        
        hi_axiom(N % 2 == 0);

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
    [[nodiscard]] friend constexpr numeric_array addsub(numeric_array const& lhs, numeric_array const& rhs) noexcept
    {
        constexpr std::size_t not_mask = (1 << N) - 1;
        return lhs + neg<Mask ^ not_mask>(rhs);
    }

    /** Calculate the 2D normal on a 2D vector.
     */
    [[nodiscard]] friend constexpr numeric_array cross_2D(numeric_array const& rhs) noexcept
        requires(N >= 2)
    {
        return numeric_array{-rhs.y(), rhs.x()};
    }

    /** Calculate the 2D unit-normal on a 2D vector.
     */
    [[nodiscard]] friend constexpr numeric_array normal_2D(numeric_array const& rhs) noexcept
        requires(N >= 2)
    {
        return normalize<0b0011>(cross_2D(rhs));
    }

    /** Calculate the cross-product between two 2D vectors.
     * a.x * b.y - a.y * b.x
     */
    [[nodiscard]] friend constexpr float cross_2D(numeric_array const& lhs, numeric_array const& rhs) noexcept
        requires(N >= 2)
    {
        hilet tmp1 = rhs.yxwz();
        hilet tmp2 = lhs * tmp1;
        hilet tmp3 = hsub(tmp2, tmp2);
        return get<0>(tmp3);
    }

    // x=a.y*b.z - a.z*b.y
    // y=a.z*b.x - a.x*b.z
    // z=a.x*b.y - a.y*b.x
    // w=a.w*b.w - a.w*b.w
    [[nodiscard]] constexpr friend numeric_array cross_3D(numeric_array const& lhs, numeric_array const& rhs) noexcept
        requires(N == 4)
    {
        hilet a_left = lhs.yzxw();
        hilet b_left = rhs.zxyw();
        hilet left = a_left * b_left;

        hilet a_right = lhs.zxyw();
        hilet b_right = rhs.yzxw();
        hilet right = a_right * b_right;
        return left - right;
    }

    [[nodiscard]] static constexpr numeric_array byte_srl_shuffle_indices(unsigned int rhs)
        requires(is_i8x16)
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

    [[nodiscard]] static constexpr numeric_array byte_sll_shuffle_indices(unsigned int rhs)
        requires(is_i8x16)
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
    [[nodiscard]] friend constexpr numeric_array shuffle(numeric_array const& lhs, numeric_array const& rhs) noexcept
        requires(std::is_integral_v<value_type>)
    {
        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSSE3)
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
    [[nodiscard]] friend constexpr numeric_array midpoint(numeric_array const& p1, numeric_array const& p2) noexcept
    {
        return (p1 + p2) * 0.5f;
    }

    /** Find the point on the other side and at the same distance of an anchor-point.
     */
    [[nodiscard]] friend constexpr numeric_array reflect_point(numeric_array const& p, numeric_array const anchor) noexcept
    {
        return anchor - (p - anchor);
    }

    hi_warning_push();
    // C26494 Variable '...' is uninitialized. Always initialize an object (type.5).
    // Internal to _MM_TRANSPOSE4_PS
    hi_warning_ignore_msvc(26494);
    template<typename... Columns>
    [[nodiscard]] friend constexpr std::array<numeric_array, N> transpose(Columns const&...columns) noexcept
    {
        static_assert(sizeof...(Columns) == N, "Can only transpose square matrices");

        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE)
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
        auto f = [&r, &columns... ]<std::size_t... Ints>(std::index_sequence<Ints...>)
        {
            auto tf = [&r](auto i, auto v) {
                for (std::size_t j = 0; j != N; ++j) {
                    r[j][i] = v[j];
                }
                return 0;
            };
            static_cast<void>((tf(Ints, columns) + ...));
        };
        f(std::make_index_sequence<sizeof...(columns)>{});
        return r;
    }
    hi_warning_pop();

    [[nodiscard]] constexpr friend numeric_array composit(numeric_array const& under, numeric_array const& over) noexcept
        requires(N == 4 && std::is_floating_point_v<T>)
    {
        if (get<3>(over) <= value_type{0}) {
            // fully transparent.
            return under;
        }
        if (get<3>(over) >= value_type{1}) {
            // fully opaque;
            return over;
        }

        hilet over_alpha = over.wwww();
        hilet under_alpha = under.wwww();

        hilet over_color = over.xyz1();
        hilet under_color = under.xyz1();

        hilet output_color = over_color * over_alpha + under_color * under_alpha * (T{1} - over_alpha);

        return output_color / output_color.www1();
    }

    [[nodiscard]] constexpr friend numeric_array composit(numeric_array const& under, numeric_array const& over) noexcept
        requires(is_f16x4)
    {
        return numeric_array{composit(static_cast<numeric_array<float, 4>>(under), static_cast<numeric_array<float, 4>>(over))};
    }

    [[nodiscard]] friend std::string to_string(numeric_array const& rhs) noexcept
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

    friend std::ostream& operator<<(std::ostream& lhs, numeric_array const& rhs)
    {
        return lhs << to_string(rhs);
    }

    /** Insert an element from rhs into the result.
     * This function copies the lhs, then inserts one element from rhs into the result.
     * It also can clear any of the elements to zero.
     */
    template<std::size_t FromElement, std::size_t ToElement>
    [[nodiscard]] constexpr friend numeric_array insert(numeric_array const& lhs, numeric_array const& rhs)
    {
        auto r = numeric_array{};

        if (not std::is_constant_evaluated()) {
#if defined(HI_HAS_SSE4_1)
            if constexpr (is_f32x4) {
                constexpr uint8_t insert_mask = static_cast<uint8_t>((FromElement << 6) | (ToElement << 4));
                return numeric_array{_mm_insert_ps(lhs.reg(), rhs.reg(), insert_mask)};

            } else if constexpr (is_i32x4 or is_u32x4) {
                constexpr uint8_t insert_mask = static_cast<uint8_t>((FromElement << 6) | (ToElement << 4));
                return numeric_array{
                    _mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(lhs.reg()), _mm_castsi128_ps(rhs.reg()), insert_mask))};
            }
#endif
#if defined(HI_HAS_SSE2)
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
                hilet lhs_ = _mm_castsi128_pd(lhs.reg());
                hilet rhs_ = _mm_castsi128_pd(rhs.reg());

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
     * @tparam Order a list of elements encoded as characters, 'a' - 'z' for indices to elements.
     *         '0' for a literal zero and '1' for a literal one.
     * @return A new array with the elements ordered based on the @a Order.
     *         The elements at the end of the array are set to zero.
     */
    template<fixed_string Order>
    [[nodiscard]] constexpr numeric_array swizzle() const
    {
        static_assert(Order.size() <= N);

        HI_X_runtime_evaluate_if_valid(numeric_array{simd().swizzle<Order>()});

        auto r = numeric_array{};
        swizzle_detail<0, Order>(r);
        return r;
    }

#define SWIZZLE(name, str) \
    [[nodiscard]] constexpr numeric_array name() const noexcept \
        requires(sizeof(str) - 1 <= N) \
    { \
        return swizzle<str>(); \
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

    template<size_t I, fixed_string Order>
    constexpr void swizzle_detail(numeric_array& r) const noexcept
    {
        static_assert(I < size);

        // Get the source element, or '0'.
        constexpr char c = I < Order.size() ? get<I>(Order) : '0';

        if constexpr (c == '1') {
            r = insert<I>(r, value_type{1});
        } else if constexpr (c == '0') {
            r = insert<I>(r, value_type{0});
        } else {
            constexpr size_t src_index = c - 'a';
            static_assert(src_index < size);

            r = insert<I>(r, get<src_index>(*this));
        }

        if constexpr (I + 1 < size) {
            swizzle_detail<I + 1, Order>(r);
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

} // namespace hi::inline v1

template<class T, std::size_t N>
struct std::tuple_size<hi::numeric_array<T, N>> : std::integral_constant<std::size_t, N> {};

template<std::size_t I, class T, std::size_t N>
struct std::tuple_element<I, hi::numeric_array<T, N>> {
    using type = T;
};

hi_warning_pop();
