
target_sources(ttauri PRIVATE
    iso_15924.cpp    
    iso_15924.hpp
)

if(TT_BUILD_TESTS)
    #target_sources(ttauri_tests PRIVATE
    #    color_space_tests.cpp
    #)
endif()
