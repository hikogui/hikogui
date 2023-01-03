# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


target_sources(hikogui PRIVATE
    file.hpp
    $<$<PLATFORM_ID:Linux,Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/file_posix_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/file_win32_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/file_win32.hpp>
    file_view.hpp
    $<$<PLATFORM_ID:Linux,Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/file_view_posix_impl.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/file_view_win32_impl.cpp>
    glob.hpp
    path_location.hpp
    $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/path_location_macos.mm>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/path_location_win32_impl.cpp>
    resource_view.hpp
    URI.hpp
    URL.hpp
)
