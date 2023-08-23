
add_custom_target(hikogui_all_tests)

add_custom_target(hikogui_tests_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/data ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(hikogui_tests)
target_link_libraries(hikogui_tests PRIVATE gtest_main hikogui)
target_include_directories(hikogui_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(hikogui_tests hikogui_tests_resources)
add_dependencies(hikogui_all_tests hikogui_tests)

target_sources(hikogui_tests PRIVATE
    ${HIKOGUI_SOURCE_DIR}/algorithm/algorithm_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/algorithm/ranges_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/algorithm/strings_tests.cpp
    #${HIKOGUI_SOURCE_DIR}/audio/audio_sample_packer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_sample_unpacker_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/ascii_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/char_converter_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/cp_1252_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/iso_8859_1_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_16_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_32_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_8_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/base_n_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/BON8_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/datum_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/gzip_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/jsonpath_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/JSON_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/SHA2_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/color/color_space_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/unfair_mutex_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/notifier_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/rcu_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/container/gap_buffer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/container/lean_vector_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/container/packed_int_array_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/container/polymorphic_optional_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/container/small_map_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/container/tree_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/coroutine/generator_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/file/file_view_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/font/font_char_map_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/matrix3_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/point2_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/point3_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/scale2_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/scale3_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/transform_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/translate2_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/translate3_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/vector2_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/vector3_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/graphic_path/bezier_curve_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/graphic_path/graphic_path_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_15924_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_3166_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_639_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/language_tag_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap_span_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/layout/spreadsheet_address_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/numeric/bigint_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/numeric/bound_integer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/numeric/decimal_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/numeric/interval_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/numeric/int_carry_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/numeric/int_overflow_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/numeric/polynomial_tests.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/polynomial_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/numeric/safe_int_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/observer/group_ptr_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/observer/shared_state_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/parser/lexer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/parser/lookahead_iterator_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/path/glob_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/path/URI_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/path/URL_tests.cpp
    #${HIKOGUI_SOURCE_DIR}/random/dither_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/random/seed_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/random/xorshift128p_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/security/sip_hash_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/settings/user_settings_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/simd_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/telemetry/counters_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/telemetry/format_check_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/grapheme_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/gstring_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/markup_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_scripts_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_bidi_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_break_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_normalization_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/cast_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/defer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/enum_metadata_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/exceptions_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/fixed_string_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/float16_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/forward_value_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/math_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/reflection_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/type_traits_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/units_tests.cpp
    #${HIKOGUI_SOURCE_DIR}/widgets/text_widget_tests.cpp
)

install(DIRECTORY tests/data/ DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
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
        install(TARGETS hikogui_tests hikogui_x64v1_tests DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
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
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_f32x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_i32x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_u32x4_tests.cpp
        )
    endif()

    if(HOST_IS_X86_64_2)
        add_executable(hikogui_x64v2_tests)
        add_test(NAME hikogui_x64v2_tests COMMAND hikogui_x64v2_tests)
        target_link_libraries(hikogui_x64v2_tests PRIVATE gtest_main hikogui)
        target_include_directories(hikogui_x64v2_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(hikogui_x64v2_tests hikogui_tests_resources)
        add_dependencies(hikogui_all_tests hikogui_x64v2_tests)
        install(TARGETS hikogui_tests hikogui_x64v2_tests DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
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
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_f32x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_i32x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_u32x4_tests.cpp
        )
    endif()

    if(HOST_IS_X86_64_3)
        add_executable(hikogui_x64v3_tests)
        add_test(NAME hikogui_x64v3_tests COMMAND hikogui_x64v3_tests)
        target_link_libraries(hikogui_x64v3_tests PRIVATE gtest_main hikogui)
        target_include_directories(hikogui_x64v3_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(hikogui_x64v3_tests hikogui_tests_resources)
        add_dependencies(hikogui_all_tests hikogui_x64v3_tests)
        install(TARGETS hikogui_tests hikogui_x64v3_tests DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
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
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_f32x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_f64x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_i32x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_i64x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_u32x4_tests.cpp
        )
    endif()

    if(HOST_IS_X86_64_4)
        add_executable(hikogui_x64v4_tests)
        add_test(NAME hikogui_x64v4_tests COMMAND hikogui_x64v4_tests)
        target_link_libraries(hikogui_x64v4_tests PRIVATE gtest_main hikogui)
        target_include_directories(hikogui_x64v4_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
        add_dependencies(hikogui_x64v4_tests hikogui_tests_resources)
        add_dependencies(hikogui_all_tests hikogui_x64v4_tests)
        install(TARGETS hikogui_tests hikogui_x64v4_tests DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
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
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_f32x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_f64x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_i32x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_i64x4_tests.cpp
            ${HIKOGUI_SOURCE_DIR}/SIMD/native_u32x4_tests.cpp
        )
    endif()

else()
    message(WARNING "Unknown CPU to generate architecture depended tests.") 
endif()
