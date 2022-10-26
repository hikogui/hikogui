# Copyright Take Vos 2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    color.hpp
    color_space.hpp
    sRGB.hpp
    Rec2020.hpp
    Rec2100.hpp
    quad_color.hpp
	semantic_color.hpp
    sRGB.hpp
)

if(BUILD_TESTING)
    target_sources(hikogui_tests PRIVATE
        color_space_tests.cpp
    )
endif()
