
target_sources(hikogui PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/" FILES
    ${HIKOGUI_SOURCE_DIR}/hikogui.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/architecture.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/assert.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/bits.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/cast.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/charconv.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/compare.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/concepts.ixx
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/utility/debugger_win32_impl.ixx>
    ${HIKOGUI_SOURCE_DIR}/utility/debugger_intf.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/debugger.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/defer.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/endian.ixx
    $<$<PLATFORM_ID:Windows>:${HIKOGUI_SOURCE_DIR}/utility/exception_win32_impl.ixx>
    ${HIKOGUI_SOURCE_DIR}/utility/exception_intf.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/exception.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/utility.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/misc.ixx
)
