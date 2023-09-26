
add_custom_target(hikogui_all_tests)

add_executable(hikogui_tests)
target_link_libraries(hikogui_tests PRIVATE gtest_main hikogui)
target_include_directories(hikogui_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(hikogui_all_tests hikogui_tests)

target_sources(hikogui_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/algorithm/algorithm_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/algorithm/ranges_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/algorithm/strings_tests.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/audio/audio_sample_packer_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/audio/audio_sample_unpacker_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/char_maps/ascii_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/char_maps/char_converter_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/char_maps/cp_1252_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/char_maps/iso_8859_1_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/char_maps/utf_16_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/char_maps/utf_32_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/char_maps/utf_8_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/codec/base_n_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/codec/BON8_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/codec/datum_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/codec/gzip_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/codec/jsonpath_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/codec/JSON_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/codec/SHA2_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/color/color_space_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/concurrency/unfair_mutex_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/concurrency/notifier_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/concurrency/rcu_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/container/lean_vector_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/container/polymorphic_optional_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/container/small_map_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/container/tree_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/coroutine/generator_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/file/file_view_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/font/font_char_map_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/font/font_weight_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/formula/formula_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/matrix3_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/point2_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/point3_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/scale2_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/scale3_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/transform_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/translate2_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/translate3_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/vector2_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/geometry/vector3_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/graphic_path/bezier_curve_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/graphic_path/graphic_path_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/i18n/iso_15924_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/i18n/iso_3166_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/i18n/iso_639_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/i18n/language_tag_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/image/pixmap_span_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/image/pixmap_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/layout/spreadsheet_address_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/numeric/bigint_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/numeric/bound_integer_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/numeric/decimal_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/numeric/interval_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/numeric/int_carry_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/numeric/int_overflow_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/numeric/polynomial_tests.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/numeric/polynomial_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/numeric/safe_int_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/observer/group_ptr_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/observer/shared_state_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/parser/lexer_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/parser/lookahead_iterator_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/path/glob_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/path/URI_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/path/URL_tests.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/random/dither_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/random/seed_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/random/xorshift128p_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/security/sip_hash_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/settings/user_settings_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/simd_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/skeleton/skeleton_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/telemetry/counters_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/telemetry/format_check_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/unicode/grapheme_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/unicode/gstring_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/unicode/markup_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/unicode/ucd_scripts_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/unicode/unicode_bidi_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/unicode/unicode_break_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/unicode/unicode_normalization_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/cast_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/defer_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/enum_metadata_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/exceptions_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/fixed_string_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/float16_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/forward_value_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/math_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/reflection_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/type_traits_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/units_tests.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/widgets/text_widget_tests.cpp
)

show_build_target_properties(hikogui_tests)
#gtest_discover_tests(hikogui_tests DISCOVERY_MODE PRE_TEST)
add_test(NAME hikogui_tests COMMAND hikogui_tests)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64|x86_64")
    if(HOST_IS_X86_64_1)
        add_executable(hikogui_x64v1_tests)
        add_test(NAME hikogui_x64v1_tests COMMAND hikogui_x64v1_tests)
        target_link_libraries(hikogui_x64v1_tests PRIVATE gtest_main hikogui)
        target_include_directories(hikogui_x64v1_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(hikogui_x64v1_tests hikogui_tests_resources)
        add_dependencies(hikogui_all_tests hikogui_x64v1_tests)
        show_build_target_properties(hikogui_x64v1_tests)

        if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GCC")
            target_compile_options(hikogui_x64v1_tests PRIVATE -march=x86-64)
        elseif (MSVC)
            # On MSVC the architecture flags do not exactly correspond with the x86-64 architecture levels.
            # The HI_X86_64_MAX_LEVEL macro limits use for intrinsic usage to the exact level for testing purposes.
            target_compile_options(hikogui_x64v1_tests PRIVATE -DHI_X86_64_MAX_LEVEL=1)
        else()
            message(WARNING "Unknown compiler to generate architecture depended tests.") 
        endif()

        target_sources(hikogui_x64v1_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_f32x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_i32x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_u32x4_tests.cpp
        )
    endif()

    if(HOST_IS_X86_64_2)
        add_executable(hikogui_x64v2_tests)
        add_test(NAME hikogui_x64v2_tests COMMAND hikogui_x64v2_tests)
        target_link_libraries(hikogui_x64v2_tests PRIVATE gtest_main hikogui)
        target_include_directories(hikogui_x64v2_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(hikogui_x64v2_tests hikogui_tests_resources)
        add_dependencies(hikogui_all_tests hikogui_x64v2_tests)
        show_build_target_properties(hikogui_x64v2_tests)

        if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GCC")
            target_compile_options(hikogui_x64v2_tests PRIVATE -march=x86-64-v2)
        elseif (MSVC)
            # On MSVC the architecture flags do not exactly correspond with the x86-64 architecture levels.
            # The HI_X86_64_MAX_LEVEL macro limits use for intrinsic usage to the exact level for testing purposes.
            target_compile_options(hikogui_x64v2_tests PRIVATE -arch:AVX -DHI_X86_64_MAX_LEVEL=2)
        else()
            message(WARNING "Unknown compiler to generate architecture depended tests.") 
        endif()

        target_sources(hikogui_x64v2_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_f32x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_i32x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_u32x4_tests.cpp
        )
    endif()

    if(HOST_IS_X86_64_3)
        add_executable(hikogui_x64v3_tests)
        add_test(NAME hikogui_x64v3_tests COMMAND hikogui_x64v3_tests)
        target_link_libraries(hikogui_x64v3_tests PRIVATE gtest_main hikogui)
        target_include_directories(hikogui_x64v3_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(hikogui_all_tests hikogui_x64v3_tests)
        show_build_target_properties(hikogui_x64v3_tests)

        if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GCC")
            target_compile_options(hikogui_x64v3_tests PRIVATE -march=x86-64-v3)
        elseif (MSVC)
            # On MSVC the architecture flags do not exactly correspond with the x86-64 architecture levels.
            # The HI_X86_64_MAX_LEVEL macro limits use for intrinsic usage to the exact level for testing purposes.
            target_compile_options(hikogui_x64v3_tests PRIVATE -arch:AVX2 -DHI_X86_64_MAX_LEVEL=3)
        else()
            message(WARNING "Unknown compiler to generate architecture depended tests.") 
        endif()

        target_sources(hikogui_x64v3_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_f32x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_f64x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_i32x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_i64x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_u32x4_tests.cpp
        )
    endif()

    if(HOST_IS_X86_64_4)
        add_executable(hikogui_x64v4_tests)
        add_test(NAME hikogui_x64v4_tests COMMAND hikogui_x64v4_tests)
        target_link_libraries(hikogui_x64v4_tests PRIVATE gtest_main hikogui)
        target_include_directories(hikogui_x64v4_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(hikogui_all_tests hikogui_x64v4_tests)
        show_build_target_properties(hikogui_x64v4_tests)

        if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GCC")
            target_compile_options(hikogui_x64v4_tests PRIVATE -march=x86-64-v4)
        elseif (MSVC)
            # On MSVC the architecture flags do not exactly correspond with the x86-64 architecture levels.
            # The HI_X86_64_MAX_LEVEL macro limits use for intrinsic usage to the exact level for testing purposes.
            target_compile_options(hikogui_x64v4_tests PRIVATE -arch:AVX512 -DHI_X86_64_MAX_LEVEL=4)
        else()
            message(WARNING "Unknown compiler to generate architecture depended tests.") 
        endif()

        target_sources(hikogui_x64v4_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_f32x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_f64x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_i32x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_i64x4_tests.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/native_u32x4_tests.cpp
        )
    endif()

else()
    message(WARNING "Unknown CPU to generate architecture depended tests.") 
endif()
