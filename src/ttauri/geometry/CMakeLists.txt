
target_sources(ttauri PRIVATE
    axis_aligned_rectangle.cpp
    axis_aligned_rectangle.hpp
    extent.hpp
    identity.hpp
    f32x4_sse.hpp
    matrix.hpp
    numeric_array.hpp
    point.hpp
    rectangle.hpp
    rotate.hpp
    scale.hpp
    spread_sheet_address.hpp
    transform.hpp
    translate.hpp
    vector.hpp
)

target_sources(ttauri_tests PRIVATE
    identity_tests.cpp
    matrix_tests.cpp
    numeric_array_tests.cpp
    point_tests.cpp
    scale_tests.cpp
    spread_sheet_address_tests.cpp
    transform_tests.cpp
    translate_tests.cpp
    vector_tests.cpp
)

