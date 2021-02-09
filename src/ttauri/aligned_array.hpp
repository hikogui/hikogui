// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "os_detect.hpp"

#if TT_PROCESSOR == TT_CPU_X64
#include <xmmintrin.h>
#include <immintrin.h>
#endif

#include <array>
#include <algorithm>
#include <type_traits>
#include <bit>
#include <stdexcept>

namespace tt {

template<typename T, size_t N>
struct alignas(sizeof(T) * std::bit_ceil(N)) aligned_array {
public:
    using container_type = std::array<T,N>;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
    using difference_type = typename container_type::difference_type;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;
    using pointer = typename container_type::pointer;
    using const_pointer = typename container_type::const_pointer;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    constexpr aligned_array() = default;
    constexpr aligned_array(aligned_array const &) noexcept = default;
    constexpr aligned_array(aligned_array &&) noexcept = default;
    constexpr aligned_array &operator=(aligned_array const &) noexcept = default;
    constexpr aligned_array &operator=(aligned_array &&) noexcept = default;

    [[nodiscard]] constexpr aligned_array(std::initializer_list<T> rhs) noexcept : v(rhs) {}

    [[nodiscard]] explicit aligned_array(__m128 const &rhs) noexcept requires(N == 4 && std::is_same_v<T, float> && has_sse)
    {
        _mm_store_ps(v.data(), rhs);
    }

    [[nodiscard]] explicit aligned_array(__m128d const &rhs) noexcept requires(N == 2 && std::is_same_v<T, double> && has_sse)
    {
        _mm_store_pd(v.data(), rhs);
    }

    [[nodiscard]] explicit aligned_array(__m128i const &rhs) noexcept
        requires(std::is_integral_v<T> &&std::is_signed_v<T> && sizeof(T) * N == 16 && has_sse)
    {
        _mm_store_si128(reinterpret_cast<__m128i *>(v.data()), rhs);
    }

    [[nodiscard]] explicit aligned_array(__m256 const &rhs) noexcept requires(N == 8 && std::is_same_v<T, float> && has_sse)
    {
        _mm256_store_ps(v.data(), rhs);
    }

    [[nodiscard]] explicit aligned_array(__m256d const &rhs) noexcept requires(N == 4 && std::is_same_v<T, double> && has_sse)
    {
        _mm256_store_pd(v.data(), rhs);
    }

    [[nodiscard]] explicit aligned_array(__m256i const &rhs) noexcept
        requires(std::is_integral_v<T> &&std::is_signed_v<T> && sizeof(T) * N == 32 && has_sse)
    {
        _mm256_store_si256(reinterpret_cast<__m256i *>(v.data()), rhs);
    }

    [[nodiscard]] explicit operator __m128() const noexcept requires(N == 4 && std::is_same_v<T, float> && has_sse)
    {
        return _mm_load_ps(v.data());
    }

    [[nodiscard]] explicit operator __m128d() const noexcept requires(N == 2 && std::is_same_v<T, double> && has_sse)
    {
        return _mm_load_pd(v.data());
    }

    [[nodiscard]] explicit operator __m128i() const noexcept
        requires(std::is_integral_v<T> &&std::is_signed_v<T> && sizeof(T) * N == 16 && has_sse)
    {
        return _mm_load_si128(reinterpret_cast<__m128i*>(data()));
    }

    [[nodiscard]] explicit operator __m256() const noexcept requires(N == 8 && std::is_same_v<T, float> && has_sse)
    {
        return _mm256_load_ps(data());
    }

    [[nodiscard]] explicit operator __m256d() const noexcept requires(N == 4 && std::is_same_v<T, double> && has_sse)
    {
        return _mm256_load_pd(data());
    }

    [[nodiscard]] explicit operator __m256i() const noexcept
        requires(std::is_integral_v<T> &&std::is_signed_v<T> && sizeof(T) * N == 32 && has_sse)
    {
        return _mm256_load_si256(reinterpret_cast<__m256i *>(data()));
    }

    /** Select item at pos.
     * @param pos The position in the array.
     * @return A reference to the item.
     * @throws std::out_of_range When pos is out of range.
     */
    [[nodiscard]] constexpr reference at(size_t pos)
    {
        if (pos < size()) {
            throw std::out_of_range("index out of range");
        }
        return v[pos];
    }

    [[nodiscard]] constexpr const_reference at(size_t pos) const
    {
        if (pos < size()) {
            throw std::out_of_range("index out of range");
        }
        return v[pos];
    }

    [[nodiscard]] constexpr reference operator[](size_t pos) noexcept
    {
        return v[pos];
    }

    [[nodiscard]] constexpr const_reference operator[](size_t pos) const noexcept
    {
        return v[pos];
    }

    [[nodiscard]] constexpr reference front() noexcept
    {
        return v[0];
    }

    [[nodiscard]] constexpr const_reference front() const noexcept
    {
        return v[0];
    }

    [[nodiscard]] constexpr reference back() noexcept
    {
        return v[N - 1];
    }

