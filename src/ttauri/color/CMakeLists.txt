
target_sources(ttauri PRIVATE
    color.hpp
    color_space.hpp
    sRGB.hpp
    Rec2020.hpp
    Rec2100.hpp
    quad_color.hpp
    sRGB.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        color_space_tests.cpp
    )
endif()
