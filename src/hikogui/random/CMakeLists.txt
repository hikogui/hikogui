# Copyright Take Vos 2020.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    dither.hpp
    xorshift128p.hpp
)

if(TT_BUILD_TESTS)
    target_sources(hikogui_tests PRIVATE
        dither_tests.cpp
        xorshift128p_tests.cpp
    )
endif()

