
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
    ${HIKOGUI_SOURCE_DIR}/audio/audio_sample_unpacker_tests.cpp
    #${HIKOGUI_SOURCE_DIR}/audio/audio_sample_packer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/random_char.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/ascii_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/char_converter_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/cp_1252_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/iso_8859_1_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_8_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_16_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_32_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/BON8_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/JSON_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/gzip_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/base_n_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/codec/SHA2_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/color/color_space_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/file/file_view_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/file/glob_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/file/URI_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/file/URL_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/font/font_char_map_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/identity_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/matrix_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/point_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/scale_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/transform_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/translate_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/geometry/vector_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_639_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_3166_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_15924_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/language_tag_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap_span_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/layout/spreadsheet_address_tests.cpp
    #${HIKOGUI_SOURCE_DIR}/random/dither_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/random/seed_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/random/xorshift128p_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/SIMD/simd_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_bidi_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_break_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_normalization_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/widgets/text_widget_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/dead_lock_detector_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/rcu_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/cast_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/enum_metadata_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/fixed_string_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/float16_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/math_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/exceptions_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/type_traits_tests.cpp
    $<$<PLATFORM_ID:Darwin>:${HIKOGUI_SOURCE_DIR}/utility/debugger_macos.mm>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/utility/debugger_win32_impl.cpp>
    ${HIKOGUI_SOURCE_DIR}/algorithm_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/bezier_curve_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/bigint_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/bound_integer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/counters_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/datum_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/decimal_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/defer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/format_check_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/forward_value_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/gap_buffer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/generator_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/interval_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/int_carry_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/int_overflow_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/jsonpath_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/lean_vector_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/group_ptr_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/notifier_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/graphic_path_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/packed_int_array_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/polymorphic_optional_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/polynomial_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/ranges_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/reflection_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/safe_int_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/shared_state_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/sip_hash_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/small_map_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/strings_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/tokenizer_tests.cpp
    ${HIKOGUI_SOURCE_DIR}/tree_tests.cpp
)

install(DIRECTORY tests/data/ DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
show_build_target_properties(hikogui_tests)
gtest_discover_tests(hikogui_tests DISCOVERY_MODE PRE_TEST)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64|x86_64")
    add_executable(hikogui_x64v1_tests)
    add_executable(hikogui_x64v2_tests)
    add_executable(hikogui_x64v3_tests)
    add_executable(hikogui_x64v4_tests)

    target_link_libraries(hikogui_x64v1_tests PRIVATE gtest_main hikogui)
    target_link_libraries(hikogui_x64v2_tests PRIVATE gtest_main hikogui)
    target_link_libraries(hikogui_x64v3_tests PRIVATE gtest_main hikogui)
    target_link_libraries(hikogui_x64v4_tests PRIVATE gtest_main hikogui)

    target_include_directories(hikogui_x64v1_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_include_directories(hikogui_x64v2_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_include_directories(hikogui_x64v3_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_include_directories(hikogui_x64v4_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    add_dependencies(hikogui_x64v1_tests hikogui_tests_resources)
    add_dependencies(hikogui_x64v2_tests hikogui_tests_resources)
    add_dependencies(hikogui_x64v3_tests hikogui_tests_resources)
    add_dependencies(hikogui_x64v4_tests hikogui_tests_resources)

    add_dependencies(hikogui_all_tests hikogui_x64v1_tests)
    add_dependencies(hikogui_all_tests hikogui_x64v2_tests)
    add_dependencies(hikogui_all_tests hikogui_x64v3_tests)
    add_dependencies(hikogui_all_tests hikogui_x64v4_tests)

    target_sources(hikogui_x64v1_tests PRIVATE
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_f32x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_i32x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_u32x4_tests.cpp
    )
    target_sources(hikogui_x64v2_tests PRIVATE
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_f32x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_i32x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_u32x4_tests.cpp
    )
    target_sources(hikogui_x64v3_tests PRIVATE
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_f32x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_f64x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_i32x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_i64x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_u32x4_tests.cpp
    )
    target_sources(hikogui_x64v4_tests PRIVATE
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_f32x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_f64x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_i32x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_i64x4_tests.cpp
        ${HIKOGUI_SOURCE_DIR}/SIMD/native_u32x4_tests.cpp
    )
    install(TARGETS hikogui_tests hikogui_x64v1_tests DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
    install(TARGETS hikogui_tests hikogui_x64v2_tests DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
    install(TARGETS hikogui_tests hikogui_x64v3_tests DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
    install(TARGETS hikogui_tests hikogui_x64v4_tests DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)

    show_build_target_properties(hikogui_x64v1_tests)
    show_build_target_properties(hikogui_x64v2_tests)
    show_build_target_properties(hikogui_x64v3_tests)
    show_build_target_properties(hikogui_x64v4_tests)

    if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GCC")
        target_compile_options(hikogui_x64v1_tests PRIVATE -march=x86-64)
        target_compile_options(hikogui_x64v2_tests PRIVATE -march=x86-64-v2)
        target_compile_options(hikogui_x64v3_tests PRIVATE -march=x86-64-v3)
        target_compile_options(hikogui_x64v4_tests PRIVATE -march=x86-64-v4)
    elseif (MSVC)
        # On MSVC the architecture flags do not exactly correspond with the x86-64 architecture levels.
        # The HI_X86_64_MAX_LEVEL macro limits use for intrinsic usage to the exact level for testing purposes.
        target_compile_options(hikogui_x64v1_tests PRIVATE -DHI_X86_64_MAX_LEVEL=1)
        target_compile_options(hikogui_x64v2_tests PRIVATE -arch:AVX -DHI_X86_64_MAX_LEVEL=2)
        target_compile_options(hikogui_x64v3_tests PRIVATE -arch:AVX2 -DHI_X86_64_MAX_LEVEL=3)
        target_compile_options(hikogui_x64v4_tests PRIVATE -arch:AVX512 -DHI_X86_64_MAX_LEVEL=4)
    else()
        message(WARNING "Unknown compiler to generate architecture depended tests.") 
    endif()
     
    # Only execute the tests that will run on the current host.
    if(HOST_IS_X86_64_1)
        gtest_discover_tests(hikogui_x64v1_tests DISCOVERY_MODE PRE_TEST)
    endif()
    if(HOST_IS_X86_64_2)
        gtest_discover_tests(hikogui_x64v2_tests DISCOVERY_MODE PRE_TEST)
    endif()
    if(HOST_IS_X86_64_3)
        gtest_discover_tests(hikogui_x64v3_tests DISCOVERY_MODE PRE_TEST)
    endif()
    if(HOST_IS_X86_64_4)
        gtest_discover_tests(hikogui_x64v4_tests DISCOVERY_MODE PRE_TEST)
    endif()

else()
    message(WARNING "Unknown CPU to generate architecture depended tests.") 
endif()
