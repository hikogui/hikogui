// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_intrinsic.hpp"
#include "../macros.hpp"
#include <cstddef>
#include <array>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>

hi_export_module(hikogui.simd.simd_intrinsic);

namespace hi { inline namespace v1 {

template<typename T, size_t N>
struct simd_intrinsic;

}}

