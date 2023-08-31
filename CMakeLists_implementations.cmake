
target_sources(hikogui PRIVATE
    ${HIKOGUI_SOURCE_DIR}/GFX/draw_context_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/GFX/gfx_pipeline_vulkan_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/GFX/RenderDoc_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/GFX/VulkanMemoryAllocator_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/GUI/gui_system_impl.cpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/GUI/gui_system_win32_impl.cpp>
    ${HIKOGUI_SOURCE_DIR}/GUI/gui_window_impl.cpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/GUI/gui_window_win32_impl.cpp>
    ${HIKOGUI_SOURCE_DIR}/GUI/keyboard_bindings_impl.cpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/GUI/keyboard_virtual_key_win32_impl.cpp>
    ${HIKOGUI_SOURCE_DIR}/GUI/theme_book_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/GUI/theme_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/skeleton/skeleton_parse_context_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/text/text_shaper_char_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/text/text_shaper_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/text/text_shaper_line_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/text/text_style_impl.cpp
) 
