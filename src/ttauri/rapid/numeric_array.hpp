// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../architecture.hpp"
#include "../concepts.hpp"
#include "../cast.hpp"
#include "../type_traits.hpp"
#include "raw_numeric_array.hpp"
#if TT_X86_64_V2
#include "f32x4_x64v2.hpp"
#include "i8x16_x64v2.hpp"
#endif

#if TT_X86_64_V1
#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#endif
#if TT_X86_64_V2
#include <pmmintrin.h> // SSE3
#include <tmmintrin.h> // SSSE3
#include <smmintrin.h> // SSE4.1
#include <nmmintrin.h> // SSE4.2
#include <ammintrin.h> // SSE4A
#endif
#if TT_X86_64_V2_5
#include <immintrin.h> // AVX, AVX2, FMA
#endif

#include <cstdint>
#include <ostream>
#include <string>
#include <array>
#include <type_traits>
#include <concepts>
#include <bit>
#include <climits>

namespace tt {

template<arithmetic T, size_t N>
class numeric_array {
public:
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

    constexpr numeric_array() noexcept = default;
    constexpr numeric_array(numeric_array const &rhs) noexcept = default;
    constexpr numeric_array(numeric_array &&rhs) noexcept = default;
    constexpr numeric_array &operator=(numeric_array const &rhs) noexcept = default;
    constexpr numeric_array &operator=(numeric_array &&rhs) noexcept = default;

    template<arithmetic U, size_t M>
    [[nodiscard]] constexpr explicit numeric_array(numeric_array<U, M> const &other) noexcept : v()
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (x86_64_v2 and is_f64x2 and other.is_i32x4) {
                *this = numeric_array{_mm_cvtepi32_pd(other.reg())};
                return;
            } else if constexpr (x86_64_v2 and is_f32x4 and other.is_i32x4) {
                *this = numeric_array{_mm_cvtepi32_ps(other.reg())};
                return;
            } else if constexpr (x86_64_v2 and is_i64x4 and other.is_i32x4) {
                *this = numeric_array{_mm_cvtepi32_epi64(other.reg())};
                return;
            } else if constexpr (x86_64_v2 and is_i64x4 and other.is_i16x8) {
                *this = numeric_array{_mm_cvtepi16_epi64(other.reg())};
                return;
            } else if constexpr (x86_64_v2 and is_i32x4 and other.is_i16x8) {
                *this = numeric_array{_mm_cvtepi16_epi32(other.reg())};
                return;
            } else if constexpr (x86_64_v2 and is_i64x2 and other.is_i8x16) {
                *this = numeric_array{_mm_cvtepi8_epi64(other.reg())};
                return;
            } else if constexpr (x86_64_v2 and is_i32x4 and other.is_i8x16) {
                *this = numeric_array{_mm_cvtepi8_epi32(other.reg())};
                return;
            } else if constexpr (x86_64_v2 and is_i16x8 and other.is_i8x16) {
                *this = numeric_array{_mm_cvtepi8_epi16(other.reg())};
                return;
            } else if constexpr (x86_64_v2_5 and is_f64x4 and other.is_f32x4) {
                *this = numeric_array{_mm256_cvteps_pd(other.reg())};
                return;
            } else if constexpr (x86_64_v2_5 and is_f64x4 and other.is_i32x4) {
                *this = numeric_array{_mm256_cvtepi32_pd(other.reg())};
                return;
            } else if constexpr (x86_64_v2_5 and is_f32x4 and other.is_f64x4) {
                *this = numeric_array{_mm256_cvtpd_ps(other.reg())};
                return;
            } else if constexpr (x86_64_v2_5 and is_i32x4 and other.is_f64x4) {
                *this = numeric_array{_mm256_cvtpd_epi32(other.reg())};
                return;
            } else if constexpr (x86_64_v2_5 and is_i32x8 and other.is_f32x8) {
                *this = numeric_array{_mm256_cvtps_epi32(other.reg())};
                return;
            } else if constexpr (x86_64_v2_5 and is_f32x8 and other.is_i32x8) {
                *this = numeric_array{_mm256_cvtepi32_ps(other.reg())};
                return;
            }
        }

