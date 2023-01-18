
target_precompile_headers(hikogui PUBLIC
    ${HIKOGUI_SOURCE_DIR}/module.hpp
)

target_precompile_headers(hikogui PRIVATE
    ${HIKOGUI_SOURCE_DIR}/win32_headers.hpp
)
