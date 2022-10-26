# Copyright Take Vos 2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    axis.hpp
    axis_aligned_rectangle.cpp
    axis_aligned_rectangle.hpp
    circle.hpp
    extent.hpp
    identity.hpp
    line_segment.hpp
    line_end_cap.hpp
    line_join_style.hpp
    lookat.hpp
    margins.hpp
    matrix.hpp
    perspective.hpp
    point.hpp
    rectangle.hpp
    rotate.hpp
    scale.hpp
    spreadsheet_address.hpp
    transform.hpp
    translate.hpp
    vector.hpp
)

if(BUILD_TESTING)
    target_sources(hikogui_tests PRIVATE
        identity_tests.cpp
        matrix_tests.cpp
        point_tests.cpp
        scale_tests.cpp
        spreadsheet_address_tests.cpp
        transform_tests.cpp
        translate_tests.cpp
        vector_tests.cpp
    )
endif()