        for (size_t i = 0; i != N; ++i) {
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

    template<arithmetic U, size_t M>
    [[nodiscard]] constexpr explicit numeric_array(numeric_array<U, M> const &other1, numeric_array<U, M> const &other2) noexcept
        :
        v()
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (x86_64_v2_5 and is_f32x8 and other1.is_f32x4) {
                *this = numeric_array{_mm256_set_m128(other2.reg(), other1.reg())};
                return;
            } else if constexpr (x86_64_v2_5 and is_f64x4 and other1.is_f64x2) {
                *this = numeric_array{_mm256_set_m128d(other2.reg(), other1.reg())};
                return;
            } else if constexpr (
                x86_64_v2_5 and std::is_integral_v<T> and std::is_integral_v<U> and (sizeof(T) * N == 32) and
                (sizeof(U) * M == 16)) {
                *this = numeric_array{_mm256_set_m128i(other2.reg(), other1.reg())};
                return;
            }
        }

        for (size_t i = 0; i != N; ++i) {
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

    [[nodiscard]] constexpr numeric_array(std::initializer_list<T> rhs) noexcept : v()
    {
        auto src = std::begin(rhs);
        auto dst = std::begin(v);

        // Copy all values from the initializer list.
        while (src != std::end(rhs) && dst != std::end(v)) {
            *(dst++) = *(src++);
        }

        tt_axiom(
            dst != std::end(v) || src == std::end(rhs),
            "Expecting the std:initializer_list size to be <= to the size of the numeric array");

        // Set all other elements to zero
        while (dst != std::end(v)) {
            *(dst++) = {};
        }
    }

    [[nodiscard]] constexpr numeric_array(T const &first) noexcept requires(N == 1) : numeric_array({first}) {}

    template<arithmetic... Rest>
    requires(sizeof...(Rest) + 2 <= N)
        [[nodiscard]] constexpr numeric_array(T const &first, T const &second, Rest const &...rest) noexcept :
        numeric_array({first, second, narrow_cast<T>(rest)...})
    {
    }

    [[nodiscard]] static constexpr numeric_array broadcast(T rhs) noexcept
    {
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = rhs;
        }
        return r;
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

    [[nodiscard]] __m128i reg() const noexcept requires(x86_64_v2 and std::is_integral_v<T> and sizeof(T) * N == 16)
    {
        return _mm_loadu_si128(reinterpret_cast<__m128i const *>(v.data()));
    }

    [[nodiscard]] __m128 reg() const noexcept requires(x86_64_v2 and is_f32x4)
    {
        return _mm_loadu_ps(v.data());
    }

    [[nodiscard]] __m128d reg() const noexcept requires(x86_64_v2 and is_f64x2)
    {
        return _mm_loadu_pd(v.data());
    }

    [[nodiscard]] explicit numeric_array(__m128i const &rhs) noexcept
        requires(x86_64_v2 and std::is_integral_v<T> and sizeof(T) * N == 16)
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(v.data()), rhs);
    }

    [[nodiscard]] explicit numeric_array(__m128 const &rhs) noexcept requires(x86_64_v2 and is_f32x4)
    {
        _mm_storeu_ps(v.data(), rhs);
    }

    [[nodiscard]] explicit numeric_array(__m128d const &rhs) noexcept requires(x86_64_v2 and is_f64x2)
    {
        _mm_storeu_pd(v.data(), rhs);
    }

    numeric_array &operator=(__m128i const &rhs) noexcept requires(x86_64_v2 and std::is_integral_v<T> and sizeof(T) * N == 16)
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(v.data()), rhs);
        return *this;
    }

    numeric_array &operator=(__m128 const &rhs) noexcept requires(x86_64_v2 and is_f32x4)
    {
        _mm_storeu_ps(v.data(), rhs);
        return *this;
    }

    numeric_array &operator=(__m128d const &rhs) noexcept requires(x86_64_v2 and is_f64x2)
    {
        _mm_storeu_pd(v.data(), rhs);
        return *this;
    }

    [[nodiscard]] __m256i reg() const noexcept requires(x86_64_v2_5 and std::is_integral_v<T> and sizeof(T) * N == 32)
    {
        return _mm256_loadu_si256(reinterpret_cast<__m256i const *>(v.data()));
    }

    [[nodiscard]] __m256 reg() const noexcept requires(x86_64_v2_5 and is_f32x8)
    {
        return _mm256_loadu_ps(v.data());
    }

    [[nodiscard]] __m256d reg() const noexcept requires(x86_64_v2_5 and is_f64x4)
    {
        return _mm256_loadu_pd(v.data());
    }

    [[nodiscard]] explicit numeric_array(__m256i const &rhs) noexcept
        requires(x86_64_v2_5 and std::is_integral_v<T> and sizeof(T) * N == 32)
    {
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(v.data()), rhs);
    }

    [[nodiscard]] explicit numeric_array(__m256 const &rhs) noexcept requires(x86_64_v2_5 and is_f32x8)
    {
        _mm256_storeu_ps(v.data(), rhs);
    }

    [[nodiscard]] explicit numeric_array(__m256d const &rhs) noexcept requires(x86_64_v2_5 and is_f64x4)
    {
        _mm256_storeu_pd(v.data(), rhs);
    }

    numeric_array &operator=(__m256i const &rhs) noexcept requires(x86_64_v2_5 and std::is_integral_v<T> and sizeof(T) * N == 32)
    {
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(v.data()), rhs);
        return *this;
    }

    numeric_array &operator=(__m256 const &rhs) noexcept requires(x86_64_v2_5 and is_f32x8)
    {
        _mm256_storeu_ps(v.data(), rhs);
        return *this;
    }

    numeric_array &operator=(__m256d const &rhs) noexcept requires(x86_64_v2_5 and is_f64x4)
    {
        _mm256_storeu_pd(v.data(), rhs);
        return *this;
    }

    template<typename Other>
    requires(sizeof(Other) == sizeof(numeric_array)) [[nodiscard]] friend Other bit_cast(numeric_array const &rhs) noexcept
    {
        using rhs_value_type = typename std::remove_cvref_t<decltype(rhs)>::value_type;

        if constexpr (Other::is_f32x4 and std::is_integral_v<rhs_value_type> and x86_64_v2) {
            return Other{_mm_castsi128_ps(rhs.reg())};
        } else if constexpr (Other::is_f32x4 and rhs.is_f64x2 and x86_64_v2) {
            return Other{_mm_castpd_ps(rhs.reg())};
        } else if constexpr (Other::is_f64x2 and std::is_integral_v<rhs_value_type> and x86_64_v2) {
            return Other{_mm_castsi128_pd(rhs.reg())};
        } else if constexpr (Other::is_f64x2 and rhs.is_f32x4 and x86_64_v2) {
            return Other{_mm_castps_pd(rhs.reg())};
        } else if constexpr (std::is_integral_v<Other::value_type> and rhs.is_f32x4 and x86_64_v2) {
            return Other{_mm_castps_si128(rhs.reg())};
        } else if constexpr (std::is_integral_v<Other::value_type> and rhs.is_f64x2 and x86_64_v2) {
            return Other{_mm_castpd_si128(rhs.reg())};
        } else if constexpr (std::is_integral_v<Other::value_type> and std::is_integral_v<rhs_value_type> and x86_64_v2) {
            return Other{rhs.reg()};
        } else {
            return std::bit_cast<Other>(rhs);
        }
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

    /** Store a numeric array into memory.
     * @param [out]ptr A pointer to where the numeric array should be stored into memory.
     */
    constexpr void store(std::byte *ptr) const noexcept
    {
        std::memcpy(ptr, this, sizeof(*this));
    }

    [[nodiscard]] constexpr T const &operator[](size_t i) const noexcept
    {
        static_assert(std::endian::native == std::endian::little, "Indices need to be reversed on big endian machines");
        tt_axiom(i < N);
        return v[i];
    }

    [[nodiscard]] constexpr T &operator[](size_t i) noexcept
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
    template<size_t I>
    [[nodiscard]] friend constexpr T &get(numeric_array &rhs) noexcept
    {
        static_assert(I >= 0 && I < N, "Index out of bounds");
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
     * @tparam Mask bit mask where '1' means to negate, '0' to keep original.
     */
    template<size_t Mask = ~size_t{0}>
    [[nodiscard]] friend constexpr numeric_array zero(numeric_array rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 && x86_64_v2) {
                return numeric_array{f32x4_x64v2_zero<Mask & 0xf>(rhs.v)};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            if (static_cast<bool>((Mask >> i) & 1)) {
                r.v[i] = T{0};
            } else {
                r.v[i] = rhs.v[i];
            }
        }
        return r;
    }

    /** Negate individual elements.
     *
     * @tparam Mask bit mask where '1' means to negate, '0' to keep original.
     */
    template<size_t Mask = ~size_t{0}>
    [[nodiscard]] friend constexpr numeric_array neg(numeric_array rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 && x86_64_v2) {
                return numeric_array{f32x4_x64v2_neg<Mask & 0xf>(rhs.v)};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            if (static_cast<bool>((Mask >> i) & 1)) {
                r.v[i] = -rhs.v[i];
            } else {
                r.v[i] = rhs.v[i];
            }
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            // -rhs.v[i] will cause a memory load with msvc.
            r.v[i] = T{} - rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array abs(numeric_array const &rhs) noexcept
    {
        auto neg_rhs = -rhs;

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r.v[i] = rhs.v[i] < T{} ? neg_rhs.v[i] : rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array rcp(numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return numeric_array{_mm_rcp_ps(rhs.reg())};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = 1.0f / rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array sqrt(numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return numeric_array{_mm_sqrt_ps(rhs.reg())};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = std::sqrt(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array rcp_sqrt(numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return numeric_array{_mm_rcp_sqrt_ps(rhs.reg())};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = 1.0f / std::sqrt(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array floor(numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return numeric_array{_mm_floor_ps(rhs.reg())};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = std::floor(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array ceil(numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return numeric_array{_mm_ceil_ps(rhs.reg())};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r[i] = std::ceil(rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array round(numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return numeric_array{_mm_round_ps(rhs.reg(), _MM_FROUND_CUR_DIRECTION)};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
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
    template<size_t Mask>
    [[nodiscard]] friend constexpr T dot(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return f32x4_x64v2_dot<Mask>(lhs.v, rhs.v);
            }
        }

        auto r = T{};
        for (size_t i = 0; i != N; ++i) {
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
    template<size_t Mask>
    [[nodiscard]] friend constexpr T hypot(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && x86_64_v2 && !std::is_constant_evaluated()) {
            return f32x4_x64v2_hypot<Mask>(rhs.v);
        }
        return std::sqrt(dot<Mask>(rhs, rhs));
    }

    /** Take the squared length of the vector.
     *
     * @tparam Mask A mask for which elements participate in the hypot calculation.
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     * @return Result of the hypot-squared calculation.
     */
    template<size_t Mask>
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
    template<size_t Mask>
    [[nodiscard]] friend constexpr T rcp_hypot(numeric_array const &rhs) noexcept
    {
        if (is_f32x4 && x86_64_v2 && !std::is_constant_evaluated()) {
            return f32x4_x64v2_rcp_hypot<Mask>(rhs.v);
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
    template<size_t Mask>
    [[nodiscard]] friend constexpr numeric_array normalize(numeric_array const &rhs) noexcept
    {
        tt_axiom(rhs.is_vector());

        if (is_f32x4 && x86_64_v2 && !std::is_constant_evaluated()) {
            return numeric_array{f32x4_x64v2_normalize<Mask>(rhs.v)};
        }

        ttlet rcp_hypot_ = rcp_hypot<Mask>(rhs);

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            if (static_cast<bool>(Mask & (1_uz << i))) {
                r.v[i] = rhs.v[i] * rcp_hypot_;
            }
        }
        return r;
    }

    [[nodiscard]] friend constexpr unsigned int eq(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return static_cast<unsigned int>(_mm_movemask_ps(_mm_cmpeq_ps(lhs.reg(), rhs.reg())));
            }
        }

        unsigned int r = 0;
        for (size_t i = 0; i != N; ++i) {
            r |= static_cast<unsigned int>(lhs.v[i] == rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr unsigned int ne(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return static_cast<unsigned int>(_mm_movemask_ps(_mm_cmpne_ps(lhs.reg(), rhs.reg())));
            }
        }
        unsigned int r = 0;
        for (size_t i = 0; i != N; ++i) {
            r |= static_cast<unsigned int>(lhs.v[i] != rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr unsigned int lt(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return static_cast<unsigned int>(_mm_movemask_ps(_mm_cmplt_ps(lhs.reg(), rhs.reg())));
            }
        }
        unsigned int r = 0;
        for (size_t i = 0; i != N; ++i) {
            r |= static_cast<unsigned int>(lhs.v[i] < rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr unsigned int gt(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return static_cast<unsigned int>(_mm_movemask_ps(_mm_cmpgt_ps(lhs.reg(), rhs.reg())));
            }
        }
        unsigned int r = 0;
        for (size_t i = 0; i != N; ++i) {
            r |= static_cast<unsigned int>(lhs.v[i] > rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr unsigned int le(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return static_cast<unsigned int>(_mm_movemask_ps(_mm_cmple_ps(lhs.reg(), rhs.reg())));
            }
        }
        unsigned int r = 0;
        for (size_t i = 0; i != N; ++i) {
            r |= static_cast<unsigned int>(lhs.v[i] <= rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr unsigned int ge(numeric_array const &lhs, numeric_array const &rhs) noexcept
        requires(N <= sizeof(unsigned int) * CHAR_BIT)
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 and x86_64_v2) {
                return static_cast<unsigned int>(_mm_movemask_ps(_mm_cmpge_ps(lhs.reg(), rhs.reg())));
            }
        }
        unsigned int r = 0;
        for (size_t i = 0; i != N; ++i) {
            r |= static_cast<unsigned int>(lhs.v[i] >= rhs.v[i]) << i;
        }
        return r;
    }

    [[nodiscard]] friend constexpr bool operator==(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 && x86_64_v2) {
                // MSVC cannot vectorize comparison.
                return f32x4_x64v2_eq(lhs.v, rhs.v);
            }
        }

        auto r = true;
        for (size_t i = 0; i != N; ++i) {
            r &= (lhs.v[i] == rhs.v[i]);
        }
        return r;
    }

    [[nodiscard]] friend constexpr bool operator!=(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend constexpr numeric_array operator<<(numeric_array const &lhs, unsigned int rhs) noexcept
    {
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] << rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator>>(numeric_array const &lhs, unsigned int rhs) noexcept
    {
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] >> rhs;
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator|(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (std::is_integral_v<T> and x86_64_v2) {
                return numeric_array{_mm_or_si128(lhs.reg(), rhs.reg())};
            }
        }
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] | rhs.v[i];
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
        if (!std::is_constant_evaluated()) {
            if constexpr (std::is_integral_v<T> and x86_64_v2) {
                return numeric_array{_mm_and_si128(lhs.reg(), rhs.reg())};
            }
        }
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
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
        if (!std::is_constant_evaluated()) {
            if constexpr (std::is_integral_v<T> and x86_64_v2) {
                return numeric_array{_mm_xor_si128(lhs.reg(), rhs.reg())};
            }
        }
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
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
        if (!std::is_constant_evaluated()) {
            if constexpr (x86_64_v2_5 and lhs.is_f32x8 and rhs.is_f32x8) {
                return numeric_array{_mm256_add_ps(lhs.reg(), rhs.reg())};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
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

    [[nodiscard]] friend constexpr numeric_array hadd(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f64x2 and x86_64_v2) {
                return numeric_array{_mm_hadd_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x4 and x86_64_v2) {
                return numeric_array{_mm_hadd_ps(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i32x4 and x86_64_v2) {
                return numeric_array{_mm_hadd_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x8 and x86_64_v2) {
                return numeric_array{_mm_hadd_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x16 and x86_64_v2) {
                return numeric_array{_mm_hadd_epi8(lhs.reg(), rhs.reg())};
            }
        }

        tt_axiom(N % 2 == 0);

        auto r = numeric_array{};

        size_t src_i = 0;
        size_t dst_i = 0;
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
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f64x2 and x86_64_v2) {
                return numeric_array{_mm_hsub_pd(lhs.reg(), rhs.reg())};
            } else if constexpr (is_f32x4 and x86_64_v2) {
                return numeric_array{_mm_hsub_ps(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i32x4 and x86_64_v2) {
                return numeric_array{_mm_hsub_epi32(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i16x8 and x86_64_v2) {
                return numeric_array{_mm_hsub_epi16(lhs.reg(), rhs.reg())};
            } else if constexpr (is_i8x16 and x86_64_v2) {
                return numeric_array{_mm_hsub_epi8(lhs.reg(), rhs.reg())};
            }
        }

        tt_axiom(N % 2 == 0);

        auto r = numeric_array{};

        size_t src_i = 0;
        size_t dst_i = 0;
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

    [[nodiscard]] friend constexpr numeric_array operator-(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (x86_64_v2_5 and lhs.is_f32x8 and rhs.is_f32x8) {
                return numeric_array{_mm256_sub_ps(lhs.reg(), rhs.reg())};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
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

    /** Add or subtract individual elements.
     *
     * @tparam Mask bit mask where '1' means to add, '0' means to subtract.
     */
    template<size_t Mask = ~size_t{0}>
    [[nodiscard]] friend constexpr numeric_array addsub(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 && x86_64_v2) {
                return numeric_array{f32x4_x64v2_addsub<Mask & 0xf>(lhs.v, rhs.v)};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            if (static_cast<bool>((Mask >> i) & 1)) {
                r.v[i] = lhs.v[i] + rhs.v[i];
            } else {
                r.v[i] = lhs.v[i] - rhs.v[i];
            }
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array operator*(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (x86_64_v2_5 and lhs.is_f32x8 and rhs.is_f32x8) {
                return numeric_array{_mm256_mul_ps(lhs.reg(), rhs.reg())};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
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
        if (!std::is_constant_evaluated()) {
            if constexpr (x86_64_v2_5 and lhs.is_f32x8 and rhs.is_f32x8) {
                return numeric_array{_mm256_div_ps(lhs.reg(), rhs.reg())};
            }
        }

        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
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
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            r.v[i] = lhs.v[i] % rhs.v[i];
        }
        return r;
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
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            // std::min() causes vectorization failure with msvc
            r.v[i] = lhs.v[i] < rhs.v[i] ? lhs.v[i] : rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array max(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            // std::max() causes vectorization failure with msvc
            r.v[i] = lhs.v[i] > rhs.v[i] ? lhs.v[i] : rhs.v[i];
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array
    clamp(numeric_array const &lhs, numeric_array const &low, numeric_array const &high) noexcept
    {
        auto r = numeric_array{};
        for (size_t i = 0; i != N; ++i) {
            // std::clamp() causes vectorization failure with msvc
            r.v[i] = lhs.v[i] < low.v[i] ? low.v[i] : lhs.v[i] > high.v[i] ? high.v[i] : lhs.v[i];
        }
        return r;
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
     */
    [[nodiscard]] friend constexpr float cross_2D(numeric_array const &lhs, numeric_array const &rhs) noexcept requires(N >= 2)
    {
        if (is_f32x4 && x86_64_v2 && !std::is_constant_evaluated()) {
            return f32x4_x64v2_viktor_cross(lhs.v, rhs.v);

        } else {
            return lhs.x() * rhs.y() - lhs.y() * rhs.x();
        }
    }

    // x=a.y*b.z - a.z*b.y
    // y=a.z*b.x - a.x*b.z
    // z=a.x*b.y - a.y*b.x
    // w=a.w*b.w - a.w*b.w
    [[nodiscard]] constexpr friend numeric_array cross_3D(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 && x86_64_v2) {
                return numeric_array{f32x4_x64v2_cross(lhs.v, rhs.v)};
            }
        }

        return numeric_array{
            lhs.y() * rhs.z() - lhs.z() * rhs.y(),
            lhs.z() * rhs.x() - lhs.x() * rhs.z(),
            lhs.x() * rhs.y() - lhs.y() * rhs.x(),
            0.0f};
    }

    // w + x*i + y*j + z*k
    //
    //   (w1*x2 + x1*w2 + y1*z2 - z1*y2)i
    // + (w1*y2 - x1*z2 + y1*w2 + z1*x2)j
    // + (w1*z2 + x1*y2 - y1*x2 + z1*w2)k
    // + (w1*w2 - x1*x2 - y1*y2 - z1*z2)
    template<int D>
    requires(D == 4) [[nodiscard]] friend numeric_array
        hamilton_cross(numeric_array const &lhs, numeric_array const &rhs) noexcept
    {
        ttlet col0 = lhs.wwww() * rhs;
        ttlet col1 = lhs.xxxx() * rhs.wzyx();
        ttlet col2 = lhs.yyyy() * rhs.zwxy();
        ttlet col3 = lhs.zzzz() * rhs.yxwz();

        ttlet col01 = addsub(col0, col1);
        ttlet col012 = addsub(col01.xzyw(), col2.xzyw()).xzyw();

        return numeric_array{

        };
    }

    /** Shift the elements left.
     * Other element are set to zero.
     */
    [[nodiscard]] friend constexpr numeric_array shift_left(numeric_array const &lhs, int rhs) noexcept
    {
        numeric_array r;
        for (size_t i = 0; i != N; ++i) {
            if ((i - rhs) >= 0) {
                r[i] = lhs[i - rhs];
            } else {
                r[i] = T{};
            }
        }
        return r;
    }

    /** Shift the elements left.
     * Other element are set to zero.
     */
    [[nodiscard]] friend constexpr numeric_array shift_right(numeric_array const &lhs, int rhs) noexcept
    {
        numeric_array r;
        for (size_t i = 0; i != N; ++i) {
            if ((i + rhs) < N) {
                r[i] = lhs[i + rhs];
            } else {
                r[i] = T{};
            }
        }
        return r;
    }

    [[nodiscard]] friend constexpr numeric_array
    blend(numeric_array const &a, numeric_array const &b, numeric_array const &mask) requires(is_i8x16)
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (x86_64_v2) {
                return numeric_array{_mm_blendv_epi8(a.reg(), b.reg(), mask.reg())};
            }
        }

        auto r = numeric_array{};

        for (size_t i = 0; i != N; ++i) {
            r[i] = mask[i] >= 0 ? a[i] : b[i];
        }

        return r;
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
    [[nodiscard]] friend constexpr numeric_array shuffle(numeric_array const &lhs, numeric_array const &rhs) requires(is_i8x16)
    {
        if (!std::is_constant_evaluated()) {
            if constexpr (x86_64_v2) {
                return numeric_array{_mm_shuffle_epi8(lhs.reg(), rhs.reg())};
            }
        }

        auto r = numeric_array{};

        for (size_t i = 0; i != N; ++i) {
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

        auto r = std::array<numeric_array, N>{};

        if (is_f32x4 && x86_64_v2 && !std::is_constant_evaluated()) {
            auto tmp = f32x4_x64v2_transpose(columns.v...);
            for (int i = 0; i != N; ++i) {
                r[i] = numeric_array{tmp[i]};
            }

        } else {
            transpose_detail<0, Columns...>(columns..., r);
        }

        return r;
    }

    [[nodiscard]] friend numeric_array composit(numeric_array const &under, numeric_array const &over) noexcept
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

    [[nodiscard]] friend std::string to_string(numeric_array const &rhs) noexcept
    {
        auto r = std::string{};

        r += '(';
        for (size_t i = 0; i != N; ++i) {
            if (i != 0) {
                r += "; ";
            }
            r += fmt::format("{}", rhs[i]);
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
    template<size_t FromElement, size_t ToElement, size_t ZeroMask = 0>
    [[nodiscard]] constexpr friend numeric_array insert(numeric_array const &lhs, numeric_array const &rhs)
    {
        auto r = numeric_array{};

        if (!std::is_constant_evaluated()) {
            if constexpr (is_f32x4 && x86_64_v2) {
                return numeric_array{f32x4_x64v2_insert<FromElement, ToElement, ZeroMask>(lhs.v, rhs.v)};
            } else if constexpr (is_u64x2 and x86_64_v2) {
                return numeric_array{u64x2_x64v2_insert<FromElement, ToElement, ZeroMask>(lhs.v, rhs.v)};
            }
        }

        for (size_t i = 0; i != N; ++i) {
            if ((ZeroMask >> i) & 1) {
                r[i] = T{};
            } else if (i == ToElement) {
                r[i] = rhs[FromElement];
            } else {
                r[i] = lhs[i];
            }
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
            if constexpr (is_f32x4 && x86_64_v2) {
                return numeric_array{f32x4_x64v2_swizzle<Elements...>(v)};
            } else if constexpr (is_u64x2 and x86_64_v2) {
                return numeric_array{u64x2_x64v2_swizzle<Elements...>(v)};
            }
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

private:
    container_type v;

    template<int I, typename First, typename... Rest>
    friend constexpr void transpose_detail(First const &first, Rest const &...rest, std::array<numeric_array, N> &r) noexcept
    {
        for (size_t j = 0; j != N; ++j) {
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

} // namespace tt

namespace std {
template<class T, std::size_t N>
struct tuple_size<tt::numeric_array<T, N>> : std::integral_constant<std::size_t, N> {
};

template<std::size_t I, class T, std::size_t N>
struct tuple_element<I, tt::numeric_array<T, N>> {
    using type = T;
};

} // namespace std
