// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "array_generic.hpp" // export
#ifndef HI_GENERIC
#include "array_intrinsic_f32x4_x86.hpp" // export
#endif
#include "array_intrinsic.hpp" // export
#include "simd_intf.hpp" // export

hi_export_module(hikogui.SIMD);
