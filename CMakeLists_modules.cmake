
target_sources(hikogui PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/" FILES
    ${HIKOGUI_SOURCE_DIR}/hikogui.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/utility.ixx
    ${HIKOGUI_SOURCE_DIR}/utility/defer.ixx
)
