
target_sources(ttauri PRIVATE
    color_space.hpp
    sRGB.hpp
    Rec2020.hpp
    Rec2100.hpp
    unorm_a2bgr10_pack.hpp
    srgb_abgr8_pack.hpp
    sfloat_rgba16.hpp
    sfloat_rgba32.hpp
    sfloat_rgb32.hpp
    sfloat_rg32.hpp
    snorm_r8.hpp
    sdf_r8.hpp
)

target_sources(ttauri_tests PRIVATE
    color_space_tests.cpp
)
