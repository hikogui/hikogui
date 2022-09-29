# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


target_sources(hikogui PRIVATE
    file.hpp
    $<${HI_POSIX}:${CMAKE_CURRENT_SOURCE_DIR}/file_posix.cpp>
    $<${HI_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/file_win32.cpp>
    file_mapping.cpp
    file_mapping.hpp
    $<${HI_POSIX}:${CMAKE_CURRENT_SOURCE_DIR}/file_mapping_posix.cpp>
    $<${HI_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/file_mapping_win32.cpp>
    file_view.cpp
    file_view.hpp
    $<${HI_POSIX}:${CMAKE_CURRENT_SOURCE_DIR}/file_view_posix.cpp>
    $<${HI_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/file_view_win32.cpp>
    glob.hpp
    path_location.hpp
    $<${HI_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/path_location_macos.mm>
    $<${HI_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/path_location_win32.cpp>
    resource_view.hpp
    URI.hpp
    URL.hpp
)

if(HI_BUILD_TESTS)
    target_sources(hikogui_tests PRIVATE
        file_view_tests.cpp
        glob_tests.cpp
        URI_tests.cpp
        URL_tests.cpp
    )
endif()
