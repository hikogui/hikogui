
target_sources(hikogui PUBLIC FILE_SET hikogui_include_files TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/" FILES
    ${HIKOGUI_SOURCE_DIR}/image/pixmap.hpp
    ${HIKOGUI_SOURCE_DIR}/image/pixmap_view.hpp

)
