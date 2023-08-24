
configure_file(
    ${HIKOGUI_SOURCE_DIR}/metadata/library_metadata.hpp.in
    ${HIKOGUI_SOURCE_DIR}/metadata/library_metadata.hpp @ONLY)

target_sources(hikogui PUBLIC FILE_SET hikogui_include_files TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/" FILES
    ${HIKOGUI_SOURCE_DIR}/audio/audio_block.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_channel.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_device.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_device_asio.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_device_delegate.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_device_state.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/audio/audio_device_win32.hpp>
    ${HIKOGUI_SOURCE_DIR}/audio/audio_direction.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_format_range.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_sample_format.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_sample_packer.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_sample_unpacker.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_stream_config.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_stream_format.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/audio/audio_stream_format_win32.hpp>
    ${HIKOGUI_SOURCE_DIR}/audio/audio_system.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_system_aggregate.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/audio_system_asio.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/audio/audio_system_win32.hpp>
    ${HIKOGUI_SOURCE_DIR}/audio/audio.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/pcm_format.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/speaker_mapping.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/audio/speaker_mapping_win32.hpp>
    ${HIKOGUI_SOURCE_DIR}/audio/surround_mode.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/win32_device_interface.hpp
    ${HIKOGUI_SOURCE_DIR}/audio/win32_wave_device.hpp
    ${HIKOGUI_SOURCE_DIR}/algorithm/algorithm.hpp
    ${HIKOGUI_SOURCE_DIR}/algorithm/animator.hpp
    ${HIKOGUI_SOURCE_DIR}/algorithm/lookahead_iterator.hpp
    ${HIKOGUI_SOURCE_DIR}/algorithm/module.hpp
    ${HIKOGUI_SOURCE_DIR}/algorithm/ranges.hpp
    ${HIKOGUI_SOURCE_DIR}/algorithm/recursive_iterator.hpp
    ${HIKOGUI_SOURCE_DIR}/algorithm/strings.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/ascii.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/char_converter.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/cp_1252.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/iso_8859_1.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/module.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/random_char.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/to_string.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_16.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_32.hpp
    ${HIKOGUI_SOURCE_DIR}/char_maps/utf_8.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/base_n.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/BON8.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/datum.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/gzip.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/huffman.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/indent.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/inflate.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/JSON.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/jsonpath.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/codec.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/pickle.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/png.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/SHA2.hpp
    ${HIKOGUI_SOURCE_DIR}/codec/zlib.hpp
    ${HIKOGUI_SOURCE_DIR}/color/color.hpp
    ${HIKOGUI_SOURCE_DIR}/color/color_space.hpp
    ${HIKOGUI_SOURCE_DIR}/color/module.hpp
    ${HIKOGUI_SOURCE_DIR}/color/quad_color.hpp
    ${HIKOGUI_SOURCE_DIR}/color/Rec2020.hpp
    ${HIKOGUI_SOURCE_DIR}/color/Rec2100.hpp
    ${HIKOGUI_SOURCE_DIR}/color/semantic_color.hpp
    ${HIKOGUI_SOURCE_DIR}/color/sRGB.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/atomic.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/callback_flags.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/global_state.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/concurrency.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/notifier.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/rcu.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/subsystem.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/thread.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/thread_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/concurrency/thread_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/concurrency/unfair_mutex.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/unfair_mutex_intf.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/unfair_mutex_impl.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/unfair_recursive_mutex.hpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/wfree_idle_count.hpp
    ${HIKOGUI_SOURCE_DIR}/console/console.hpp
    ${HIKOGUI_SOURCE_DIR}/console/dialog.hpp
    ${HIKOGUI_SOURCE_DIR}/console/dialog_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/console/dialog_win32_impl.hpp>
    $<$<PLATFORM_ID:MacOS>:${HIKOGUI_SOURCE_DIR}/console/dialog_macos_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/console/print.hpp
    ${HIKOGUI_SOURCE_DIR}/console/print_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/console/print_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/container/byte_string.hpp
    ${HIKOGUI_SOURCE_DIR}/container/function_fifo.hpp
    ${HIKOGUI_SOURCE_DIR}/container/functional.hpp
    ${HIKOGUI_SOURCE_DIR}/container/gap_buffer.hpp
    ${HIKOGUI_SOURCE_DIR}/container/hash_map.hpp
    ${HIKOGUI_SOURCE_DIR}/container/lean_vector.hpp
    ${HIKOGUI_SOURCE_DIR}/container/module.hpp
    ${HIKOGUI_SOURCE_DIR}/container/packed_int_array.hpp
    ${HIKOGUI_SOURCE_DIR}/container/polymorphic_optional.hpp
    ${HIKOGUI_SOURCE_DIR}/container/secure_vector.hpp
    ${HIKOGUI_SOURCE_DIR}/container/small_map.hpp
    ${HIKOGUI_SOURCE_DIR}/container/small_vector.hpp
    ${HIKOGUI_SOURCE_DIR}/container/stable_set.hpp
    ${HIKOGUI_SOURCE_DIR}/container/stack.hpp
    ${HIKOGUI_SOURCE_DIR}/container/tree.hpp
    ${HIKOGUI_SOURCE_DIR}/container/undo_stack.hpp
    ${HIKOGUI_SOURCE_DIR}/container/vector_span.hpp
    ${HIKOGUI_SOURCE_DIR}/container/void_span.hpp
    ${HIKOGUI_SOURCE_DIR}/container/wfree_fifo.hpp
    ${HIKOGUI_SOURCE_DIR}/container/wfree_unordered_map.hpp
    ${HIKOGUI_SOURCE_DIR}/coroutine/awaitable.hpp
    ${HIKOGUI_SOURCE_DIR}/coroutine/generator.hpp
    ${HIKOGUI_SOURCE_DIR}/coroutine/module.hpp
    ${HIKOGUI_SOURCE_DIR}/coroutine/scoped_task.hpp
    ${HIKOGUI_SOURCE_DIR}/coroutine/task.hpp
    ${HIKOGUI_SOURCE_DIR}/coroutine/when_any.hpp
    ${HIKOGUI_SOURCE_DIR}/crt/crt.hpp
    ${HIKOGUI_SOURCE_DIR}/crt/crt_utils.hpp
    ${HIKOGUI_SOURCE_DIR}/crt/crt_utils_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/crt/crt_utils_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/crt/terminate.hpp
    ${HIKOGUI_SOURCE_DIR}/file/access_mode.hpp
    ${HIKOGUI_SOURCE_DIR}/file/file_file_intf.hpp
    $<$<PLATFORM_ID:Linux>:${HIKOGUI_SOURCE_DIR}/file/file_file_posix_impl.hpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/file/file_file_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/file/file_file.hpp
    ${HIKOGUI_SOURCE_DIR}/file/file_view_intf.hpp
    $<$<PLATFORM_ID:Linux>:${HIKOGUI_SOURCE_DIR}/file/file_view_posix_impl.hpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/file/file_view_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/file/file_view.hpp
    ${HIKOGUI_SOURCE_DIR}/file/file.hpp
    ${HIKOGUI_SOURCE_DIR}/file/resource_view.hpp
    ${HIKOGUI_SOURCE_DIR}/file/seek_whence.hpp
    ${HIKOGUI_SOURCE_DIR}/font/elusive_icon.hpp
    ${HIKOGUI_SOURCE_DIR}/font/font.hpp
    ${HIKOGUI_SOURCE_DIR}/font/font_book.hpp
    ${HIKOGUI_SOURCE_DIR}/font/font_char_map.hpp
    ${HIKOGUI_SOURCE_DIR}/font/font_family_id.hpp
    ${HIKOGUI_SOURCE_DIR}/font/font_grapheme_id.hpp
    ${HIKOGUI_SOURCE_DIR}/font/font_metrics.hpp
    ${HIKOGUI_SOURCE_DIR}/font/font_variant.hpp
    ${HIKOGUI_SOURCE_DIR}/font/font_weight.hpp
    ${HIKOGUI_SOURCE_DIR}/font/glyph_atlas_info.hpp
    ${HIKOGUI_SOURCE_DIR}/font/glyph_id.hpp
    ${HIKOGUI_SOURCE_DIR}/font/glyph_ids.hpp
    ${HIKOGUI_SOURCE_DIR}/font/glyph_metrics.hpp
    ${HIKOGUI_SOURCE_DIR}/font/hikogui_icon.hpp
    ${HIKOGUI_SOURCE_DIR}/font/module.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_cmap.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_glyf.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_head.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_hhea.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_hmtx.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_kern.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_loca.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_maxp.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_name.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_os2.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_sfnt.hpp
    ${HIKOGUI_SOURCE_DIR}/font/otype_utilities.hpp
    ${HIKOGUI_SOURCE_DIR}/font/true_type_font.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_add_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_arguments.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_assign_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_binary_operator_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_bit_and_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_bit_or_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_bit_xor_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_call_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_decrement_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_div_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_eq_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_evaluation_context.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_filter_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_ge_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_gt_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_increment_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_index_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_add_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_and_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_div_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_mod_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_mul_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_or_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_shl_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_shr_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_sub_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_inplace_xor_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_invert_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_le_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_literal_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_logical_and_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_logical_not_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_logical_or_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_lt_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_map_literal_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_member_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_minus_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_mod_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_mul_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_name_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_ne_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_plus_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_post_process_context.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_pow_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_shl_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_shr_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_sub_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_ternary_operator_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_unary_operator_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula_vector_literal_node.hpp
    ${HIKOGUI_SOURCE_DIR}/formula/formula.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/draw_context.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_device.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_device_vulkan.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_queue_vulkan.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_surface.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_surface_delegate.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_surface_delegate_vulkan.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_surface_state.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_surface_vulkan.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_system.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_system_globals.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_system_vulkan.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/module.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/paged_image.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_alpha.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_alpha_device_shared.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_alpha_push_constants.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_alpha_vertex.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_box.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_box_device_shared.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_box_push_constants.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_box_vertex.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_image.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_image_device_shared.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_image_push_constants.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_image_texture_map.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_image_vertex.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_SDF.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_SDF_device_shared.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_SDF_push_constants.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_SDF_specialization_constants.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_SDF_texture_map.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_SDF_vertex.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_tone_mapper.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_tone_mapper_device_shared.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_tone_mapper_push_constants.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/pipeline_vulkan.hpp
    ${HIKOGUI_SOURCE_DIR}/GFX/RenderDoc.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/gui_event.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/gui_event_type.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/gui_event_variant.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/gui_system.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/gui_system_delegate.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/GUI/gui_system_win32.hpp>
    ${HIKOGUI_SOURCE_DIR}/GUI/gui_window.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/gui_window_size.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/GUI/gui_window_win32.hpp>
    ${HIKOGUI_SOURCE_DIR}/GUI/hitbox.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/keyboard_bindings.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/keyboard_focus_direction.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/keyboard_focus_group.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/keyboard_key.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/keyboard_modifiers.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/keyboard_state.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/keyboard_virtual_key.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/module.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/mouse_buttons.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/mouse_cursor.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/theme.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/theme_book.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/widget_id.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/widget_intf.hpp
    ${HIKOGUI_SOURCE_DIR}/GUI/widget_layout.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/aarectangle.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/alignment.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/axis.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/circle.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/corner_radii.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/extent2.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/extent3.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/line_end_cap.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/line_join_style.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/line_segment.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/lookat.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/margins.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/matrix2.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/matrix3.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/module.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/perspective.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/point2.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/point3.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/quad.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/rectangle.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/rotate2.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/rotate3.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/scale2.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/scale3.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/transform.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/transform_fwd.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/translate2.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/translate3.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/vector2.hpp
    ${HIKOGUI_SOURCE_DIR}/geometry/vector3.hpp
    ${HIKOGUI_SOURCE_DIR}/graphic_path/bezier.hpp
    ${HIKOGUI_SOURCE_DIR}/graphic_path/bezier_curve.hpp
    ${HIKOGUI_SOURCE_DIR}/graphic_path/bezier_point.hpp
    ${HIKOGUI_SOURCE_DIR}/graphic_path/graphic_path.hpp
    ${HIKOGUI_SOURCE_DIR}/graphic_path/graphic_path.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_15924.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_15924_intf.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_15924_impl.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_3166.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_3166_intf.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_3166_impl.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_639.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/language_tag.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/language_tag_intf.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/language_tag_impl.hpp
    ${HIKOGUI_SOURCE_DIR}/i18n/i18n.hpp
    ${HIKOGUI_SOURCE_DIR}/image/module.hpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap.hpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap_span.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sdf_r8.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sfloat_rg32.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sfloat_rgb32.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sfloat_rgba16.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sfloat_rgba32.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sfloat_rgba32x4.hpp
    ${HIKOGUI_SOURCE_DIR}/image/sint_abgr8_pack.hpp
    ${HIKOGUI_SOURCE_DIR}/image/snorm_r8.hpp
    ${HIKOGUI_SOURCE_DIR}/image/srgb_abgr8_pack.hpp
    ${HIKOGUI_SOURCE_DIR}/image/uint_abgr8_pack.hpp
    ${HIKOGUI_SOURCE_DIR}/image/unorm_a2bgr10_pack.hpp
    ${HIKOGUI_SOURCE_DIR}/l10n/label.hpp
    ${HIKOGUI_SOURCE_DIR}/l10n/l10n.hpp
    ${HIKOGUI_SOURCE_DIR}/l10n/po_parser.hpp
    ${HIKOGUI_SOURCE_DIR}/l10n/txt.hpp
    ${HIKOGUI_SOURCE_DIR}/l10n/translation.hpp
    ${HIKOGUI_SOURCE_DIR}/dispatch/awaitable_timer.hpp
    ${HIKOGUI_SOURCE_DIR}/dispatch/awaitable_timer_intf.hpp
    ${HIKOGUI_SOURCE_DIR}/dispatch/awaitable_timer_impl.hpp
    ${HIKOGUI_SOURCE_DIR}/dispatch/function_timer.hpp
    ${HIKOGUI_SOURCE_DIR}/dispatch/loop.hpp
    ${HIKOGUI_SOURCE_DIR}/dispatch/loop_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/dispatch/loop_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/dispatch/dispatch.hpp
    ${HIKOGUI_SOURCE_DIR}/dispatch/socket_event.hpp
    ${HIKOGUI_SOURCE_DIR}/dispatch/socket_event_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/dispatch/socket_event_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/layout/box_constraints.hpp
    ${HIKOGUI_SOURCE_DIR}/layout/box_shape.hpp
    ${HIKOGUI_SOURCE_DIR}/layout/grid_layout.hpp
    ${HIKOGUI_SOURCE_DIR}/layout/module.hpp
    ${HIKOGUI_SOURCE_DIR}/layout/row_column_layout.hpp
    ${HIKOGUI_SOURCE_DIR}/layout/spreadsheet_address.hpp
    ${HIKOGUI_SOURCE_DIR}/memory/locked_memory_allocator.hpp
    ${HIKOGUI_SOURCE_DIR}/memory/locked_memory_allocator_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/memory/locked_memory_allocator_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/memory/memory.hpp
    ${HIKOGUI_SOURCE_DIR}/memory/secure_memory_allocator.hpp
    ${HIKOGUI_SOURCE_DIR}/metadata/application_metadata.hpp
    ${HIKOGUI_SOURCE_DIR}/metadata/library_metadata.hpp # generated.
    ${HIKOGUI_SOURCE_DIR}/metadata/metadata.hpp
    ${HIKOGUI_SOURCE_DIR}/metadata/semantic_version.hpp
    ${HIKOGUI_SOURCE_DIR}/net/module.hpp
    ${HIKOGUI_SOURCE_DIR}/net/packet.hpp
    #${HIKOGUI_SOURCE_DIR}/net/packet_buffer.hpp
    #${HIKOGUI_SOURCE_DIR}/net/stream.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/bigint.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/bound_integer.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/decimal.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/fixed.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/interval.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/int_carry.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/int_overflow.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/module.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/polynomial.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/safe_int.hpp
    ${HIKOGUI_SOURCE_DIR}/numeric/stdint.hpp
    ${HIKOGUI_SOURCE_DIR}/observer/group_ptr.hpp
    ${HIKOGUI_SOURCE_DIR}/observer/module.hpp
    ${HIKOGUI_SOURCE_DIR}/observer/observable.hpp
    ${HIKOGUI_SOURCE_DIR}/observer/observable_value.hpp
    ${HIKOGUI_SOURCE_DIR}/observer/observer.hpp
    ${HIKOGUI_SOURCE_DIR}/observer/shared_state.hpp
    ${HIKOGUI_SOURCE_DIR}/parser/lexer.hpp
    ${HIKOGUI_SOURCE_DIR}/parser/lookahead_iterator.hpp
    ${HIKOGUI_SOURCE_DIR}/parser/operator.hpp
    ${HIKOGUI_SOURCE_DIR}/parser/parse_location.hpp
    ${HIKOGUI_SOURCE_DIR}/parser/parser.hpp
    ${HIKOGUI_SOURCE_DIR}/parser/placement.hpp
    ${HIKOGUI_SOURCE_DIR}/parser/token.hpp
    ${HIKOGUI_SOURCE_DIR}/path/glob.hpp
    ${HIKOGUI_SOURCE_DIR}/path/path.hpp
    ${HIKOGUI_SOURCE_DIR}/path/path_location.hpp
    ${HIKOGUI_SOURCE_DIR}/path/path_location_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/path/path_location_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/path/URI.hpp
    ${HIKOGUI_SOURCE_DIR}/path/URL.hpp
    ${HIKOGUI_SOURCE_DIR}/random/dither.hpp
    ${HIKOGUI_SOURCE_DIR}/random/random.hpp
    ${HIKOGUI_SOURCE_DIR}/random/seed.hpp
    ${HIKOGUI_SOURCE_DIR}/random/seed_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/random/seed_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/random/xorshift128p.hpp
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
    ${HIKOGUI_SOURCE_DIR}/SIMD/simd_test_utility.hpp
    ${HIKOGUI_SOURCE_DIR}/security/module.hpp
    ${HIKOGUI_SOURCE_DIR}/security/security.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/security/security_win32.hpp>
    ${HIKOGUI_SOURCE_DIR}/security/sip_hash.hpp
    #${HIKOGUI_SOURCE_DIR}/settings/cpu_id.hpp
    ${HIKOGUI_SOURCE_DIR}/settings/settings.hpp
    ${HIKOGUI_SOURCE_DIR}/settings/os_settings.hpp
    ${HIKOGUI_SOURCE_DIR}/settings/os_settings_intf.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/settings/os_settings_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/settings/preferences.hpp
    ${HIKOGUI_SOURCE_DIR}/settings/theme_mode.hpp
    ${HIKOGUI_SOURCE_DIR}/settings/subpixel_orientation.hpp
    ${HIKOGUI_SOURCE_DIR}/settings/user_settings.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/settings/user_settings_win32_impl.hpp>
    ${HIKOGUI_SOURCE_DIR}/skeleton/module.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_block_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_break_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_continue_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_do_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_expression_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_for_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_function_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_if_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_parse_context.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_placeholder_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_return_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_string_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_top_node.hpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_while_node.hpp
    ${HIKOGUI_SOURCE_DIR}/telemetry/counters.hpp
    ${HIKOGUI_SOURCE_DIR}/telemetry/delayed_format.hpp
    ${HIKOGUI_SOURCE_DIR}/telemetry/format_check.hpp
    ${HIKOGUI_SOURCE_DIR}/telemetry/log.hpp
    ${HIKOGUI_SOURCE_DIR}/telemetry/module.hpp
    ${HIKOGUI_SOURCE_DIR}/telemetry/trace.hpp
    ${HIKOGUI_SOURCE_DIR}/text/module.hpp
    ${HIKOGUI_SOURCE_DIR}/text/semantic_text_style.hpp
    ${HIKOGUI_SOURCE_DIR}/text/text_cursor.hpp
    ${HIKOGUI_SOURCE_DIR}/text/text_decoration.hpp
    ${HIKOGUI_SOURCE_DIR}/text/text_selection.hpp
    ${HIKOGUI_SOURCE_DIR}/text/text_shaper.hpp
    ${HIKOGUI_SOURCE_DIR}/text/text_shaper_char.hpp
    ${HIKOGUI_SOURCE_DIR}/text/text_shaper_line.hpp
    ${HIKOGUI_SOURCE_DIR}/text/text_style.hpp
    ${HIKOGUI_SOURCE_DIR}/time/chrono.hpp
    ${HIKOGUI_SOURCE_DIR}/time/module.hpp
    ${HIKOGUI_SOURCE_DIR}/time/time_stamp_count.hpp
    ${HIKOGUI_SOURCE_DIR}/time/time_stamp_utc.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/grapheme.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/gstring.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/markup.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/phrasing.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_bidi_classes.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_bidi_mirroring_glyphs.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_bidi_paired_bracket_types.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_canonical_combining_classes.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_compositions.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_decompositions.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_east_asian_widths.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_general_categories.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_grapheme_cluster_breaks.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_lexical_classes.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_line_break_classes.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_scripts.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_sentence_break_properties.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/ucd_word_break_properties.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_bidi.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_break_opportunity.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_description.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_grapheme_cluster_break.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_line_break.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_normalization.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_plural.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_sentence_break.hpp
    ${HIKOGUI_SOURCE_DIR}/unicode/unicode_word_break.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/architecture.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/assert.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/bits.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/cast.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/charconv.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/compare.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/concepts.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/debugger_intf.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/debugger_win32_impl.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/debugger.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/defer.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/endian.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/enum_metadata.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/exception_intf.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/exception_win32_impl.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/exception.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/fixed_string.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/float16.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/forward_value.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/hash.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/math.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/memory.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/misc.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/utility.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/numbers.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/policy.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/reflection.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/tagged_id.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/time_zone.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/type_traits.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/units.hpp
    ${HIKOGUI_SOURCE_DIR}/utility/value_traits.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/abstract_button_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/audio_device_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/button_delegate.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/checkbox_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/grid_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/icon_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/label_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/menu_button_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/module.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/momentary_button_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/overlay_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/radio_button_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/row_column_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/scroll_aperture_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/scroll_bar_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/scroll_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/selection_delegate.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/selection_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/spacer_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/system_menu_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/tab_delegate.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/tab_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/text_delegate.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/text_field_delegate.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/text_field_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/text_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/toggle_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/toolbar_button_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/toolbar_tab_button_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/toolbar_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/widget_mode.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/window_controls_macos_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/window_controls_win32_widget.hpp
    ${HIKOGUI_SOURCE_DIR}/widgets/window_widget.hpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/win32/base.hpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/win32/win32.hpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/win32/winnls.hpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/win32/winreg.hpp>
    ${HIKOGUI_SOURCE_DIR}/macros.hpp
    ${HIKOGUI_SOURCE_DIR}/crt.hpp
    ${HIKOGUI_SOURCE_DIR}/module.hpp
    ${HIKOGUI_SOURCE_DIR}/test.hpp
    ${HIKOGUI_SOURCE_DIR}/win32_headers.hpp
)
