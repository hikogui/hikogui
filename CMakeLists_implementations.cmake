
target_sources(hikogui PRIVATE
    ${HIKOGUI_SOURCE_DIR}/concurrency/dead_lock_detector_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/concurrency/thread_impl.cpp
    $<$<PLATFORM_ID:Darwin>:${HIKOGUI_SOURCE_DIR}/concurrency/thread_macos_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/concurrency/thread_win32_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/file/file_win32_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/file/file_view_win32_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/file/path_location_win32_impl.cpp>    ${HIKOGUI_SOURCE_DIR}/font/font_book_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/font/font_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/font/glyph_ids_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/font/true_type_font_coverage_impl.cpp
    #${HIKOGUI_SOURCE_DIR}/font/true_type_font_GSUB_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/font/true_type_font_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_3166_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/iso_15924_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/i18n/language_tag_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/l10n/translation_impl.cpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/utility/debugger_win32_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/utility/exception_win32_impl.cpp>
)
