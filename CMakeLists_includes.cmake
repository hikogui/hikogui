
target_sources(hikogui PUBLIC FILE_SET hikogui_include_files TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/" FILES
    ${HIKOGUI_SOURCE_DIR}/image/module.hpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap.hpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap_view.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sdf_r8.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sfloat_rg32.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sfloat_rgb32.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sfloat_rgba16.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sfloat_rgba32.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sint_abgr8_pack.hpp
    ${HIKOGUI_SOURCE_DIR}/image/snorm_r8.hpp
    ${HIKOGUI_SOURCE_DIR}/image/srgb_abgr8_pack.hpp
    ${HIKOGUI_SOURCE_DIR}/image/uint_abgr8_pack.hpp
    ${HIKOGUI_SOURCE_DIR}/image/unorm_a2bgr10_pack.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/float16_sse4_1.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/module.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_f16x8_sse2.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_f32x4_sse.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_f64x4_avx.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_i16x8_sse2.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_i32x4_sse2.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_i64x4_avx2.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_i8x16_sse2.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_simd_conversions_x86.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_simd_utility.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/native_u32x4_sse2.hpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/simd.hpp

)
