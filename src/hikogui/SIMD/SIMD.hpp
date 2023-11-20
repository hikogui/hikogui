// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_intf.hpp" // export
//#include "half_sse4_1.hpp" // export
#include "native_f16x8_sse2.hpp" // export
#include "native_f32x4_sse.hpp" // export
#include "native_f64x4_avx.hpp" // export
#include "native_i16x8_sse2.hpp" // export
#include "native_i32x4_sse2.hpp" // export
#include "native_i64x4_avx2.hpp" // export
#include "native_i8x16_sse2.hpp" // export
#include "native_simd_conversions_x86.hpp" // export
#include "native_simd_utility.hpp" // export
#include "native_u32x4_sse2.hpp" // export

hi_export_module(hikogui.SIMD);
