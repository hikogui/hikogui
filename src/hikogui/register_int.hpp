
/** @file
 */

#pragma once

#include "architecture.hpp"
#include "bigint.hpp"
#include <cstdint>

/** @file
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
 * @typedef hi::register_signed_int
 * A signed integer with the maximum size of a scaler register.
 *
 * @typedef hi::register_unsigned_int
 * A unsigned integer with the maximum size of a scaler register.
 *
 * @typedef hi::register_signed_long
 * A signed integer twice the maximum size of a scaler register.
 *
 * @typedef hi::register_unsigned_long
 * A unsigned integer twice the maximum size of a scaler register.
 */

namespace hi::inline v1 {

#if HI_PROCESSOR == HI_CPU_X64
using register_int = int64_t;
using register_signed_int = int64_t;
using register_unsigned_int = uint64_t;

#if HI_COMPILER == HI_CC_CLANG || HI_COMPILER == HI_CC_GCC
using register_long = __int128_t;
using register_signed_long = __int128_t;
using register_unsigned_long = unsigned __int128_t;

#else
using register_long = bigint<register_unsigned_int, 2, true>;
using register_signed_long = bigint<register_unsigned_int, 2, true>;
using register_unsigned_long = bigint<register_unsigned_int, 2, false>;
#endif

#else
#error "register_int missing implementation"
#endif

} // namespace hi::inline v1
