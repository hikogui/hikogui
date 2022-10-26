// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"

#if (HI_COMPILER == HI_CC_GCC || HI_COMPILER == HI_CC_CLANG) && (HI_PROCESSOR == HI_CPU_X64 || HI_PROCESSOR == HI_CPU_ARM64)
#define HI_HAS_INT128 1
#endif

#if !defined(HI_HAS_INT128)
#include "bigint.hpp"
#endif
#include <cstdint>

/** @file stdint.hpp Extra integer definitions.
 *
 * This file defines types that match the CPU's native register.
 * `hi::register_signed_long` and `hi::register_unsigned_long` are double the
 * size of a native register; the double size integers are often the results of
 * a single CPU instruction.
 *
 * For example the x86-64 instruction MUL can multiply two 64 bit integers together,
 * the result is placed in two 64 bit register. DIV can divide a 128 bit integer in
 * two registers by a 64 bit integer, with a 64 bit integer result.
 *
 * @typedef hi::longreg_t
 * A signed integer twice the maximum size of a scaler register.
 *
 * @typedef hi::ulongreg_t
 * A unsigned integer twice the maximum size of a scaler register.
 */

namespace hi::inline v1 {

#if defined(HI_HAS_INT128)
using int128_t = __int128_t;
using uint128_t = unsigned __int128_t;
#else
using int128_t = bigint<uintreg_t, 128 / (sizeof(uintreg_t) * CHAR_BIT), true>;
using uint128_t = bigint<uintreg_t, 128 / (sizeof(uintreg_t) * CHAR_BIT), false>;
#endif

#if HI_PROCESSOR == HI_CPU_X86
using longreg_t = int64_t;
using ulongreg_t = uint64_t;
#elif HI_PROCESSOR == HI_CPU_X64
using longreg_t = int128_t;
using ulongreg_t = uint128_t;
#elif HI_PROCESS = HI_CPU_ARM
using longreg_t = int64_t;
using ulongreg_t = uint64_t;
#elif HI_PROCESS = HI_CPU_ARM64
using longreg_t = int128_t;
using ulongreg_t = uint128_t;
#else
#error "register_int missing implementation"
#endif

} // namespace hi::inline v1