    [[nodiscard]] constexpr const_reference back() const noexcept
    {
        return v[N - 1];
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

    constexpr void fill(T const &value) noexcept
    {
        for (size_t i = 0; i != N; ++i) {
            v[i] = value;
        }
    }

    constexpr void swap(aligned_array &other) noexcept
    {
        for (size_t i = 0; i != N; ++i) {
            std::swap(v[i], other[i]);
        }
    }

    constexpr void swap(std::array<T, N> &other) noexcept
    {
        for (size_t i = 0; i != N; ++i) {
            std::swap(v[i], other[i]);
        }
    }

    template<std::size_t I>
    [[nodiscard]] friend constexpr T &get(aligned_array &rhs) noexcept
    {
        static_assert(I < N, "Index out of bounds");
        return get<I>(rhs.v);
    }

    template<std::size_t I>
    [[nodiscard]] friend constexpr T get(aligned_array &&rhs) noexcept
    {
        static_assert(I < N, "Index out of bounds");
        return get<I>(rhs.v);
    }

    template<std::size_t I>
    [[nodiscard]] friend constexpr T const &get(aligned_array const &rhs) noexcept
    {
        static_assert(I < N, "Index out of bounds");
        return get<I>(rhs.v);
    }

    [[nodiscard]] constexpr friend bool operator==(aligned_array const &lhs, aligned_array const &rhs) noexcept
    {
        for (size_t i = 0; i != N; ++i) {
            if (!(lhs == rhs)) {
                return false;
            }
        }
        return true;
    }

private:
    container_type v;
};

template<class T, class... U>
aligned_array(T, U...) -> aligned_array<T, 1 + sizeof...(U)>;

using i8x1_raw = aligned_array<int8_t, 1>;
using i8x2_raw = aligned_array<int8_t, 2>;
using i8x4_raw = aligned_array<int8_t, 4>;
using i8x8_raw = aligned_array<int8_t, 8>;
using i8x16_raw = aligned_array<int8_t, 16>;
using i8x32_raw = aligned_array<int8_t, 32>;
using i8x64_raw = aligned_array<int8_t, 64>;

using u8x1_raw = aligned_array<uint8_t, 1>;
using u8x2_raw = aligned_array<uint8_t, 2>;
using u8x4_raw = aligned_array<uint8_t, 4>;
using u8x8_raw = aligned_array<uint8_t, 8>;
using u8x16_raw = aligned_array<uint8_t, 16>;
using u8x32_raw = aligned_array<uint8_t, 32>;
using u8x64_raw = aligned_array<uint8_t, 64>;

using i16x1_raw = aligned_array<int16_t, 1>;
using i16x2_raw = aligned_array<int16_t, 2>;
using i16x4_raw = aligned_array<int16_t, 4>;
using i16x8_raw = aligned_array<int16_t, 8>;
using i16x16_raw = aligned_array<int16_t, 16>;
using i16x32_raw = aligned_array<int16_t, 32>;

using u16x1_raw = aligned_array<uint16_t, 1>;
using u16x2_raw = aligned_array<uint16_t, 2>;
using u16x4_raw = aligned_array<uint16_t, 4>;
using u16x8_raw = aligned_array<uint16_t, 8>;
using u16x16_raw = aligned_array<uint16_t, 16>;
using u16x32_raw = aligned_array<uint16_t, 32>;

using i32x1_raw = aligned_array<int32_t, 1>;
using i32x2_raw = aligned_array<int32_t, 2>;
using i32x4_raw = aligned_array<int32_t, 4>;
using i32x8_raw = aligned_array<int32_t, 8>;
using i32x16_raw = aligned_array<int32_t, 16>;

using u32x1_raw = aligned_array<uint32_t, 1>;
using u32x2_raw = aligned_array<uint32_t, 2>;
using u32x4_raw = aligned_array<uint32_t, 4>;
using u32x8_raw = aligned_array<uint32_t, 8>;
using u32x16_raw = aligned_array<uint32_t, 16>;

using f32x1_raw = aligned_array<float, 1>;
using f32x2_raw = aligned_array<float, 2>;
using f32x4_raw = aligned_array<float, 4>;
using f32x8_raw = aligned_array<float, 8>;
using f32x16_raw = aligned_array<float, 16>;

using i64x1_raw = aligned_array<int64_t, 1>;
using i64x2_raw = aligned_array<int64_t, 2>;
using i64x4_raw = aligned_array<int64_t, 4>;
using i64x8_raw = aligned_array<int64_t, 8>;

using u64x1_raw = aligned_array<uint64_t, 1>;
using u64x2_raw = aligned_array<uint64_t, 2>;
using u64x4_raw = aligned_array<uint64_t, 4>;
using u64x8_raw = aligned_array<uint64_t, 8>;

using f64x1_raw = aligned_array<double, 1>;
using f64x2_raw = aligned_array<double, 2>;
using f64x4_raw = aligned_array<double, 4>;
using f64x8_raw = aligned_array<double, 8>;


} // namespace tt

namespace std {

template<class T, std::size_t N>
struct tuple_size<tt::aligned_array<T, N>> : std::integral_constant<std::size_t, N> {
};

template<std::size_t I, class T, std::size_t N>
struct tuple_element<I, tt::aligned_array<T, N>> {
    using type = T;
};

} // namespace std
