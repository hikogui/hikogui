
#set(STL_MODULE_DIR "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Tools/MSVC/14.37.32820/modules")
#
#target_sources(hikogui PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${STL_MODULE_DIR}/" FILES
#    ${STL_MODULE_DIR}/std.ixx
#    ${STL_MODULE_DIR}/std.compat.ixx)


if(VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    # vcpkg does not allow absolute paths anywhere in the install directory.
    # These directories are normally used to execute files in their build
    # directory; which does not happen on a vcpkg install.
    set(LIBRARY_SOURCE_DIR "vcpkg_no_source_dir")
    set(LIBRARY_BUILD_DIR "vcpkg_no_build_dir")
else()
    set(LIBRARY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(LIBRARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/gstring.ixx PROPERTIES COMPILE_FLAGS "-d1module:enableLoggingfile.json")

target_sources(hikogui PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/mod/" FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/algorithm/algorithm.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/algorithm/animator.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/algorithm/algorithm_misc.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/algorithm/ranges.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/algorithm/recursive_iterator.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/algorithm/strings.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/ascii.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/char_converter.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/cp_1252.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/iso_8859_1.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/char_maps.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/random_char.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/to_string.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/utf_16.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/utf_32.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/char_maps/utf_8.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/atomic.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/callback.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/callback_flags.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/global_state.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/id_factory.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/concurrency.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/rcu.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/subsystem.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/thread.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/thread_intf.ixx
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/thread_win32_impl.ixx>
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/unfair_mutex.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/unfair_mutex_intf.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/unfair_mutex_impl.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/unfair_recursive_mutex.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/concurrency/wfree_idle_count.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/console/console.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/console/dialog.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/console/dialog_intf.ixx
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/console/dialog_win32_impl.ixx>
    $<$<PLATFORM_ID:MacOS>:${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/console/dialog_macos_impl.ixx>
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/console/print.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/console/print_intf.ixx
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/console/print_win32_impl.ixx>
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/byte_string.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/function_fifo.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/functional.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/lean_vector.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/container.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/polymorphic_optional.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/small_map.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/small_vector.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/stable_set.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/stack.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/tree.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/undo_stack.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/vector_span.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/void_span.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/wfree_fifo.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/container/wfree_unordered_map.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/coroutine/awaitable.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/coroutine/generator.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/coroutine/coroutine.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/coroutine/task.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/iso_15924.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/iso_15924_intf.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/iso_15924_impl.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/iso_3166.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/iso_3166_intf.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/iso_3166_impl.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/iso_639.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/language_tag.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/language_tag_intf.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/language_tag_impl.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/i18n/i18n.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/bigint.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/bound_integer.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/decimal.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/fixed.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/interval.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/int_carry.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/int_overflow.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/numeric.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/polynomial.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/numeric/safe_int.ixx
    #${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/float16_sse4_1.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/SIMD.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_f16x8_sse2.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_f32x4_sse.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_f64x4_avx.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_i16x8_sse2.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_i32x4_sse2.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_i64x4_avx2.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_i8x16_sse2.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_simd_conversions_x86.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_simd_utility.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/native_u32x4_sse2.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/SIMD/simd_intf.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/telemetry/counters.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/telemetry/delayed_format.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/telemetry/format_check.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/telemetry/log.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/telemetry/telemetry.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/telemetry/trace.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/time/chrono.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/time/time.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/time/time_stamp_count.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/time/time_stamp_utc.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/grapheme.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/gstring.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/markup.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/phrasing.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_bidi_classes.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_bidi_mirroring_glyphs.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_bidi_paired_bracket_types.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_canonical_combining_classes.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_compositions.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_decompositions.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_east_asian_widths.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_general_categories.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_grapheme_cluster_breaks.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_lexical_classes.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_line_break_classes.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_scripts.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_sentence_break_properties.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/ucd_word_break_properties.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode_bidi.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode_break_opportunity.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode_description.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode_grapheme_cluster_break.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode_line_break.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode_normalization.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode_plural.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode_sentence_break.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/unicode/unicode_word_break.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/architecture.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/assert.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/bits.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/cast.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/charconv.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/compare.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/concepts.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/debugger_intf.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/debugger_win32_impl.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/debugger.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/defer.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/endian.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/enum_metadata.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/exception_intf.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/exception_win32_impl.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/exception.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/fixed_string.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/float16.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/forward_value.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/hash.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/math.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/memory.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/misc.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/utility.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/not_null.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/numbers.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/policy.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/reflection.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/tagged_id.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/time_zone.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/type_traits.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/units.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/utility/value_traits.ixx
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/win32/base.ixx>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/win32/win32.ixx>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/win32/winnls.ixx>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/win32/winreg.ixx>
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/hikogui.ixx
)

target_sources(hikogui INTERFACE FILE_SET hikogui_include_files TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/mod/"  FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/macros.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/crt.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/test.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mod/hikogui/win32_headers.hpp
)
