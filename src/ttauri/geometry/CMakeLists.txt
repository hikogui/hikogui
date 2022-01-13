
target_sources(ttauri PRIVATE
    axis.hpp
    axis_aligned_rectangle.cpp
    axis_aligned_rectangle.hpp
    circle.hpp
    extent.hpp
    identity.hpp
    line_segment.hpp
    line_end_cap.hpp
    line_join_style.hpp
    margins.hpp
    matrix.hpp
    point.hpp
    rectangle.hpp
    rotate.hpp
    scale.hpp
    spreadsheet_address.hpp
    transform.hpp
    translate.hpp
    vector.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
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
