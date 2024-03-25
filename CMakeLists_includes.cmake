# This file was generated with tools/generate_cmakelists.sh

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

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/metadata/library_metadata.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/hikogui/metadata/library_metadata.hpp @ONLY)

target_sources(hikogui INTERFACE FILE_SET hikogui_generated_include_files TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/src/" FILES
    ${CMAKE_CURRENT_BINARY_DIR}/src/hikogui/metadata/library_metadata.hpp
)

target_sources(hikogui INTERFACE FILE_SET hikogui_include_files TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/"  FILES
    src/hikogui/DSP/DSP.hpp
    src/hikogui/DSP/dsp_float.hpp
    src/hikogui/DSP/dsp_mul.hpp
    src/hikogui/DSP/for_each.hpp
    src/hikogui/GFX/GFX.hpp
    src/hikogui/GFX/draw_context_impl.hpp
    src/hikogui/GFX/draw_context_intf.hpp
    src/hikogui/GFX/gfx_device_vulkan_impl.hpp
    src/hikogui/GFX/gfx_device_vulkan_intf.hpp
    src/hikogui/GFX/gfx_pipeline_SDF_vulkan_impl.hpp
    src/hikogui/GFX/gfx_pipeline_SDF_vulkan_intf.hpp
    src/hikogui/GFX/gfx_pipeline_box_vulkan_impl.hpp
    src/hikogui/GFX/gfx_pipeline_box_vulkan_intf.hpp
    src/hikogui/GFX/gfx_pipeline_image_vulkan_impl.hpp
    src/hikogui/GFX/gfx_pipeline_image_vulkan_intf.hpp
    src/hikogui/GFX/gfx_pipeline_override_vulkan_impl.hpp
    src/hikogui/GFX/gfx_pipeline_override_vulkan_intf.hpp
    src/hikogui/GFX/gfx_pipeline_tone_mapper_vulkan_impl.hpp
    src/hikogui/GFX/gfx_pipeline_tone_mapper_vulkan_intf.hpp
    src/hikogui/GFX/gfx_pipeline_vulkan_impl.hpp
    src/hikogui/GFX/gfx_pipeline_vulkan_intf.hpp
    src/hikogui/GFX/gfx_queue_vulkan.hpp
    src/hikogui/GFX/gfx_surface_delegate_vulkan.hpp
    src/hikogui/GFX/gfx_surface_state.hpp
    src/hikogui/GFX/gfx_surface_vulkan_impl.hpp
    src/hikogui/GFX/gfx_surface_vulkan_intf.hpp
    src/hikogui/GFX/gfx_system_globals.hpp
    src/hikogui/GFX/gfx_system_vulkan_impl.hpp
    src/hikogui/GFX/gfx_system_vulkan_intf.hpp
    src/hikogui/GFX/render_doc.hpp
    src/hikogui/GFX/renderdoc_app.h
    src/hikogui/GUI/GUI.hpp
    src/hikogui/GUI/gui_event.hpp
    src/hikogui/GUI/gui_event_type.hpp
    src/hikogui/GUI/gui_event_variant.hpp
    src/hikogui/GUI/gui_window_size.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/GUI/gui_window_win32.hpp>
    src/hikogui/GUI/gui_window_win32.hpp
    src/hikogui/GUI/hitbox.hpp
    src/hikogui/GUI/keyboard_bindings.hpp
    src/hikogui/GUI/keyboard_focus_direction.hpp
    src/hikogui/GUI/keyboard_focus_group.hpp
    src/hikogui/GUI/keyboard_key.hpp
    src/hikogui/GUI/keyboard_modifiers.hpp
    src/hikogui/GUI/keyboard_state.hpp
    src/hikogui/GUI/keyboard_virtual_key_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/GUI/keyboard_virtual_key_win32_impl.hpp>
    src/hikogui/GUI/keyboard_virtual_key_win32_impl.hpp
    src/hikogui/GUI/mouse_buttons.hpp
    src/hikogui/GUI/mouse_cursor.hpp
    src/hikogui/GUI/theme.hpp
    src/hikogui/GUI/theme_book.hpp
    src/hikogui/GUI/widget_id.hpp
    src/hikogui/GUI/widget_intf.hpp
    src/hikogui/GUI/widget_layout.hpp
    src/hikogui/GUI/widget_state.hpp
    src/hikogui/SIMD/SIMD.hpp
    $<$<STREQUAL:${ARCHITECTURE_ID},none>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/array_generic.hpp>
    src/hikogui/SIMD/array_generic.hpp
    src/hikogui/SIMD/array_intrinsic.hpp
    src/hikogui/SIMD/array_intrinsic_f16x4.hpp
    $<$<STREQUAL:${ARCHITECTURE_ID},x86>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/array_intrinsic_f32x4_x86.hpp>
    src/hikogui/SIMD/array_intrinsic_f32x4_x86.hpp
    $<$<STREQUAL:${ARCHITECTURE_ID},x86>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/array_intrinsic_f64x2_x86.hpp>
    src/hikogui/SIMD/array_intrinsic_f64x2_x86.hpp
    $<$<STREQUAL:${ARCHITECTURE_ID},x86>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/array_intrinsic_f64x4_x86.hpp>
    src/hikogui/SIMD/array_intrinsic_f64x4_x86.hpp
    $<$<STREQUAL:${ARCHITECTURE_ID},none>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/cpu_id_generic.hpp>
    src/hikogui/SIMD/cpu_id_generic.hpp
    $<$<STREQUAL:${ARCHITECTURE_ID},x86>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/SIMD/cpu_id_x86.hpp>
    src/hikogui/SIMD/cpu_id_x86.hpp
    src/hikogui/SIMD/float_to_half.hpp
    src/hikogui/SIMD/half.hpp
    src/hikogui/SIMD/half_to_float.hpp
    src/hikogui/SIMD/simd_intf.hpp
    src/hikogui/algorithm/algorithm.hpp
    src/hikogui/algorithm/algorithm_misc.hpp
    src/hikogui/algorithm/animator.hpp
    src/hikogui/algorithm/ranges.hpp
    src/hikogui/algorithm/recursive_iterator.hpp
    src/hikogui/algorithm/strings.hpp
    src/hikogui/audio/audio.hpp
    src/hikogui/audio/audio_block.hpp
    src/hikogui/audio/audio_channel.hpp
    src/hikogui/audio/audio_device.hpp
    src/hikogui/audio/audio_device_asio.hpp
    src/hikogui/audio/audio_device_delegate.hpp
    src/hikogui/audio/audio_device_state.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/audio/audio_device_win32.hpp>
    src/hikogui/audio/audio_device_win32.hpp
    src/hikogui/audio/audio_direction.hpp
    src/hikogui/audio/audio_format_range.hpp
    src/hikogui/audio/audio_sample_format.hpp
    src/hikogui/audio/audio_sample_packer.hpp
    src/hikogui/audio/audio_sample_unpacker.hpp
    src/hikogui/audio/audio_stream_config.hpp
    src/hikogui/audio/audio_stream_format.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/audio/audio_stream_format_win32.hpp>
    src/hikogui/audio/audio_stream_format_win32.hpp
    src/hikogui/audio/audio_system.hpp
    src/hikogui/audio/audio_system_aggregate.hpp
    src/hikogui/audio/audio_system_asio.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/audio/audio_system_win32.hpp>
    src/hikogui/audio/audio_system_win32.hpp
    src/hikogui/audio/pcm_format.hpp
    src/hikogui/audio/speaker_mapping.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/audio/speaker_mapping_win32.hpp>
    src/hikogui/audio/speaker_mapping_win32.hpp
    src/hikogui/audio/surround_mode.hpp
    src/hikogui/audio/win32_device_interface.hpp
    src/hikogui/audio/win32_wave_device.hpp
    src/hikogui/char_maps/ascii.hpp
    src/hikogui/char_maps/char_converter.hpp
    src/hikogui/char_maps/char_maps.hpp
    src/hikogui/char_maps/cp_1252.hpp
    src/hikogui/char_maps/iso_8859_1.hpp
    src/hikogui/char_maps/random_char.hpp
    src/hikogui/char_maps/to_string.hpp
    src/hikogui/char_maps/utf_16.hpp
    src/hikogui/char_maps/utf_32.hpp
    src/hikogui/char_maps/utf_8.hpp
    src/hikogui/codec/BON8.hpp
    src/hikogui/codec/JSON.hpp
    src/hikogui/codec/SHA2.hpp
    src/hikogui/codec/base_n.hpp
    src/hikogui/codec/codec.hpp
    src/hikogui/codec/datum.hpp
    src/hikogui/codec/gzip.hpp
    src/hikogui/codec/huffman.hpp
    src/hikogui/codec/indent.hpp
    src/hikogui/codec/inflate.hpp
    src/hikogui/codec/jsonpath.hpp
    src/hikogui/codec/pickle.hpp
    src/hikogui/codec/png.hpp
    src/hikogui/codec/zlib.hpp
    src/hikogui/color/Rec2020.hpp
    src/hikogui/color/Rec2100.hpp
    src/hikogui/color/color.hpp
    src/hikogui/color/color_intf.hpp
    src/hikogui/color/color_space.hpp
    src/hikogui/color/quad_color.hpp
    src/hikogui/color/sRGB.hpp
    src/hikogui/color/semantic_color.hpp
    src/hikogui/concurrency/atomic.hpp
    src/hikogui/concurrency/callback.hpp
    src/hikogui/concurrency/callback_flags.hpp
    src/hikogui/concurrency/concurrency.hpp
    src/hikogui/concurrency/global_state.hpp
    src/hikogui/concurrency/id_factory.hpp
    src/hikogui/concurrency/subsystem.hpp
    src/hikogui/concurrency/thread.hpp
    src/hikogui/concurrency/thread_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/concurrency/thread_win32_impl.hpp>
    src/hikogui/concurrency/thread_win32_impl.hpp
    src/hikogui/concurrency/unfair_mutex.hpp
    src/hikogui/concurrency/unfair_mutex_impl.hpp
    src/hikogui/concurrency/unfair_mutex_intf.hpp
    src/hikogui/concurrency/unfair_recursive_mutex.hpp
    src/hikogui/container/byte_string.hpp
    src/hikogui/container/container.hpp
    src/hikogui/container/function_fifo.hpp
    src/hikogui/container/functional.hpp
    src/hikogui/container/lean_vector.hpp
    src/hikogui/container/polymorphic_optional.hpp
    src/hikogui/container/secure_vector.hpp
    src/hikogui/container/stable_set.hpp
    src/hikogui/container/stack.hpp
    src/hikogui/container/undo_stack.hpp
    src/hikogui/container/vector_span.hpp
    src/hikogui/container/void_span.hpp
    src/hikogui/container/wfree_fifo.hpp
    src/hikogui/crt.hpp
    src/hikogui/crt/crt.hpp
    src/hikogui/crt/crt_utils.hpp
    src/hikogui/crt/crt_utils_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/crt/crt_utils_win32_impl.hpp>
    src/hikogui/crt/crt_utils_win32_impl.hpp
    src/hikogui/dispatch/async_task.hpp
    src/hikogui/dispatch/awaitable.hpp
    src/hikogui/dispatch/awaitable_stop_token_impl.hpp
    src/hikogui/dispatch/awaitable_stop_token_intf.hpp
    src/hikogui/dispatch/awaitable_timer_impl.hpp
    src/hikogui/dispatch/awaitable_timer_intf.hpp
    src/hikogui/dispatch/dispatch.hpp
    src/hikogui/dispatch/function_timer.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/dispatch/loop_win32_intf.hpp>
    src/hikogui/dispatch/loop_win32_intf.hpp
    src/hikogui/dispatch/notifier.hpp
    src/hikogui/dispatch/progress.hpp
    src/hikogui/dispatch/socket_event.hpp
    src/hikogui/dispatch/socket_event_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/dispatch/socket_event_win32_impl.hpp>
    src/hikogui/dispatch/socket_event_win32_impl.hpp
    src/hikogui/dispatch/task.hpp
    src/hikogui/dispatch/task_controller.hpp
    src/hikogui/dispatch/when_any.hpp
    src/hikogui/file/access_mode.hpp
    src/hikogui/file/file.hpp
    src/hikogui/file/file_intf.hpp
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/file/file_posix_impl.hpp>
    src/hikogui/file/file_posix_impl.hpp
    src/hikogui/file/file_view.hpp
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/file/file_view_posix_impl.hpp>
    src/hikogui/file/file_view_posix_impl.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/file/file_view_win32_impl.hpp>
    src/hikogui/file/file_view_win32_impl.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/file/file_win32_impl.hpp>
    src/hikogui/file/file_win32_impl.hpp
    src/hikogui/file/resource_view.hpp
    src/hikogui/file/seek_whence.hpp
    src/hikogui/font/elusive_icon.hpp
    src/hikogui/font/font.hpp
    src/hikogui/font/font_book.hpp
    src/hikogui/font/font_char_map.hpp
    src/hikogui/font/font_family_id.hpp
    src/hikogui/font/font_font.hpp
    src/hikogui/font/font_metrics.hpp
    src/hikogui/font/font_style.hpp
    src/hikogui/font/font_variant.hpp
    src/hikogui/font/font_weight.hpp
    src/hikogui/font/glyph_atlas_info.hpp
    src/hikogui/font/glyph_id.hpp
    src/hikogui/font/glyph_metrics.hpp
    src/hikogui/font/hikogui_icon.hpp
    src/hikogui/font/otype_GSUB.hpp
    src/hikogui/font/otype_cmap.hpp
    src/hikogui/font/otype_coverage.hpp
    src/hikogui/font/otype_glyf.hpp
    src/hikogui/font/otype_head.hpp
    src/hikogui/font/otype_hhea.hpp
    src/hikogui/font/otype_hmtx.hpp
    src/hikogui/font/otype_kern.hpp
    src/hikogui/font/otype_loca.hpp
    src/hikogui/font/otype_maxp.hpp
    src/hikogui/font/otype_name.hpp
    src/hikogui/font/otype_os2.hpp
    src/hikogui/font/otype_sfnt.hpp
    src/hikogui/font/otype_utilities.hpp
    src/hikogui/font/true_type_font.hpp
    src/hikogui/geometry/aarectangle.hpp
    src/hikogui/geometry/alignment.hpp
    src/hikogui/geometry/axis.hpp
    src/hikogui/geometry/circle.hpp
    src/hikogui/geometry/corner_radii.hpp
    src/hikogui/geometry/extent2.hpp
    src/hikogui/geometry/extent3.hpp
    src/hikogui/geometry/geometry.hpp
    src/hikogui/geometry/line_end_cap.hpp
    src/hikogui/geometry/line_join_style.hpp
    src/hikogui/geometry/line_segment.hpp
    src/hikogui/geometry/lookat.hpp
    src/hikogui/geometry/margins.hpp
    src/hikogui/geometry/matrix2.hpp
    src/hikogui/geometry/matrix3.hpp
    src/hikogui/geometry/perspective.hpp
    src/hikogui/geometry/point2.hpp
    src/hikogui/geometry/point3.hpp
    src/hikogui/geometry/quad.hpp
    src/hikogui/geometry/rectangle.hpp
    src/hikogui/geometry/rotate2.hpp
    src/hikogui/geometry/rotate3.hpp
    src/hikogui/geometry/scale2.hpp
    src/hikogui/geometry/scale3.hpp
    src/hikogui/geometry/transform.hpp
    src/hikogui/geometry/translate2.hpp
    src/hikogui/geometry/translate3.hpp
    src/hikogui/geometry/vector2.hpp
    src/hikogui/geometry/vector3.hpp
    src/hikogui/graphic_path/bezier.hpp
    src/hikogui/graphic_path/bezier_curve.hpp
    src/hikogui/graphic_path/bezier_point.hpp
    src/hikogui/graphic_path/graphic_path.hpp
    src/hikogui/hikogui.hpp
    src/hikogui/i18n/i18n.hpp
    src/hikogui/i18n/iso_15924.hpp
    src/hikogui/i18n/iso_15924_impl.hpp
    src/hikogui/i18n/iso_15924_intf.hpp
    src/hikogui/i18n/iso_3166.hpp
    src/hikogui/i18n/iso_3166_impl.hpp
    src/hikogui/i18n/iso_3166_intf.hpp
    src/hikogui/i18n/iso_639.hpp
    src/hikogui/i18n/language_tag.hpp
    src/hikogui/i18n/language_tag_impl.hpp
    src/hikogui/i18n/language_tag_intf.hpp
    src/hikogui/image/image.hpp
    src/hikogui/image/pixmap.hpp
    src/hikogui/image/pixmap_span.hpp
    src/hikogui/image/sdf_r8.hpp
    src/hikogui/image/sfloat_rg32.hpp
    src/hikogui/image/sfloat_rgb32.hpp
    src/hikogui/image/sfloat_rgba16.hpp
    src/hikogui/image/sfloat_rgba32.hpp
    src/hikogui/image/sfloat_rgba32x4.hpp
    src/hikogui/image/sint_abgr8_pack.hpp
    src/hikogui/image/snorm_r8.hpp
    src/hikogui/image/srgb_abgr8_pack.hpp
    src/hikogui/image/uint_abgr8_pack.hpp
    src/hikogui/image/unorm_a2bgr10_pack.hpp
    src/hikogui/l10n/l10n.hpp
    src/hikogui/l10n/label.hpp
    src/hikogui/l10n/po_parser.hpp
    src/hikogui/l10n/po_translations.hpp
    src/hikogui/l10n/translation.hpp
    src/hikogui/l10n/txt.hpp
    src/hikogui/layout/box_constraints.hpp
    src/hikogui/layout/box_shape.hpp
    src/hikogui/layout/grid_layout.hpp
    src/hikogui/layout/layout.hpp
    src/hikogui/layout/row_column_layout.hpp
    src/hikogui/layout/spreadsheet_address.hpp
    src/hikogui/macros.hpp
    src/hikogui/memory/locked_memory_allocator.hpp
    src/hikogui/memory/locked_memory_allocator_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/memory/locked_memory_allocator_win32_impl.hpp>
    src/hikogui/memory/locked_memory_allocator_win32_impl.hpp
    src/hikogui/memory/memory.hpp
    src/hikogui/memory/secure_memory_allocator.hpp
    src/hikogui/metadata/application_metadata.hpp
    src/hikogui/metadata/metadata.hpp
    src/hikogui/metadata/semantic_version.hpp
    src/hikogui/net/net.hpp
    src/hikogui/net/packet.hpp
    src/hikogui/net/packet_buffer.hpp
    src/hikogui/net/stream.hpp
    src/hikogui/numeric/bigint.hpp
    src/hikogui/numeric/decimal.hpp
    src/hikogui/numeric/int_carry.hpp
    src/hikogui/numeric/numeric.hpp
    src/hikogui/numeric/polynomial.hpp
    src/hikogui/observer/group_ptr.hpp
    src/hikogui/observer/observed.hpp
    src/hikogui/observer/observer.hpp
    src/hikogui/observer/observer_intf.hpp
    src/hikogui/observer/shared_state.hpp
    src/hikogui/parser/lexer.hpp
    src/hikogui/parser/lookahead_iterator.hpp
    src/hikogui/parser/operator.hpp
    src/hikogui/parser/parser.hpp
    src/hikogui/parser/placement.hpp
    src/hikogui/parser/token.hpp
    src/hikogui/path/URI.hpp
    src/hikogui/path/URL.hpp
    src/hikogui/path/glob.hpp
    src/hikogui/path/path.hpp
    src/hikogui/path/path_location.hpp
    src/hikogui/path/path_location_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/path/path_location_win32_impl.hpp>
    src/hikogui/path/path_location_win32_impl.hpp
    src/hikogui/random/dither.hpp
    src/hikogui/random/random.hpp
    src/hikogui/random/seed.hpp
    src/hikogui/random/seed_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/random/seed_win32_impl.hpp>
    src/hikogui/random/seed_win32_impl.hpp
    src/hikogui/random/xorshift128p.hpp
    src/hikogui/security/security.hpp
    src/hikogui/security/security_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/security/security_win32_impl.hpp>
    src/hikogui/security/security_win32_impl.hpp
    src/hikogui/security/sip_hash.hpp
    src/hikogui/settings/os_settings.hpp
    src/hikogui/settings/os_settings_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/settings/os_settings_win32_impl.hpp>
    src/hikogui/settings/os_settings_win32_impl.hpp
    src/hikogui/settings/preferences.hpp
    src/hikogui/settings/settings.hpp
    src/hikogui/settings/subpixel_orientation.hpp
    src/hikogui/settings/theme_mode.hpp
    src/hikogui/settings/user_settings.hpp
    src/hikogui/settings/user_settings_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/settings/user_settings_win32_impl.hpp>
    src/hikogui/settings/user_settings_win32_impl.hpp
    src/hikogui/telemetry/counters.hpp
    src/hikogui/telemetry/delayed_format.hpp
    src/hikogui/telemetry/format_check.hpp
    src/hikogui/telemetry/log.hpp
    src/hikogui/telemetry/telemetry.hpp
    src/hikogui/telemetry/trace.hpp
    src/hikogui/test.hpp
    src/hikogui/text/semantic_text_style.hpp
    src/hikogui/text/text.hpp
    src/hikogui/text/text_cursor.hpp
    src/hikogui/text/text_decoration.hpp
    src/hikogui/text/text_selection.hpp
    src/hikogui/text/text_shaper.hpp
    src/hikogui/text/text_shaper_char.hpp
    src/hikogui/text/text_shaper_line.hpp
    src/hikogui/text/text_style.hpp
    src/hikogui/time/chrono.hpp
    src/hikogui/time/time.hpp
    src/hikogui/time/time_stamp_count.hpp
    src/hikogui/time/time_stamp_utc.hpp
    src/hikogui/unicode/grapheme.hpp
    src/hikogui/unicode/gstring.hpp
    src/hikogui/unicode/markup.hpp
    src/hikogui/unicode/phrasing.hpp
    src/hikogui/unicode/ucd_bidi_classes.hpp
    src/hikogui/unicode/ucd_bidi_mirroring_glyphs.hpp
    src/hikogui/unicode/ucd_bidi_paired_bracket_types.hpp
    src/hikogui/unicode/ucd_canonical_combining_classes.hpp
    src/hikogui/unicode/ucd_compositions.hpp
    src/hikogui/unicode/ucd_decompositions.hpp
    src/hikogui/unicode/ucd_east_asian_widths.hpp
    src/hikogui/unicode/ucd_general_categories.hpp
    src/hikogui/unicode/ucd_grapheme_cluster_breaks.hpp
    src/hikogui/unicode/ucd_lexical_classes.hpp
    src/hikogui/unicode/ucd_line_break_classes.hpp
    src/hikogui/unicode/ucd_scripts.hpp
    src/hikogui/unicode/ucd_sentence_break_properties.hpp
    src/hikogui/unicode/ucd_word_break_properties.hpp
    src/hikogui/unicode/unicode.hpp
    src/hikogui/unicode/unicode_bidi.hpp
    src/hikogui/unicode/unicode_break_opportunity.hpp
    src/hikogui/unicode/unicode_description.hpp
    src/hikogui/unicode/unicode_grapheme_cluster_break.hpp
    src/hikogui/unicode/unicode_line_break.hpp
    src/hikogui/unicode/unicode_normalization.hpp
    src/hikogui/unicode/unicode_plural.hpp
    src/hikogui/unicode/unicode_sentence_break.hpp
    src/hikogui/unicode/unicode_word_break.hpp
    src/hikogui/utility/architecture.hpp
    src/hikogui/utility/assert.hpp
    src/hikogui/utility/bits.hpp
    src/hikogui/utility/cast.hpp
    src/hikogui/utility/charconv.hpp
    src/hikogui/utility/compare.hpp
    src/hikogui/utility/concepts.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/console_win32.hpp>
    src/hikogui/utility/console_win32.hpp
    src/hikogui/utility/debugger.hpp
    $<$<STREQUAL:${ARCHITECTURE_ID},none>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/debugger_generic_impl.hpp>
    src/hikogui/utility/debugger_generic_impl.hpp
    src/hikogui/utility/debugger_intf.hpp
    src/hikogui/utility/debugger_utils.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/debugger_win32_impl.hpp>
    src/hikogui/utility/debugger_win32_impl.hpp
    src/hikogui/utility/defer.hpp
    src/hikogui/utility/dialog.hpp
    src/hikogui/utility/dialog_intf.hpp
    $<$<PLATFORM_ID:MacOS>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/dialog_macos_impl.hpp>
    src/hikogui/utility/dialog_macos_impl.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/dialog_win32_impl.hpp>
    src/hikogui/utility/dialog_win32_impl.hpp
    src/hikogui/utility/endian.hpp
    src/hikogui/utility/enum_metadata.hpp
    src/hikogui/utility/exception.hpp
    src/hikogui/utility/exception_intf.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/exception_win32_impl.hpp>
    src/hikogui/utility/exception_win32_impl.hpp
    src/hikogui/utility/fixed_string.hpp
    src/hikogui/utility/forward_value.hpp
    src/hikogui/utility/generator.hpp
    src/hikogui/utility/hash.hpp
    src/hikogui/utility/initialize.hpp
    src/hikogui/utility/math.hpp
    src/hikogui/utility/memory.hpp
    src/hikogui/utility/misc.hpp
    src/hikogui/utility/not_null.hpp
    src/hikogui/utility/numbers.hpp
    src/hikogui/utility/policy.hpp
    src/hikogui/utility/reflection.hpp
    src/hikogui/utility/tagged_id.hpp
    src/hikogui/utility/terminate.hpp
    src/hikogui/utility/time_zone.hpp
    src/hikogui/utility/type_traits.hpp
    src/hikogui/utility/units.hpp
    src/hikogui/utility/utility.hpp
    src/hikogui/utility/value_traits.hpp
    src/hikogui/widgets/abstract_button_widget.hpp
    src/hikogui/widgets/async_delegate.hpp
    src/hikogui/widgets/async_widget.hpp
    src/hikogui/widgets/audio_device_widget.hpp
    src/hikogui/widgets/button_delegate.hpp
    src/hikogui/widgets/checkbox_widget.hpp
    src/hikogui/widgets/grid_widget.hpp
    src/hikogui/widgets/icon_widget.hpp
    src/hikogui/widgets/label_widget.hpp
    src/hikogui/widgets/menu_button_widget.hpp
    src/hikogui/widgets/momentary_button_widget.hpp
    src/hikogui/widgets/overlay_widget.hpp
    src/hikogui/widgets/radio_delegate.hpp
    src/hikogui/widgets/radio_widget.hpp
    src/hikogui/widgets/scroll_aperture_widget.hpp
    src/hikogui/widgets/scroll_bar_widget.hpp
    src/hikogui/widgets/scroll_widget.hpp
    src/hikogui/widgets/selection_delegate.hpp
    src/hikogui/widgets/selection_widget.hpp
    src/hikogui/widgets/spacer_widget.hpp
    src/hikogui/widgets/system_menu_widget.hpp
    src/hikogui/widgets/tab_delegate.hpp
    src/hikogui/widgets/tab_widget.hpp
    src/hikogui/widgets/text_delegate.hpp
    src/hikogui/widgets/text_field_delegate.hpp
    src/hikogui/widgets/text_field_widget.hpp
    src/hikogui/widgets/text_widget.hpp
    src/hikogui/widgets/toggle_delegate.hpp
    src/hikogui/widgets/toggle_widget.hpp
    src/hikogui/widgets/toolbar_button_widget.hpp
    src/hikogui/widgets/toolbar_tab_button_widget.hpp
    src/hikogui/widgets/toolbar_widget.hpp
    src/hikogui/widgets/widget.hpp
    src/hikogui/widgets/widgets.hpp
    src/hikogui/widgets/window_controls_macos_widget.hpp
    src/hikogui/widgets/window_controls_win32_widget.hpp
    src/hikogui/widgets/window_widget.hpp
    src/hikogui/widgets/with_label_widget.hpp
    src/hikogui/win32/hresult_error_impl.hpp
    src/hikogui/win32/hresult_error_intf.hpp
    src/hikogui/win32/libloaderapi.hpp
    src/hikogui/win32/processthreadsapi.hpp
    src/hikogui/win32/shlobj_core.hpp
    src/hikogui/win32/stringapiset.hpp
    src/hikogui/win32/synchapi.hpp
    src/hikogui/win32/utility.hpp
    src/hikogui/win32/win32.hpp
    src/hikogui/win32/win32_error_impl.hpp
    src/hikogui/win32/win32_error_intf.hpp
    src/hikogui/win32/winbase.hpp
    src/hikogui/win32/winnls.hpp
    src/hikogui/win32/winreg.hpp
    src/hikogui/win32/winuser.hpp
    src/hikogui/win32_headers.hpp
)

