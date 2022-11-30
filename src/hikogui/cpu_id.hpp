// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include "cast.hpp"
#include <array>

#if HI_COMPILER == HI_CC_MSVC
#include <intrin.h>
#elif HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG
#include <cpuid.h>
#else
#error "Unsuported compiler for x64 cpu_id"
#endif

namespace hi {
inline namespace v1 {

#if HI_COMPILER == HI_CC_MSVC

inline void _cpu_id(uint32_t leaf_id, uint32_t index, uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d) noexcept
{
    std::array<int, 4> info;
    __cpuindex(info.data(), char_cast<int>(cpu_id_leaf), char_cast<int>(index));

    a = char_cast<uint32_t>get<0>(info);
    b = char_cast<uint32_t>get<1>(info);
    c = char_cast<uint32_t>get<2>(info);
    d = char_cast<uint32_t>get<3>(info);
}

#elif HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG

inline void _cpu_id(uint32_t leaf_id, uint32_t index, uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d) noexcept
{
    __cpuid_count(leaf_id, index, a, b, c, d);
}

#else
#error "Unsuported compiler for x64 cpu_id"
#endif

inline bool cpu_id(uint32_t leaf_id, uint32_t index, uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d) noexcept
{
    _cpu_id(leaf_id & 0x8000'0000, 0, a, b, c, d);
    if (a == 0 or leaf_id > a) {
        return false;
    }
    _cpu_id(leaf_id, index, a, b, c, d);
    return true;
}

}}

