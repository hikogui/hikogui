// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "architecture.hpp"
#include "memory.hpp"
#include "assert.hpp"
#include "cast.hpp"
#include "concepts.hpp"

#ifdef HI_HAS_SSE
#include <immintrin.h>
#endif
#ifdef HI_HAS_SSE2
#include <emmintrin.h>
#endif
#ifdef HI_HAS_SSE4_1
#include <smmintrin.h>
#endif
#if HI_COMPILER == HI_CC_MSVC
#include <stdlib.h>
#endif
#include <bit>
#include <concepts>

hi_warning_push();
// C26472: Don't use a static_cast for arithmetic conversions. Use brace initialization, gsl::narrow_cast or gsl::narrow
// (type.1).
// static_cast are used to cheaply cast integers to unsigned and back, for byteswapping.
hi_warning_ignore_msvc(26472);

namespace hi::inline v1 {

template<std::unsigned_integral T>
[[nodiscard]] T byte_swap(T x) noexcept
{
#if HI_COMPILER == HI_CC_CLANG || HI_COMPILER == HI_CC_GCC
    if constexpr (sizeof(T) == sizeof(uint64_t)) {
        return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(x)));
    } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
        return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(x)));
    } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
        return static_cast<T>(__builtin_bswap16(static_cast<uint16_t>(x)));
    } else {
        hi_no_default();
    }
#elif HI_COMPILER == HI_CC_MSVC
    if constexpr (sizeof(T) == sizeof(uint64_t)) {
        return static_cast<T>(_byteswap_uint64(static_cast<uint64_t>(x)));
    } else if constexpr (sizeof(T) == sizeof(unsigned long)) {
        return static_cast<T>(_byteswap_ulong(static_cast<unsigned long>(x)));
    } else if constexpr (sizeof(T) == sizeof(unsigned short)) {
        return static_cast<T>(_byteswap_ushort(static_cast<unsigned short>(x)));
    } else {
        hi_no_default();
    }
#else
#error "Byteswap not implemented for this compiler."
#endif
}

template<std::signed_integral T>
[[nodiscard]] T byte_swap(T x) noexcept
{
    return static_cast<T>(byte_swap(static_cast<std::make_unsigned_t<T>>(x)));
}

template<std::floating_point T>
[[nodiscard]] T byte_swap(T x) noexcept
{
    if constexpr (std::is_same_v<T, float>) {
        auto utmp = std::bit_cast<uint32_t>(x);
        utmp = byte_swap(utmp);
        return std::bit_cast<float>(x);
    } else if constexpr (std::is_same_v<T, double>) {
        auto utmp = std::bit_cast<uint64_t>(x);
        utmp = byte_swap(utmp);
        return std::bit_cast<double>(x);
    } else {
        hi_no_default();
    }
}

