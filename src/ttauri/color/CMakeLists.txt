
target_sources(ttauri PRIVATE
    color_space.hpp
    sRGB.hpp
    Rec2020.hpp
    Rec2100.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        color_space_tests.cpp
    )
endif()
