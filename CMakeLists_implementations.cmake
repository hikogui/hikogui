
target_sources(hikogui PRIVATE
    ${HIKOGUI_SOURCE_DIR}/geometry/axis_aligned_rectangle_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/dead_lock_detector_impl.cpp
    ${HIKOGUI_SOURCE_DIR}/utility/thread_impl.cpp
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/utility/debugger_win32_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/utility/exception_win32_impl.cpp>
    $<$<PLATFORM_ID:Darwin>:${HIKOGUI_SOURCE_DIR}/utility/thread_macos_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/utility/thread_win32_impl.cpp>
)