template<std::integral T>
[[nodiscard]] T little_to_native(T x)
{
    if constexpr (std::endian::native == std::endian::little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<std::integral T>
[[nodiscard]] T big_to_native(T x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<std::integral T>
[[nodiscard]] T native_to_little(T x)
{
    if constexpr (std::endian::native == std::endian::little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<std::integral T>
[[nodiscard]] T native_to_big(T x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T, std::endian E, std::size_t A = alignof(T)>
struct endian_buf_t {
    alignas(A) std::byte _value[sizeof(T)];

    [[nodiscard]] T value() const noexcept
    {
        T x;
        std::memcpy(&x, &_value[0], sizeof(T));

        return E == std::endian::native ? x : byte_swap(x);
    }

    endian_buf_t& set_value(T x) noexcept
    {
        if constexpr (E != std::endian::native) {
            x = byte_swap(x);
        }

        std::memcpy(&_value[0], &x, sizeof(T));
        return *this;
    }

    endian_buf_t& operator=(T x) noexcept
    {
        return set_value(x);
    }

    operator T() const noexcept
    {
        return value();
    }
};

using big_uint64_buf_t = endian_buf_t<uint64_t, std::endian::big, 1>;
using big_uint32_buf_t = endian_buf_t<uint32_t, std::endian::big, 1>;
using big_uint16_buf_t = endian_buf_t<uint16_t, std::endian::big, 1>;
using big_int64_buf_t = endian_buf_t<int64_t, std::endian::big, 1>;
using big_int32_buf_t = endian_buf_t<int32_t, std::endian::big, 1>;
using big_int16_buf_t = endian_buf_t<int16_t, std::endian::big, 1>;
using little_uint64_buf_t = endian_buf_t<uint64_t, std::endian::little, 1>;
using little_uint32_buf_t = endian_buf_t<uint32_t, std::endian::little, 1>;
using little_uint16_buf_t = endian_buf_t<uint16_t, std::endian::little, 1>;
using little_int64_buf_t = endian_buf_t<int64_t, std::endian::little, 1>;
using little_int32_buf_t = endian_buf_t<int32_t, std::endian::little, 1>;
using little_int16_buf_t = endian_buf_t<int16_t, std::endian::little, 1>;
using native_uint64_buf_t = endian_buf_t<uint64_t, std::endian::native, 1>;
using native_uint32_buf_t = endian_buf_t<uint32_t, std::endian::native, 1>;
using native_uint16_buf_t = endian_buf_t<uint16_t, std::endian::native, 1>;
using native_int64_buf_t = endian_buf_t<int64_t, std::endian::native, 1>;
using native_int32_buf_t = endian_buf_t<int32_t, std::endian::native, 1>;
using native_int16_buf_t = endian_buf_t<int16_t, std::endian::native, 1>;

using big_uint64_buf_at = endian_buf_t<uint64_t, std::endian::big>;
using big_uint32_buf_at = endian_buf_t<uint32_t, std::endian::big>;
using big_uint16_buf_at = endian_buf_t<uint16_t, std::endian::big>;
using big_int64_buf_at = endian_buf_t<int64_t, std::endian::big>;
using big_int32_buf_at = endian_buf_t<int32_t, std::endian::big>;
using big_int16_buf_at = endian_buf_t<int16_t, std::endian::big>;
using little_uint64_buf_at = endian_buf_t<uint64_t, std::endian::little>;
using little_uint32_buf_at = endian_buf_t<uint32_t, std::endian::little>;
using little_uint16_buf_at = endian_buf_t<uint16_t, std::endian::little>;
using little_int64_buf_at = endian_buf_t<int64_t, std::endian::little>;
using little_int32_buf_at = endian_buf_t<int32_t, std::endian::little>;
using little_int16_buf_at = endian_buf_t<int16_t, std::endian::little>;
using native_uint64_buf_at = endian_buf_t<uint64_t, std::endian::native>;
using native_uint32_buf_at = endian_buf_t<uint32_t, std::endian::native>;
using native_uint16_buf_at = endian_buf_t<uint16_t, std::endian::native>;
using native_int64_buf_at = endian_buf_t<int64_t, std::endian::native>;
using native_int32_buf_at = endian_buf_t<int32_t, std::endian::native>;
using native_int16_buf_at = endian_buf_t<int16_t, std::endian::native>;

/** Load an integer from unaligned memory in native byte-order.
 */
template<numeric T>
[[nodiscard]] hi_force_inline T load(void const *src) noexcept
{
    return load<T>(reinterpret_cast<uint8_t const *>(src));
}

/** Load an integer from unaligned memory in little-endian byte-order.
 */
template<numeric T>
hi_force_inline T load_le(void const *src) noexcept
{
    return little_to_native(load<T>(src));
}

/** Load an integer from unaligned memory in big-endian byte-order.
 */
template<numeric T>
hi_force_inline T load_be(void const *src) noexcept
{
    return big_to_native(load<T>(src));
}

/** Load data from memory.
 *
 * @param[out] r The return value, overwrites all bits in the value at @a offset.
 * @param src The memory location to read the data from.
 */
template<std::unsigned_integral T>
hi_force_inline void unaligned_load_le(T& r, void const *src) noexcept
{
#ifdef HI_HAS_SSE4_1
    if constexpr (sizeof(T) == 8) {
        r = _mm_extract_epi64(_mm_loadu_si64(src), 0);
    } else if constexpr (sizeof(T) == 4) {
        r = _mm_extract_epi32(_mm_loadu_si32(src), 0);
    } else if constexpr (sizeof(T) == 2) {
        r = _mm_extract_epi16(_mm_loadu_si16(src), 0);
    } else if constexpr (sizeof(T) == 1) {
        r = *reinterpret_cast<uint8_t const *>(src);
    } else {
        hi_static_no_default();
    }
#else
    auto src_ = reinterpret_cast<uint8_t const *>(src);

    auto i = 8;
    auto tmp = T{};
    while (--i) {
        tmp <<= CHAR_BIT;
        tmp |= *src_++;
    }
    r = byte_swap(tmp);
#endif
}

/** Load data from memory.
 *
 * @param[in,out] r The return value, overwrites the bits in the value at @a offset.
 * @param src The memory location to read the data from.
 * @param size The number of bytes to load.
 * @param offset Byte offset in @a r where the bits are overwritten
 * @note It is undefined behavior to load bytes beyond the boundary of @a r.
 */
template<std::unsigned_integral T>
hi_force_inline void unaligned_load_le(T& r, void const *src, size_t size, size_t offset = 0) noexcept
{
    hi_axiom(offset < sizeof(T));
    hi_axiom(size <= sizeof(T));
    hi_axiom(size + offset <= sizeof(T));

    auto src_ = reinterpret_cast<uint8_t const *>(src);

    hilet first = offset * CHAR_BIT;
    hilet last = (first + size) * CHAR_BIT;

    for (auto i = first; i != last; i += CHAR_BIT) {
        r |= wide_cast<T>(*src_++) << i;
    }
}

} // namespace hi::inline v1

hi_warning_pop();
