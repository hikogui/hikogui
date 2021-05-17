// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <array>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>

namespace tt {

//inline float hadd(__m256 x)
//{
//    return _mm256_cvtss_f32(_mm256_hadd_ps(sum_v));
//}
//
//inline float hmax(__m256 x)
//{
//    auto peak_l = _mm256_extractf128_ps(peak_v, 0);
//    auto peak_r = _mm256_extractf128_ps(peak_v, 1);
//    peak_l = _mm_max_ps(peak_l, peak_r);
//    peak_r = _mm_permute_ps(peak_l, 0b00'00'11'10);
//    peak_l = _mm_max_ps(peak_l, peak_r);
//    peak_r = _mm_permute_ps(peak_l, 0b00'00'00'01);
//    peak_l = _mm_max_ps(peak_l, peak_r);
//    float peak_s = _mm_cvtss_f32(peak_l, 0);
//}

}
