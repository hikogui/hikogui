// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/os_detect.hpp"

#if COMPILER == CC_MSVC
#include <stdlib.h>
#endif

namespace TTauri {

template<typename T>
std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,T> 
byte_swap(T x) noexcept
{
    if constexpr (compiler == Compiler::clang || compiler == Compiler::gcc) {
        if constexpr (sizeof(T) == sizeof(uint64_t) {
            return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(uint32_t) {
            return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(uint16_t) {
            return static_cast<T>(__builtin_bswap16(static_cast<uint16>(x)));
        } else {
            no_default;
        }

    } else if constexpr (compiler == Compiler::MSVC) {
        if constexpr (sizeof(T) == sizeof(uint64_t) {
            return static_cast<T>(_byteswap_uint64(static_cast<uint64_t>(x)));
        } else if constexpr (sizeof(T) == sizeof(unsigned long) {
            return static_cast<T>(_byteswap_ulong(static_cast<unsigned long>(x)));
        } else if constexpr (sizeof(T) == sizeof(unsigned short) {
            return static_cast<T>(_byteswap_ushort(static_cast<unsigned short>(x)));
        } else {
            no_default;
        }

    } else {
        no_default;
    }
}

template<typename T>
std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>,T> 
byte_swap(T x) noexcept
{
    return static_cast<T>(byte_swap(static_cast<std::make_unsigned<T>(x)));
}


template<typename T>
std::enable_if_t<std::is_integral_v<T>,T> 
little_to_native(T x)
{
    if constexpr (endian == Endian::Little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>,T> 
big_to_native(T x)
{
    if constexpr (endian == Endian::Big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>,T> 
native_to_little(T x)
{
    if constexpr (endian == Endian::Little) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>,T> 
native_to_big(T x)
{
    if constexpr (endian == Endian::Big) {
        return x;
    } else {
        return byte_swap(x);
    }
}

template<typename T,size_t A=sizeof(T),Endian E>
struct endian_buf_t {
    alignas(A);
    T _value;

    T value() {
        if constexpr (E == endian) {
            return _value;
        } else {
            return byte_swap(_value);
        }
    }
};



}

