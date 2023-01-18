
target_sources(hikogui PUBLIC FILE_SET hikogui_include_files TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/" FILES
    ${HIKOGUI_SOURCE_DIR}/char_maps/ascii.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/char_converter.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/cp_1252.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/iso_8859_1.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/module.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/to_string.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_8.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_16.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_32.hpp
    ${HIKOGUI_SOURCE_DIR}/color/color.hpp
    ${HIKOGUI_SOURCE_DIR}/color/color_space.hpp
    ${HIKOGUI_SOURCE_DIR}/color/module.hpp
    ${HIKOGUI_SOURCE_DIR}/color/quad_color.hpp
    ${HIKOGUI_SOURCE_DIR}/color/Rec2020.hpp
    ${HIKOGUI_SOURCE_DIR}/color/Rec2100.hpp
    ${HIKOGUI_SOURCE_DIR}/color/semantic_color.hpp
    ${HIKOGUI_SOURCE_DIR}/color/sRGB.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/alignment.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/axis.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/axis_aligned_rectangle.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/circle.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/corner_radii.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/extent.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/identity.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/line_end_cap.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/line_join_style.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/line_segment.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/lookat.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/margins.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/matrix.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/module.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/perspective.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/point.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/quad.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/rectangle.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/rotate.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/scale.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/transform.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/translate.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/vector.hpp
    ${HIKOGUI_SOURCE_DIR}/image/module.hpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap.hpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap_span.hpp
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
    ${HIKOGUI_SOURCE_DIR}/utility/architecture.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/assert.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/cast.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/charconv.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/check.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/compare.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/concepts.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/dead_lock_detector.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/debugger.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/debugger.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/enum_metadata.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/endian.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/exception.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/fixed_string.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/float16.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/hash.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/math.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/memory.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/module.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/numbers.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/subsystem.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/test.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/thread.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/type_traits.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/unfair_mutex.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/unfair_recursive_mutex.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/utility.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/utility/win32_headers.hpp>
    ${HIKOGUI_SOURCE_DIR}/module.hpp
    ${HIKOGUI_SOURCE_DIR}/terminate.hpp
)
