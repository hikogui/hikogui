
target_sources(ttauri PRIVATE
    iso_15924.cpp    
    iso_15924.hpp
    iso_639.hpp
    iso_3166.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        iso_15924_tests.cpp
    )
endif()
