
target_sources(ttauri PRIVATE
    raw_numeric_array.hpp
    numeric_array.hpp
    f32x4_x64v2.hpp
    f32x8_x64v25.hpp
    i8x16_x64v2.hpp
    unorm_a2bgr10_pack.hpp
    srgb_abgr8_pack.hpp
    sfloat_rgba16.hpp
    sfloat_rgba32.hpp
    sfloat_rgb32.hpp
    sfloat_rg32.hpp
    snorm_r8.hpp
    sdf_r8.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        numeric_array_tests.cpp
    )
endif()

