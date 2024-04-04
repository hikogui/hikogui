// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "array_generic.hpp" // export
#ifndef HI_GENERIC
#include "array_intrinsic_f16x4_x86.hpp" // export
#include "array_intrinsic_f32x4_x86.hpp" // export
#include "array_intrinsic_f64x4_x86.hpp" // export
#include "array_intrinsic_f64x2_x86.hpp" // export
#endif
#include "array_intrinsic.hpp" // export
#include "simd_intf.hpp" // export
#if defined(HI_HAS_X86)
#include "cpu_id_x86.hpp" // export
#else
#include "cpu_id_generic.hpp" // export
#endif
#include "half.hpp" // export
#include "half_to_float.hpp" // export
#include "float_to_half.hpp" // export

hi_export_module(hikogui.SIMD);
