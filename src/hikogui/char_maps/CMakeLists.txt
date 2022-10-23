# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    ascii.hpp
    char_converter.hpp
    cp_1252.hpp
    iso_8859_1.hpp
    random_char.hpp
    to_string.hpp
    utf_8.hpp
    utf_16.hpp
    utf_32.hpp
)

if(BUILD_TESTING)
    target_sources(hikogui_tests PRIVATE
        ascii_tests.cpp
        cp_1252_tests.cpp
        iso_8859_1_tests.cpp
        utf_8_tests.cpp
        utf_16_tests.cpp
        utf_32_tests.cpp
    )
endif()
