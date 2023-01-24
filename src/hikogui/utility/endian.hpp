// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "memory.hpp"

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
[[nodiscard]] constexpr T byte_swap(T x) noexcept
{
    if (not std::is_constant_evaluated()) {
#if HI_COMPILER == HI_CC_CLANG || HI_COMPILER == HI_CC_GCC
        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
            return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
            return static_cast<T>(__builtin_bswap16(static_cast<uint16_t>(x)));
        }
#elif HI_COMPILER == HI_CC_MSVC
        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return static_cast<T>(_byteswap_uint64(static_cast<uint64_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(unsigned long)) {
            return static_cast<T>(_byteswap_ulong(static_cast<unsigned long>(x)));
        } else if constexpr (sizeof(T) == sizeof(unsigned short)) {
            return static_cast<T>(_byteswap_ushort(static_cast<unsigned short>(x)));
        }
#endif
    }

    if constexpr (sizeof(T) == 1) {
        return x;
    } else {
        auto r = T{};
        for (auto i = 0_uz; i != sizeof(T); ++i) {
            r <<= 8;
            r |= static_cast<uint8_t>(x);
            x >>= 8;
        }
        return r;
    }
}

template<std::signed_integral T>
[[nodiscard]] constexpr T byte_swap(T x) noexcept
{
    return static_cast<T>(byte_swap(static_cast<std::make_unsigned_t<T>>(x)));
}

template<std::floating_point T>
[[nodiscard]] constexpr T byte_swap(T x) noexcept
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
        hi_static_no_default();
    }
}

template<std::integral T>
[[nodiscard]] constexpr T little_to_native(T x)
{
    if constexpr (std::endian::native == std::endian::little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<std::integral T>
[[nodiscard]] constexpr T big_to_native(T x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<std::integral T>
[[nodiscard]] constexpr T native_to_little(T x)
{
    if constexpr (std::endian::native == std::endian::little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<std::integral T>
[[nodiscard]] constexpr T native_to_big(T x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<numeric T, same_as_byte B>
[[nodiscard]] constexpr T load_le(B const *src) noexcept
{
    return little_to_native(load<T>(src));
}

template<numeric T>
[[nodiscard]] inline T load_le(void const *src) noexcept
{
    return load_le<T>(reinterpret_cast<std::byte const *>(src));
}


template<numeric T, same_as_byte B>
[[nodiscard]] constexpr T load_be(B const *src) noexcept
{
    return big_to_native(load<T>(src));
}

template<numeric T>
[[nodiscard]] inline T load_be(void const *src) noexcept
{
    return load_be<T>(reinterpret_cast<std::byte const *>(src));
}

template<typename T, std::endian E, std::size_t A = alignof(T)>
struct endian_buf_t {
    using value_type = T;
    constexpr static std::endian endian = E;
    constexpr static std::size_t alignment = A;

    alignas(A) std::byte _value[sizeof(T)];

    [[nodiscard]] constexpr value_type operator*() const noexcept
    {
        auto x = load<value_type>(_value);
        if constexpr (E != std::endian::native) {
            x = byte_swap(x);
        }
        return x;
    }

    constexpr endian_buf_t& operator=(value_type x) noexcept
    {
        if constexpr (E != std::endian::native) {
            x = byte_swap(x);
        }

        store(x, _value);
        return *this;
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

} // namespace hi::inline v1

hi_warning_pop();
