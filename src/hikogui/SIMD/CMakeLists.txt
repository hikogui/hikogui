# Copyright Take Vos 2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    float16_sse4_1.hpp
    int_abgr8_pack.hpp
    numeric_array.hpp
    swizzle_avx.hpp
    unorm_a2bgr10_pack.hpp
    simd_utility.hpp
    simd_f32x4_sse.hpp
    simd_i16x8_sse2.hpp
    simd_i32x4_sse2.hpp
    srgb_abgr8_pack.hpp
    sfloat_rgba16.hpp
    sfloat_rgba32.hpp
    sfloat_rgb32.hpp
    sfloat_rg32.hpp
    snorm_r8.hpp
    sdf_r8.hpp
    uint_abgr8_pack.hpp
)

if(BUILD_TESTING)
    target_sources(hikogui_tests PRIVATE
        numeric_array_tests.cpp
    )

    target_sources(hikogui_x64v1_tests PRIVATE
        simd_f32x4_tests.cpp
    )
    target_sources(hikogui_x64v2_tests PRIVATE
        simd_f32x4_tests.cpp
    )
    target_sources(hikogui_x64v2_5_tests PRIVATE
        simd_f32x4_tests.cpp
    )
    target_sources(hikogui_x64v3_tests PRIVATE
        simd_f32x4_tests.cpp
    )
    target_sources(hikogui_x64v4_tests PRIVATE
        simd_f32x4_tests.cpp
    )
endif()

