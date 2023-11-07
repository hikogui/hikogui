// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;

//#include "float16_sse4_1.hpp" // export

export module hikogui_SIMD;
export import : intf;
export import : native_f16x8_sse2;
export import : native_f32x4_sse;
export import : native_f64x4_avx;
export import : native_i16x8_sse2;
export import : native_i32x4_sse2;
export import : native_i64x4_avx2;
export import : native_i8x16_sse2;
export import : native_simd_conversions_x86;
export import : native_simd_utility;
export import : native_u32x4_sse2;
