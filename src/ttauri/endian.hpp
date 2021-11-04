// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include "memory.hpp"
#include "assert.hpp"

#if TT_COMPILER == TT_CC_MSVC
#include <stdlib.h>
#endif
#include <bit>

namespace tt::inline v1 {

template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
[[nodiscard]] T byte_swap(T x) noexcept
{
#if TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
            return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
            return static_cast<T>(__builtin_bswap16(static_cast<uint16_t>(x)));
        } else {
            tt_no_default();
        }
#elif TT_COMPILER == TT_CC_MSVC
        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return static_cast<T>(_byteswap_uint64(static_cast<uint64_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(unsigned long)) {
            return static_cast<T>(_byteswap_ulong(static_cast<unsigned long>(x)));
        } else if constexpr (sizeof(T) == sizeof(unsigned short)) {
            return static_cast<T>(_byteswap_ushort(static_cast<unsigned short>(x)));
        } else {
            tt_no_default();
        }
#else
#error "Byteswap not implemented for this compiler."
#endif
}

template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>,int> = 0>
[[nodiscard]] T byte_swap(T x) noexcept
{
    return static_cast<T>(byte_swap(static_cast<std::make_unsigned_t<T>>(x)));
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>,T> = 0>
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
        tt_no_default();
    }
}

template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
[[nodiscard]] T little_to_native(T x)
{
    if constexpr (std::endian::native == std::endian::little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
[[nodiscard]] T big_to_native(T x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
[[nodiscard]] T native_to_little(T x)
{
    if constexpr (std::endian::native == std::endian::little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
[[nodiscard]] T native_to_big(T x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T,std::endian E,size_t A=alignof(T)>
struct endian_buf_t {
    alignas(A) std::byte _value[sizeof(T)];

    [[nodiscard]] T value() const noexcept {
        T aligned_value;
        std::memcpy(&aligned_value, &_value[0], sizeof(T));

        if constexpr (E == std::endian::native) {
            return aligned_value;
        } else {
            return byte_swap(aligned_value);
        }
    }

    operator T() const noexcept {
        T aligned_value;
        std::memcpy(&aligned_value, &_value[0], sizeof(T));

        if constexpr (E == std::endian::native) {
            return aligned_value;
        } else {
            return byte_swap(aligned_value);
        }
    }

    endian_buf_t &operator=(T x) noexcept {
        T aligned_value;
        if constexpr (E == std::endian::native) {
            aligned_value = x;
        } else {
            aligned_value = byte_swap(x);
        }
        std::memcpy(&_value[0], &aligned_value, sizeof(T));
        return *this;
    }
};

using big_uint64_buf_t = endian_buf_t<uint64_t,std::endian::big,1>;
using big_uint32_buf_t = endian_buf_t<uint32_t,std::endian::big,1>;
using big_uint16_buf_t = endian_buf_t<uint16_t,std::endian::big,1>;
using big_int64_buf_t = endian_buf_t<int64_t,std::endian::big,1>;
using big_int32_buf_t = endian_buf_t<int32_t,std::endian::big,1>;
using big_int16_buf_t = endian_buf_t<int16_t,std::endian::big,1>;
using little_uint64_buf_t = endian_buf_t<uint64_t,std::endian::little,1>;
using little_uint32_buf_t = endian_buf_t<uint32_t,std::endian::little,1>;
using little_uint16_buf_t = endian_buf_t<uint16_t,std::endian::little,1>;
using little_int64_buf_t = endian_buf_t<int64_t,std::endian::little,1>;
using little_int32_buf_t = endian_buf_t<int32_t,std::endian::little,1>;
using little_int16_buf_t = endian_buf_t<int16_t,std::endian::little,1>;
using native_uint64_buf_t = endian_buf_t<uint64_t,std::endian::native,1>;
using native_uint32_buf_t = endian_buf_t<uint32_t,std::endian::native,1>;
using native_uint16_buf_t = endian_buf_t<uint16_t,std::endian::native,1>;
using native_int64_buf_t = endian_buf_t<int64_t,std::endian::native,1>;
using native_int32_buf_t = endian_buf_t<int32_t,std::endian::native,1>;
using native_int16_buf_t = endian_buf_t<int16_t,std::endian::native,1>;

using big_uint64_buf_at = endian_buf_t<uint64_t,std::endian::big>;
using big_uint32_buf_at = endian_buf_t<uint32_t,std::endian::big>;
using big_uint16_buf_at = endian_buf_t<uint16_t,std::endian::big>;
using big_int64_buf_at = endian_buf_t<int64_t,std::endian::big>;
using big_int32_buf_at = endian_buf_t<int32_t,std::endian::big>;
using big_int16_buf_at = endian_buf_t<int16_t,std::endian::big>;
using little_uint64_buf_at = endian_buf_t<uint64_t,std::endian::little>;
using little_uint32_buf_at = endian_buf_t<uint32_t,std::endian::little>;
using little_uint16_buf_at = endian_buf_t<uint16_t,std::endian::little>;
using little_int64_buf_at = endian_buf_t<int64_t,std::endian::little>;
using little_int32_buf_at = endian_buf_t<int32_t,std::endian::little>;
using little_int16_buf_at = endian_buf_t<int16_t,std::endian::little>;
using native_uint64_buf_at = endian_buf_t<uint64_t,std::endian::native>;
using native_uint32_buf_at = endian_buf_t<uint32_t,std::endian::native>;
using native_uint16_buf_at = endian_buf_t<uint16_t,std::endian::native>;
using native_int64_buf_at = endian_buf_t<int64_t,std::endian::native>;
using native_int32_buf_at = endian_buf_t<int32_t,std::endian::native>;
using native_int16_buf_at = endian_buf_t<int16_t,std::endian::native>;


}

