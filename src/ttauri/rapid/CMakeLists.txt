
target_sources(ttauri PRIVATE
    sample_reader.hpp
)

#if(TT_BUILD_TESTS)
#    target_sources(ttauri_tests PRIVATE
#        color_space_tests.cpp
#    )
#endif()
