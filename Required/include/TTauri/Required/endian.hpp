// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/os_detect.hpp"
#include "TTauri/Required/memory.hpp"

#if COMPILER == CC_MSVC
#include <stdlib.h>
#endif

namespace TTauri {

enum class Endian {
    Little,
    Big,
    Native,
};

#if PROCESSOR == CPU_X64 || PROCESSOR == CPU_ARM
constexpr Endian endian = Endian::Little;
#else
#error "Endian not configured for this processor."
#endif

template<typename T>
force_inline std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,T> 
byte_swap(T x) noexcept
{
    if constexpr (compiler == Compiler::clang || compiler == Compiler::gcc) {
        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
            return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
            return static_cast<T>(__builtin_bswap16(static_cast<uint16_t>(x)));
        } else {
            no_default;
        }

    } else if constexpr (compiler == Compiler::MSVC) {
        if constexpr (sizeof(T) == sizeof(uint64_t)) {
            return static_cast<T>(_byteswap_uint64(static_cast<uint64_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(unsigned long)) {
            return static_cast<T>(_byteswap_ulong(static_cast<unsigned long>(x)));
        } else if constexpr (sizeof(T) == sizeof(unsigned short)) {
            return static_cast<T>(_byteswap_ushort(static_cast<unsigned short>(x)));
        } else {
            no_default;
        }

    } else {
        no_default;
    }
}

template<typename T>
force_inline std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>,T> 
byte_swap(T x) noexcept
{
    return static_cast<T>(byte_swap(static_cast<std::make_unsigned_t<T>>(x)));
}

template<typename T>
force_inline std::enable_if_t<std::is_floating_point_v<T>,T> 
byte_swap(T x) noexcept
{
    if constexpr (std::is_same_v<T, float>) {
        auto utmp = bit_cast<uint32_t>(x);
        utmp = byte_swap(utmp);
        return bit_cast<float>(x);
    } else if constexpr (std::is_same_v<T, double>) {
        auto utmp = bit_cast<uint64_t>(x);
        utmp = byte_swap(utmp);
        return bit_cast<double>(x);
    } else {
        no_default;
    }
}

template<typename T>
force_inline std::enable_if_t<std::is_integral_v<T>,T> 
little_to_native(T x)
{
    if constexpr (endian == Endian::Little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T>
force_inline std::enable_if_t<std::is_integral_v<T>,T> 
big_to_native(T x)
{
    if constexpr (endian == Endian::Big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T>
force_inline std::enable_if_t<std::is_integral_v<T>,T> 
native_to_little(T x)
{
    if constexpr (endian == Endian::Little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T>
force_inline std::enable_if_t<std::is_integral_v<T>,T> 
native_to_big(T x)
{
    if constexpr (endian == Endian::Big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T,Endian E,size_t A=alignof(T)>
struct endian_buf_t {
    alignas(A) std::byte _value[sizeof(T)];

    force_inline T value() const noexcept {
        T aligned_value;
        std::memcpy(&aligned_value, &_value[0], sizeof(T));

        if constexpr (E == endian || E == Endian::Native) {
            return aligned_value;
        } else {
            return byte_swap(aligned_value);
        }
    }

    force_inline endian_buf_t &operator=(T x) noexcept {
        T aligned_value;
        if constexpr (E == endian || E == Endian::Native) {
            aligned_value = x;
        } else {
            aligned_value = byte_swap(x);
        }
        std::memcpy(&_value[0], &aligned_value, sizeof(T));
        return *this;
    }
};

using big_uint64_buf_t = endian_buf_t<uint64_t,Endian::Big,1>;
using big_uint32_buf_t = endian_buf_t<uint32_t,Endian::Big,1>;
using big_uint16_buf_t = endian_buf_t<uint16_t,Endian::Big,1>;
using big_int64_buf_t = endian_buf_t<int64_t,Endian::Big,1>;
using big_int32_buf_t = endian_buf_t<int32_t,Endian::Big,1>;
using big_int16_buf_t = endian_buf_t<int16_t,Endian::Big,1>;
using little_uint64_buf_t = endian_buf_t<uint64_t,Endian::Little,1>;
using little_uint32_buf_t = endian_buf_t<uint32_t,Endian::Little,1>;
using little_uint16_buf_t = endian_buf_t<uint16_t,Endian::Little,1>;
using little_int64_buf_t = endian_buf_t<int64_t,Endian::Little,1>;
using little_int32_buf_t = endian_buf_t<int32_t,Endian::Little,1>;
using little_int16_buf_t = endian_buf_t<int16_t,Endian::Little,1>;
using native_uint64_buf_t = endian_buf_t<uint64_t,Endian::Native,1>;
using native_uint32_buf_t = endian_buf_t<uint32_t,Endian::Native,1>;
using native_uint16_buf_t = endian_buf_t<uint16_t,Endian::Native,1>;
using native_int64_buf_t = endian_buf_t<int64_t,Endian::Native,1>;
using native_int32_buf_t = endian_buf_t<int32_t,Endian::Native,1>;
using native_int16_buf_t = endian_buf_t<int16_t,Endian::Native,1>;

using big_uint64_buf_at = endian_buf_t<uint64_t,Endian::Big>;
using big_uint32_buf_at = endian_buf_t<uint32_t,Endian::Big>;
using big_uint16_buf_at = endian_buf_t<uint16_t,Endian::Big>;
using big_int64_buf_at = endian_buf_t<int64_t,Endian::Big>;
using big_int32_buf_at = endian_buf_t<int32_t,Endian::Big>;
using big_int16_buf_at = endian_buf_t<int16_t,Endian::Big>;
using little_uint64_buf_at = endian_buf_t<uint64_t,Endian::Little>;
using little_uint32_buf_at = endian_buf_t<uint32_t,Endian::Little>;
using little_uint16_buf_at = endian_buf_t<uint16_t,Endian::Little>;
using little_int64_buf_at = endian_buf_t<int64_t,Endian::Little>;
using little_int32_buf_at = endian_buf_t<int32_t,Endian::Little>;
using little_int16_buf_at = endian_buf_t<int16_t,Endian::Little>;
using native_uint64_buf_at = endian_buf_t<uint64_t,Endian::Native>;
using native_uint32_buf_at = endian_buf_t<uint32_t,Endian::Native>;
using native_uint16_buf_at = endian_buf_t<uint16_t,Endian::Native>;
using native_int64_buf_at = endian_buf_t<int64_t,Endian::Native>;
using native_int32_buf_at = endian_buf_t<int32_t,Endian::Native>;
using native_int16_buf_at = endian_buf_t<int16_t,Endian::Native>;


}

